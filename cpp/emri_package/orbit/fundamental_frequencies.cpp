#include "fundamental_frequencies.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "nk_orbit.hpp"

namespace {

constexpr double kPi = 3.141592653589793238462643383279502884;

struct OrbitRoots {
    double r1;
    double r2;
    double r3;
    double r4;
    double z_minus;
    double z_plus;
    double epsilon0;
};

template <typename Fn>
double simpson_integrate(Fn&& f, double a, double b, int n_panels) {
    if (n_panels < 2) {
        n_panels = 2;
    }
    if (n_panels % 2 == 1) {
        ++n_panels;
    }
    const double h = (b - a) / static_cast<double>(n_panels);
    double sum = f(a) + f(b);
    for (int i = 1; i < n_panels; ++i) {
        const double x = a + h * static_cast<double>(i);
        sum += (i % 2 == 0 ? 2.0 : 4.0) * f(x);
    }
    return sum * h / 3.0;
}

double safe_sin2_from_z(double z) {
    return std::max(1.0 - z * z, 1e-14);
}

OrbitRoots compute_polar_roots(double a, double E, double Lz, double Q) {
    const double one_minus_E2 = 1.0 - E * E;
    if (one_minus_E2 <= 0.0) {
        throw std::runtime_error("FH09 frequency formula requires bound timelike orbit with E^2 < 1");
    }
    const double beta = a * a * one_minus_E2;
    if (std::abs(beta) < 1e-14) {
        throw std::runtime_error("FH09 generic-frequency implementation does not support beta ~ 0");
    }

    OrbitRoots roots{};
    const double term_b = Q + Lz * Lz + beta;
    const double delta_z = std::sqrt(std::max(0.0, term_b * term_b - 4.0 * beta * Q));
    roots.z_plus = (term_b + delta_z) / (2.0 * beta);
    roots.z_minus = (term_b - delta_z) / (2.0 * beta);
    roots.epsilon0 = beta / (Lz * Lz);
    if (!(roots.z_plus > 0.0 && roots.z_minus >= 0.0)) {
        throw std::runtime_error("Invalid polar roots in FH09 frequency calculation");
    }
    return roots;
}

OrbitRoots compute_roots_from_constants(double a, double E, double Lz, double Q) {
    OrbitRoots roots = compute_polar_roots(a, E, Lz, Q);

    const double a4 = E * E - 1.0;
    const double a3 = 2.0;
    const double a2 = a * a * (E * E - 1.0) - Lz * Lz - Q;
    const double a1 = 2.0 * ((a * E - Lz) * (a * E - Lz) + Q);
    const double a0 = -a * a * Q;

    auto poly = [&](double r) {
        return (((a4 * r + a3) * r + a2) * r + a1) * r + a0;
    };

    double found[4];
    int n_found = 0;
    const double r_min = 1e-8;
    const double r_max = 1e4;
    const int n_scan = 20000;
    double prev_r = r_min;
    double prev_f = poly(prev_r);

    for (int i = 1; i <= n_scan && n_found < 4; ++i) {
        const double frac = static_cast<double>(i) / static_cast<double>(n_scan);
        const double r = r_min * std::pow(r_max / r_min, frac);
        const double val = poly(r);
        if (prev_f == 0.0) {
            found[n_found++] = prev_r;
        } else if (val == 0.0 || prev_f * val < 0.0) {
            double lo = prev_r;
            double hi = r;
            double flo = prev_f;
            for (int it = 0; it < 100; ++it) {
                const double mid = 0.5 * (lo + hi);
                const double fm = poly(mid);
                if (std::abs(fm) < 1e-13) {
                    lo = hi = mid;
                    break;
                }
                if (flo * fm <= 0.0) {
                    hi = mid;
                } else {
                    lo = mid;
                    flo = fm;
                }
            }
            found[n_found++] = 0.5 * (lo + hi);
        }
        prev_r = r;
        prev_f = val;
    }

    if (n_found < 4) {
        throw std::runtime_error("Failed to recover four radial roots for FH09 frequency calculation");
    }

    std::sort(found, found + 4, std::greater<double>());
    roots.r1 = found[0];
    roots.r2 = found[1];
    roots.r3 = found[2];
    roots.r4 = found[3];
    return roots;
}

OrbitRoots compute_roots_from_pei(double a, double p, double e, double E, double Lz, double Q) {
    OrbitRoots roots = compute_polar_roots(a, E, Lz, Q);
    const double one_minus_E2 = 1.0 - E * E;
    roots.r1 = p / std::max(1.0 - e, 1e-14);
    roots.r2 = p / (1.0 + e);

    const double AplusB = 2.0 / one_minus_E2 - (roots.r1 + roots.r2);
    const double AB = (a * a * Q) / (one_minus_E2 * roots.r1 * roots.r2);
    const double disc = std::sqrt(std::max(0.0, AplusB * AplusB - 4.0 * AB));
    roots.r3 = 0.5 * (AplusB + disc);
    roots.r4 = AB / roots.r3;
    if (roots.r4 > roots.r3) {
        std::swap(roots.r3, roots.r4);
    }
    return roots;
}

emri_package::KerrFrequencies integrate_frequencies(double a, double E, double Lz, const OrbitRoots& roots) {
    const double one_minus_E2 = 1.0 - E * E;
    const double e = (roots.r1 - roots.r2) / (roots.r1 + roots.r2);
    const double p = 2.0 * roots.r1 * roots.r2 / (roots.r1 + roots.r2);

    auto T_r = [&](double r) {
        const double Delta = r * r - 2.0 * r + a * a;
        const double P = E * (r * r + a * a) - a * Lz;
        return ((r * r + a * a) / Delta) * P;
    };

    auto Phi_r = [&](double r) {
        const double Delta = r * r - 2.0 * r + a * a;
        const double P = E * (r * r + a * a) - a * Lz;
        return a * P / Delta;
    };

    auto T_theta = [&](double z) {
        return -a * a * E * (1.0 - z * z);
    };

    auto Phi_theta = [&](double z) {
        return Lz / safe_sin2_from_z(z);
    };

    auto dlambda_dpsi = [&](double psi) {
        const double p3 = roots.r3 * (1.0 - e);
        const double p4 = roots.r4 * (1.0 + e);
        const double c = std::cos(psi);
        const double denom =
            std::sqrt(one_minus_E2 * ((p - p3) - e * (p + p3 * c)) * ((p - p4) + e * (p - p4 * c))) /
            std::max(1.0 - e * e, 1e-14);
        return 1.0 / denom;
    };

    auto r_of_psi = [&](double psi) {
        return p / (1.0 + e * std::cos(psi));
    };

    auto dlambda_dchi = [&](double chi) {
        return 1.0 / std::sqrt(
                   a * a * one_minus_E2 *
                   (roots.z_plus - roots.z_minus * std::cos(chi) * std::cos(chi)));
    };

    auto z_of_chi = [&](double chi) {
        return std::sqrt(std::max(roots.z_minus, 0.0)) * std::cos(chi);
    };

    const int n_panels = 4096;
    const double Lambda_r = 2.0 * simpson_integrate(dlambda_dpsi, 0.0, kPi, n_panels);
    const double Lambda_theta = 4.0 * simpson_integrate(dlambda_dchi, 0.0, 0.5 * kPi, n_panels);

    emri_package::KerrFrequencies out{};
    out.Upsilon_r = 2.0 * kPi / Lambda_r;
    out.Upsilon_theta = 2.0 * kPi / Lambda_theta;

    const double Upsilon_t_r = (2.0 / Lambda_r) * simpson_integrate(
        [&](double psi) { return T_r(r_of_psi(psi)) * dlambda_dpsi(psi); },
        0.0,
        kPi,
        n_panels);

    const double Upsilon_t_theta = (4.0 / Lambda_theta) * simpson_integrate(
        [&](double chi) { return T_theta(z_of_chi(chi)) * dlambda_dchi(chi); },
        0.0,
        0.5 * kPi,
        n_panels);

    const double Upsilon_phi_r = (2.0 / Lambda_r) * simpson_integrate(
        [&](double psi) { return Phi_r(r_of_psi(psi)) * dlambda_dpsi(psi); },
        0.0,
        kPi,
        n_panels);

    const double Upsilon_phi_theta = (4.0 / Lambda_theta) * simpson_integrate(
        [&](double chi) { return Phi_theta(z_of_chi(chi)) * dlambda_dchi(chi); },
        0.0,
        0.5 * kPi,
        n_panels);

    out.Gamma = Upsilon_t_r + Upsilon_t_theta + a * Lz;
    out.Upsilon_phi = Upsilon_phi_r + Upsilon_phi_theta - a * E;
    if (!(out.Gamma > 0.0)) {
        throw std::runtime_error("Computed non-positive Gamma in FH09 frequency calculation");
    }

    out.Omega_r = out.Upsilon_r / out.Gamma;
    out.Omega_theta = out.Upsilon_theta / out.Gamma;
    out.Omega_phi = out.Upsilon_phi / out.Gamma;
    return out;
}

}  // namespace

