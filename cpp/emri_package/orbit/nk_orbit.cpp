#include <gsl/gsl_errno.h>
#include <gsl/gsl_odeiv2.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <vector>

#include "nk_orbit.hpp"

namespace {

struct GG06Coeffs {
    double g[17];
    double g10a;
    double g10b;
};

GG06Coeffs calc_g_coeffs(double e) {
    GG06Coeffs c{};
    const double e2 = e * e;
    const double e4 = e2 * e2;
    const double e6 = e4 * e2;
    c.g[1] = 1.0 + (73.0 / 24.0) * e2 + (37.0 / 96.0) * e4;
    c.g[2] = (73.0 / 12.0) + (823.0 / 24.0) * e2 + (949.0 / 32.0) * e4 + (491.0 / 192.0) * e6;
    c.g[3] = (1247.0 / 336.0) + (9181.0 / 672.0) * e2;
    c.g[4] = 4.0 + (1375.0 / 48.0) * e2;
    c.g[5] = (44711.0 / 9072.0) + (172157.0 / 2592.0) * e2;
    c.g[6] = (33.0 / 16.0) + (359.0 / 32.0) * e2;
    c.g[7] = (8191.0 / 672.0) + (44531.0 / 336.0) * e2;
    c.g[8] = (3749.0 / 336.0) - (5143.0 / 168.0) * e2;
    c.g[9] = 1.0 + (7.0 / 8.0) * e2;
    c.g[11] = (1247.0 / 336.0) + (425.0 / 336.0) * e2;
    c.g[12] = 4.0 + (97.0 / 8.0) * e2;
    c.g[13] = (44711.0 / 9072.0) + (302893.0 / 6048.0) * e2;
    c.g[14] = (33.0 / 16.0) + (95.0 / 16.0) * e2;
    c.g[15] = (8191.0 / 672.0) + (48361.0 / 1344.0) * e2;
    c.g[16] = (417.0 / 56.0) - (37241.0 / 672.0) * e2;
    c.g10a = (61.0 / 24.0) + (63.0 / 8.0) * e2 + (95.0 / 64.0) * e4;
    c.g10b = (61.0 / 8.0) + (91.0 / 4.0) * e2 + (461.0 / 64.0) * e4;
    return c;
}

double infer_radial_direction(double psi) {
    const double s = std::sin(psi);
    if (std::abs(s) < 1e-12) {
        return 0.0;
    }
    return (s > 0.0) ? 1.0 : -1.0;
}

double infer_theta_direction(double chi) {
    const double s = std::sin(chi);
    if (std::abs(s) < 1e-12) {
        return 0.0;
    }
    return (s > 0.0) ? 1.0 : -1.0;
}

}  // namespace

namespace emri_package {

BabakNKOrbit::BabakNKOrbit(double M, double a, double p, double e, double iota, double mu)
    : p0(p),
      e0(e),
      iota0(iota),
      cached_E(0.0),
      cached_Lz(0.0),
      cached_Q(0.0),
      M_phys(M),
      mu_phys(mu),
      a_spin(a),
      do_inspiral(mu > 0.0),
      m_t(0.0),
      m_p(p),
      m_e(e),
      m_iota(iota),
      m_psi(0.0),
      m_chi(0.0),
      m_phi(0.0) {}

OrbitState BabakNKOrbit::get_current_state() const {
    const double r_val = m_p / (1.0 + m_e * std::cos(m_psi));
    return {m_t, m_p, m_e, m_iota, m_psi, m_chi, m_phi, r_val, 0.0};
}

OrbitKinematics BabakNKOrbit::get_state_kinematics(const OrbitState& state) const {
    const KerrConstants k = get_conserved_quantities(1.0, a_spin, state.p, state.e, state.iota);
    const double r = state.r;
    const double theta = state.theta;
    const double cos_th = std::cos(theta);
    const double sin_th = std::sin(theta);
    const double sin2 = sin_th * sin_th;
    const double safe_sin2 = std::max(sin2, 1e-12);
    const double Sigma = r * r + a_spin * a_spin * cos_th * cos_th;
    const double Delta = r * r - 2.0 * r + a_spin * a_spin;
    const double P = k.E * (r * r + a_spin * a_spin) - a_spin * k.Lz;

    const double ut =
        (((r * r + a_spin * a_spin) * P) / Delta + a_spin * (k.Lz - a_spin * k.E * sin2)) / Sigma;
    const double uphi = (a_spin * P / Delta + k.Lz / safe_sin2 - a_spin * k.E) / Sigma;

    double R_val = radial_potential(r, 1.0, a_spin, k.E, k.Lz, k.Q);
    if (R_val < 0.0) {
        R_val = 0.0;
    }

    double Th_val = k.Q - (k.Lz * k.Lz * cos_th * cos_th) / std::max(1.0 - cos_th * cos_th, 1e-12) -
                    a_spin * a_spin * cos_th * cos_th * (1.0 - k.E * k.E);
    if (Th_val < 0.0) {
        Th_val = 0.0;
    }

    const double ur = infer_radial_direction(state.psi) * std::sqrt(R_val) / Sigma;
    const double utheta = infer_theta_direction(state.chi) * std::sqrt(Th_val) / Sigma;

    return {
        state.t,  state.p, state.e, state.iota, state.psi, state.chi, state.phi, state.r, state.theta,
        k.E,      k.Lz,    k.Q,     ut,         ur,        utheta,    uphi,      Sigma,   Delta,
        P};
}

OrbitKinematics BabakNKOrbit::get_current_kinematics() const {
    OrbitState state = get_current_state();
    const KerrConstants k = get_conserved_quantities(1.0, a_spin, state.p, state.e, state.iota);
    double cos_theta = std::sqrt(std::max(0.0, k.z_minus)) * std::cos(state.chi);
    if (cos_theta > 1.0) {
        cos_theta = 1.0;
    }
    if (cos_theta < -1.0) {
        cos_theta = -1.0;
    }
    state.theta = std::acos(cos_theta);
    return get_state_kinematics(state);
}

double BabakNKOrbit::radial_potential(double r, double M, double a, double E, double Lz, double Q) {
    const double Delta = r * r - 2.0 * M * r + a * a;
    const double term1 = std::pow(E * (r * r + a * a) - Lz * a, 2);
    const double term2 = Delta * (r * r + std::pow(Lz - a * E, 2) + Q);
    return term1 - term2;
}

double BabakNKOrbit::radial_potential_deriv(double r, double M, double a, double E, double Lz, double Q) {
    const double Delta = r * r - 2.0 * M * r + a * a;
    const double dDelta = 2.0 * r - 2.0 * M;
    const double X = E * (r * r + a * a) - Lz * a;
    const double dX = 2.0 * E * r;
    const double Y = r * r + std::pow(Lz - a * E, 2) + Q;
    const double dY = 2.0 * r;
    return 2.0 * X * dX - (dDelta * Y + Delta * dY);
}

KerrConstants BabakNKOrbit::get_conserved_quantities(
    double,
    double a,
    double p,
    double e,
    double iota,
    double E_g,
    double L_g,
    double Q_g) {
    const double M_code = 1.0;
    const double r_p = p / (1.0 + e);
    const double r_a = p / (1.0 - e);

    double E = 0.0;
    double Lz = 0.0;
    double Q = 0.0;
    if (E_g != 0.0) {
        E = E_g;
        Lz = L_g;
        Q = Q_g;
    } else {
        const double sqrt_p = std::sqrt(p);
        double denom_kerr = p - 3.0 - e * e + 2.0 * a / sqrt_p;
        if (denom_kerr < 0.01) {
            denom_kerr = 0.01;
        }
        double num_E = (p - 2.0 - 2.0 * e) * (p - 2.0 + 2.0 * e);
        if (num_E < 0.0) {
            num_E = 0.0;
        }
        const double num_L = p * p - 2.0 * a * sqrt_p + a * a;
        E = std::sqrt(std::max(0.0, num_E / (p * denom_kerr)));
        if (E < 0.1) {
            E = 0.9;
        }
        const double L_mag = num_L / (sqrt_p * std::sqrt(p * denom_kerr));
        Lz = L_mag * std::cos(iota);
        Q = L_mag * L_mag * std::pow(std::sin(iota), 2);
    }

    constexpr int MAX_ITER = 100;
    constexpr double TOL = 1e-8;
    bool success = false;

    for (int iter = 0; iter < MAX_ITER; ++iter) {
        double F1 = radial_potential(r_p, M_code, a, E, Lz, Q);
        double F2 = (e < 1e-4) ? radial_potential_deriv(p, M_code, a, E, Lz, Q)
                               : radial_potential(r_a, M_code, a, E, Lz, Q);
        const double c2 = std::pow(std::cos(iota), 2);
        const double s2 = std::pow(std::sin(iota), 2);
        const double F3 = Q * c2 - Lz * Lz * s2;
        const double resid_norm = std::sqrt(F1 * F1 + F2 * F2 + F3 * F3);
        if (resid_norm < TOL) {
            success = true;
            break;
        }

        auto get_V_param_derivs = [&](double r) -> std::array<double, 3> {
            const double Delta = r * r - 2.0 * M_code * r + a * a;
            const double X = E * (r * r + a * a) - Lz * a;
            const double dV_dE = 2.0 * X * (r * r + a * a) + 2.0 * a * Delta * (Lz - a * E);
            const double dV_dL = -2.0 * a * X - 2.0 * Delta * (Lz - a * E);
            const double dV_dQ = -Delta;
            return {dV_dE, dV_dL, dV_dQ};
        };

        double J11, J12, J13, J21, J22, J23, J31, J32, J33;
        const auto dVp = get_V_param_derivs(r_p);
        J11 = dVp[0];
        J12 = dVp[1];
        J13 = dVp[2];

        if (e < 1e-4) {
            const double eps = 1e-6;
            const double v_base = radial_potential_deriv(p, M_code, a, E, Lz, Q);
            J21 = (radial_potential_deriv(p, M_code, a, E + eps, Lz, Q) - v_base) / eps;
            J22 = (radial_potential_deriv(p, M_code, a, E, Lz + eps, Q) - v_base) / eps;
            J23 = (radial_potential_deriv(p, M_code, a, E, Lz, Q + eps) - v_base) / eps;
        } else {
            const auto dVa = get_V_param_derivs(r_a);
            J21 = dVa[0];
            J22 = dVa[1];
            J23 = dVa[2];
        }

        J31 = 0.0;
        J32 = -2.0 * Lz * s2;
        J33 = c2;

        const double det =
            J11 * (J22 * J33 - J23 * J32) - J12 * (J21 * J33 - J23 * J31) +
            J13 * (J21 * J32 - J22 * J31);
        if (std::abs(det) < 1e-15) {
            break;
        }

        const double inv_det = 1.0 / det;
        const double b1 = -F1;
        const double b2 = -F2;
        const double b3 = -F3;
        const double dE =
            inv_det * (b1 * (J22 * J33 - J23 * J32) + b2 * (J13 * J32 - J12 * J33) +
                       b3 * (J12 * J23 - J13 * J22));
        const double dL =
            inv_det * (b1 * (J23 * J31 - J21 * J33) + b2 * (J11 * J33 - J13 * J31) +
                       b3 * (J13 * J21 - J11 * J23));
        const double dQ =
            inv_det * (b1 * (J21 * J32 - J22 * J31) + b2 * (J12 * J31 - J11 * J32) +
                       b3 * (J11 * J22 - J12 * J21));

        double alpha = 1.0;
        if (std::abs(dE) > 0.1) {
            alpha = 0.1 / std::abs(dE);
        }
        E += alpha * dE;
        Lz += alpha * dL;
        Q += alpha * dQ;
    }

    if (!success) {
        return {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    }

    KerrConstants k{};
    k.E = E;
    k.Lz = Lz;
    k.Q = Q;
    k.r_p = r_p;
    k.r_a = r_a;

    const double E2 = E * E;
    const double inv_1_E2 = 1.0 / (1.0 - E2);
    const double S_r = 2.0 * M_code * inv_1_E2 - (r_p + r_a);
    double denom = r_p * r_a;
    if (denom < 1e-8) {
        denom = 1e-8;
    }
    const double P_r = (a * a * Q * inv_1_E2) / denom;
    const double delta_r = std::sqrt(std::max(0.0, S_r * S_r - 4.0 * P_r));
    k.r3 = (S_r + delta_r) / 2.0;
    k.r4 = (S_r - delta_r) / 2.0;
    if (k.r4 > k.r3) {
        std::swap(k.r3, k.r4);
    }

    k.beta = a * a * (1.0 - E2);
    const double term_b = Q + Lz * Lz + a * a * (1.0 - E2);
    if (std::abs(k.beta) > 1e-12) {
        const double delta_z = std::sqrt(std::max(0.0, term_b * term_b - 4.0 * k.beta * Q));
        k.z_plus = (term_b + delta_z) / (2.0 * k.beta);
        k.z_minus = (term_b - delta_z) / (2.0 * k.beta);
    } else {
        k.z_plus = 0.0;
        k.z_minus = 0.0;
    }

    return k;
}

NKFluxes BabakNKOrbit::compute_gg06_fluxes(double p, double e, double iota, double a, double M, double mu) {
    NKFluxes flux{0, 0, 0, 0, 0, 0};
    const double q_mass = mu / M;
    const double safe_e = std::max(0.0, std::min(e, 0.999));
    const double q_spin = a;
    const double Mp = 1.0 / p;
    const double Mp15 = std::pow(Mp, 1.5);
    const double Mp2 = Mp * Mp;
    const double Mp35 = std::pow(Mp, 3.5);
    const double Mp5 = std::pow(Mp, 5.0);
    const double cos_i = std::cos(iota);
    const double sin_i = std::sin(iota);
    const double sin2_i = sin_i * sin_i;
    const double cos2_i = cos_i * cos_i;
    const GG06Coeffs g = calc_g_coeffs(safe_e);
    const double prefix = std::pow(1.0 - safe_e * safe_e, 1.5);

    const double factor_E = -(32.0 / 5.0) * q_mass * Mp5;
    const double term_E =
        g.g[1] - q_spin * Mp15 * g.g[2] * cos_i - Mp * g.g[3] + M_PI * Mp15 * g.g[4] -
        Mp2 * g.g[5] + q_spin * q_spin * Mp2 * g.g[6] - (527.0 / 96.0) * q_spin * q_spin * Mp2 * sin2_i;
    flux.dE_dt = factor_E * prefix * term_E;

    const double factor_L = -(32.0 / 5.0) * q_mass * Mp35;
    const double term_L =
        g.g[9] * cos_i + q_spin * Mp15 * (g.g10a - cos2_i * g.g10b) - Mp * g.g[11] * cos_i +
        M_PI * Mp15 * g.g[12] * cos_i - Mp2 * g.g[13] * cos_i +
        q_spin * q_spin * Mp2 * cos_i * (g.g[14] - (45.0 / 8.0) * sin2_i);
    flux.dLz_dt = factor_L * prefix * term_L;

    KerrConstants k_curr = get_conserved_quantities(1.0, a, p, safe_e, iota);
    if (k_curr.E == 0.0) {
        return flux;
    }

    const double Q_spec = k_curr.Q;
    const double sqrt_Q_spec = std::sqrt(std::abs(Q_spec));
    const double factor_Q = -(64.0 / 5.0) * q_mass * Mp35 * sin_i * prefix;
    const double term_Q =
        g.g[9] - q_spin * Mp15 * cos_i * g.g10b - Mp * g.g[11] + M_PI * Mp15 * g.g[12] -
        Mp2 * g.g[13] + q_spin * q_spin * Mp2 * (g.g[14] - (45.0 / 8.0) * sin2_i);
    flux.dQ_dt = factor_Q * sqrt_Q_spec * term_Q;

    if (safe_e > 1e-4) {
        const double E = k_curr.E;
        const double Lz = k_curr.Lz;
        const double Q = k_curr.Q;
        const double r_p = p / (1.0 + safe_e);
        const double r_a = p / (1.0 - safe_e);

        auto get_V_derivs_const = [&](double r) -> std::array<double, 3> {
            const double Delta = r * r - 2.0 * r + a * a;
            const double X = E * (r * r + a * a) - Lz * a;
            const double dV_dE = 2.0 * X * (r * r + a * a) + 2.0 * a * Delta * (Lz - a * E);
            const double dV_dL = -2.0 * a * X - 2.0 * Delta * (Lz - a * E);
            const double dV_dQ = -Delta;
            return {dV_dE, dV_dL, dV_dQ};
        };

        const auto dA_rp = get_V_derivs_const(r_p);
        const auto dA_ra = get_V_derivs_const(r_a);
        const double A32 = -2.0 * Lz * sin2_i;
        const double A33 = cos2_i;

        const double Y1 = -(dA_rp[0] * flux.dE_dt + dA_rp[1] * flux.dLz_dt + dA_rp[2] * flux.dQ_dt);
        const double Y2 = -(dA_ra[0] * flux.dE_dt + dA_ra[1] * flux.dLz_dt + dA_ra[2] * flux.dQ_dt);
        const double Y3 = -(A32 * flux.dLz_dt + A33 * flux.dQ_dt);

        const double Vprime_p = radial_potential_deriv(r_p, 1.0, a, E, Lz, Q);
        const double Vprime_a = radial_potential_deriv(r_a, 1.0, a, E, Lz, Q);
        const double drp_dp = 1.0 / (1.0 + safe_e);
        const double drp_de = -p / ((1.0 + safe_e) * (1.0 + safe_e));
        const double dra_dp = 1.0 / (1.0 - safe_e);
        const double dra_de = p / ((1.0 - safe_e) * (1.0 - safe_e));
        const double B11 = Vprime_p * drp_dp;
        const double B12 = Vprime_p * drp_de;
        const double B21 = Vprime_a * dra_dp;
        const double B22 = Vprime_a * dra_de;
        const double B33 = -2.0 * sin_i * cos_i * (Q + Lz * Lz);

        if (std::abs(B33) > 1e-14) {
            flux.diota_dt = Y3 / B33;
        }

        const double detB_sub = B11 * B22 - B12 * B21;
        if (std::abs(detB_sub) > 1e-18) {
            flux.dp_dt = (B22 * Y1 - B12 * Y2) / detB_sub;
            flux.de_dt = (B11 * Y2 - B21 * Y1) / detB_sub;
        }
    } else {
        const double dp = 1e-5 * p;
        const double de_step = 1e-5;
        const double di = 1e-5;

        auto get_ELQ = [&](double tp, double te, double ti) -> std::array<double, 3> {
            const KerrConstants k = get_conserved_quantities(1.0, a, tp, te, ti);
            return {k.E, k.Lz, k.Q};
        };

        const auto v0 = get_ELQ(p, safe_e, iota);
        const auto vp = get_ELQ(p + dp, safe_e, iota);
        const auto ve = get_ELQ(p, safe_e + de_step, iota);
        const auto vi = get_ELQ(p, safe_e, iota + di);

        const double J11 = (vp[0] - v0[0]) / dp;
        const double J12 = (ve[0] - v0[0]) / de_step;
        const double J13 = (vi[0] - v0[0]) / di;
        const double J21 = (vp[1] - v0[1]) / dp;
        const double J22 = (ve[1] - v0[1]) / de_step;
        const double J23 = (vi[1] - v0[1]) / di;
        const double J31 = (vp[2] - v0[2]) / dp;
        const double J32 = (ve[2] - v0[2]) / de_step;
        const double J33 = (vi[2] - v0[2]) / di;
        const double det =
            J11 * (J22 * J33 - J23 * J32) - J12 * (J21 * J33 - J23 * J31) +
            J13 * (J21 * J32 - J22 * J31);
        if (std::abs(det) > 1e-16) {
            const double idet = 1.0 / det;
            flux.dp_dt =
                idet * (flux.dE_dt * (J22 * J33 - J23 * J32) +
                        flux.dLz_dt * (J13 * J32 - J12 * J33) +
                        flux.dQ_dt * (J12 * J23 - J13 * J22));
            flux.de_dt =
                idet * (flux.dE_dt * (J23 * J31 - J21 * J33) +
                        flux.dLz_dt * (J11 * J33 - J13 * J31) +
                        flux.dQ_dt * (J13 * J21 - J11 * J23));
            flux.diota_dt =
                idet * (flux.dE_dt * (J21 * J32 - J22 * J31) +
                        flux.dLz_dt * (J12 * J31 - J11 * J32) +
                        flux.dQ_dt * (J11 * J22 - J12 * J21));
        }
    }

    return flux;
}

int BabakNKOrbit::gsl_derivs(double, const double y[], double dydt[], void* params) {
    auto* p = static_cast<GSLParams*>(params);
    const double a = p->a;
    const bool inspiral = p->do_inspiral;
    const double cp = y[0];
    const double ce = y[1];
    const double ci = y[2];
    const double cpsi = y[3];
    const double cchi = y[4];

    if (cp < 3.0 || ce >= 0.999) {
        return GSL_EDOM;
    }

    double dp_dt = 0.0;
    double de_dt = 0.0;
    double diota_dt = 0.0;
    if (inspiral) {
        const NKFluxes f = compute_gg06_fluxes(cp, ce, ci, a, 1.0, p->mu);
        dp_dt = f.dp_dt;
        de_dt = f.de_dt;
        diota_dt = f.diota_dt;
    }

    auto* orbit = static_cast<BabakNKOrbit*>(p->orbit_ptr);
    const KerrConstants ck = orbit->get_conserved_quantities(1.0, a, cp, ce, ci, orbit->cached_E, orbit->cached_Lz, orbit->cached_Q);
    if (ck.E == 0.0) {
        return GSL_EFAILED;
    }
    orbit->cached_E = ck.E;
    orbit->cached_Lz = ck.Lz;
    orbit->cached_Q = ck.Q;

    const double r = cp / (1.0 + ce * std::cos(cpsi));
    const double z = ck.z_minus * std::pow(std::cos(cchi), 2);
    const double Delta = r * r - 2.0 * r + a * a;
    const double sin2theta = 1.0 - z;
    const double V_phi = ck.Lz / sin2theta - a * ck.E +
                         (a / Delta) * (ck.E * (r * r + a * a) - ck.Lz * a);
    const double V_t = a * (ck.Lz - a * ck.E * sin2theta) +
                       ((r * r + a * a) / Delta) * (ck.E * (r * r + a * a) - ck.Lz * a);
    const double gamma = ck.E * (std::pow(r * r + a * a, 2) / Delta - a * a) -
                         (2.0 * r * a * ck.Lz) / Delta;
    const double denominator = gamma + a * a * ck.E * z;
    const double dchi_dt = std::sqrt(std::abs(ck.beta * (ck.z_plus - z))) / denominator;

    const double term_r = (1.0 - ck.E * ck.E) * (ck.r_a - r) * (r - ck.r_p) * (r - ck.r3) * (r - ck.r4);
    const double V_r = std::max(0.0, term_r);
    const double denom_psi = 1.0 + ce * std::cos(cpsi);
    const double dr_dpsi = (cp * ce * std::sin(cpsi)) / (denom_psi * denom_psi);
    double dpsi_dt = 0.0;
    if (std::abs(std::sin(cpsi)) < 1e-5) {
        dpsi_dt = std::sqrt(V_r + 1e-14) / (V_t * (std::abs(dr_dpsi) + 1e-7));
        if (std::abs(dr_dpsi) < 1e-9) {
            dpsi_dt = 1e-3;
        }
    } else {
        dpsi_dt = std::sqrt(V_r) / (V_t * std::abs(dr_dpsi));
    }
    dpsi_dt = std::abs(dpsi_dt);
    const double dphi_dt = V_phi / V_t;

    dydt[0] = dp_dt;
    dydt[1] = de_dt;
    dydt[2] = diota_dt;
    dydt[3] = dpsi_dt;
    dydt[4] = dchi_dt;
    dydt[5] = dphi_dt;
    return GSL_SUCCESS;
}

std::vector<OrbitState> BabakNKOrbit::evolve(double duration, double dt_sampling) {
    std::vector<OrbitState> traj;
    traj.reserve(static_cast<size_t>(duration / dt_sampling) + 1000);

    GSLParams params{};
    params.M = 1.0;
    params.a = a_spin;
    params.mu = mu_phys / M_phys;
    params.do_inspiral = do_inspiral;
    params.orbit_ptr = this;

    gsl_odeiv2_system sys = {gsl_derivs, nullptr, 6, &params};
    const gsl_odeiv2_step_type* T = gsl_odeiv2_step_rk8pd;
    gsl_odeiv2_step* s = gsl_odeiv2_step_alloc(T, 6);
    gsl_odeiv2_control* c = gsl_odeiv2_control_y_new(1e-8, 1e-8);
    gsl_odeiv2_evolve* e = gsl_odeiv2_evolve_alloc(6);

    double t = m_t;
    double h = 1e-3;
    double y[6] = {m_p, m_e, m_iota, m_psi, m_chi, m_phi};
    double t_target_sample = m_t;
    const double t_end = m_t + duration;

    while (t_target_sample <= t_end) {
        while (t < t_target_sample) {
            const int status = gsl_odeiv2_evolve_apply(e, c, s, &sys, &t, t_target_sample, &h, y);
            if (status != GSL_SUCCESS) {
                goto cleanup;
            }
        }

        const KerrConstants k = get_conserved_quantities(1.0, a_spin, y[0], y[1], y[2]);
        if (k.E == 0.0) {
            goto cleanup;
        }
        const double r_val = y[0] / (1.0 + y[1] * std::cos(y[3]));
        double cos_theta = std::sqrt(std::max(0.0, k.z_minus)) * std::cos(y[4]);
        if (cos_theta > 1.0) {
            cos_theta = 1.0;
        }
        if (cos_theta < -1.0) {
            cos_theta = -1.0;
        }
        const double theta_val = std::acos(cos_theta);
        traj.push_back({t, y[0], y[1], y[2], y[3], y[4], y[5], r_val, theta_val});
        t_target_sample += dt_sampling;
    }

cleanup:
    gsl_odeiv2_evolve_free(e);
    gsl_odeiv2_control_free(c);
    gsl_odeiv2_step_free(s);

    m_t = t;
    m_p = y[0];
    m_e = y[1];
    m_iota = y[2];
    m_psi = y[3];
    m_chi = y[4];
    m_phi = y[5];
    return traj;
}

}  // namespace emri_package