namespace emri_package {

KerrFrequencies KerrFundamentalFrequencies::compute(double a, double E, double Lz, double Q) {
    if (std::abs(a) >= 1.0 - 1e-12) {
        throw std::runtime_error("FH09 frequency implementation currently supports only |a| < 1");
    }
    const OrbitRoots roots = compute_roots_from_constants(a, E, Lz, Q);
    return integrate_frequencies(a, E, Lz, roots);
}

KerrFrequencies KerrFundamentalFrequencies::compute_from_pei(double a, double p, double e, double iota) {
    const KerrConstants k = BabakNKOrbit::get_conserved_quantities(1.0, a, p, e, iota);
    if (k.E == 0.0) {
        throw std::runtime_error("Failed to map (p,e,iota) to (E,Lz,Q) before FH09 frequency calculation");
    }
    return compute_from_pei_constants(a, p, e, iota, k.E, k.Lz, k.Q);
}

KerrFrequencies KerrFundamentalFrequencies::compute_from_pei_constants(
    double a,
    double p,
    double e,
    double,
    double E,
    double Lz,
    double Q) {
    if (std::abs(a) >= 1.0 - 1e-12) {
        throw std::runtime_error("FH09 frequency implementation currently supports only |a| < 1");
    }
    const OrbitRoots roots = compute_roots_from_pei(a, p, e, E, Lz, Q);
    return integrate_frequencies(a, E, Lz, roots);
}

}  // namespace emri_package
