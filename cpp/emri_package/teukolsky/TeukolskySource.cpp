#include "TeukolskySource.h"

#include <cmath>
#include <complex>

using Complex = std::complex<double>;
using namespace std::complex_literals;

namespace {
constexpr double kMass = 1.0;

inline Complex get_Vs(int s, int m, double a_omega, double sin_th, double cos_th) {
    if (std::abs(sin_th) < 1e-10) {
        sin_th = 1e-10;
    }
    const double cot_th = cos_th / sin_th;
    return -static_cast<double>(m) / sin_th + a_omega * sin_th + static_cast<double>(s) * cot_th;
}

struct SourceContext {
    double r;
    double theta;
    double ut;
    double ur;
    double uth;
    double E;
    double Lz;
    double sin_th;
    double cos_th;
    double sin2;
    double Sigma;
    double Delta;
    double a;
    double omega;
    int m;
    double a_omega;
    double K_val;
    Complex rho;
    Complex rho_bar;
    Complex rho2;
    Complex rho3;
    Complex K_div_Delta;
    Complex S;
    Complex L2_dag_S;
    Complex L1L2_dag_S;
    Complex V_2;
    Complex V_1;
    Complex dS_dtheta;
    Complex d_L2_dag_S_dtheta;
};

SourceContext make_context(
    double a,
    double omega,
    int m,
    const KerrGeo::State& st,
    const KerrGeo& geo_obj,
    SWSH& swsh) {
    SourceContext ctx{};
    ctx.r = st.x[1];
    ctx.theta = st.x[2];
    ctx.ut = st.u[0];
    ctx.ur = st.u[1];
    ctx.uth = st.u[2];
    ctx.E = geo_obj.energy();
    ctx.Lz = geo_obj.angular_momentum();
    ctx.sin_th = std::sin(ctx.theta);
    ctx.cos_th = std::cos(ctx.theta);
    ctx.sin2 = ctx.sin_th * ctx.sin_th;
    ctx.Sigma = ctx.r * ctx.r + a * a * ctx.cos_th * ctx.cos_th;
    ctx.Delta = ctx.r * ctx.r - 2.0 * kMass * ctx.r + a * a;
    ctx.a = a;
    ctx.omega = omega;
    ctx.m = m;
    ctx.a_omega = a * omega;
    ctx.K_val = (ctx.r * ctx.r + a * a) * omega - static_cast<double>(m) * a;
    ctx.rho = 1.0 / (ctx.r - 1.0i * a * ctx.cos_th);
    ctx.rho_bar = 1.0 / (ctx.r + 1.0i * a * ctx.cos_th);
    ctx.rho2 = ctx.rho * ctx.rho;
    ctx.rho3 = ctx.rho2 * ctx.rho;
    ctx.K_div_Delta = ctx.K_val / ctx.Delta;
    const double x_cos = ctx.cos_th;
    ctx.S = swsh.evaluate_S(x_cos);
    ctx.L2_dag_S = swsh.evaluate_L2dag_S(x_cos);
    ctx.L1L2_dag_S = swsh.evaluate_L1dag_L2dag_S(x_cos);
    ctx.V_2 = get_Vs(2, m, ctx.a_omega, ctx.sin_th, ctx.cos_th);
    ctx.V_1 = get_Vs(1, m, ctx.a_omega, ctx.sin_th, ctx.cos_th);
    ctx.dS_dtheta = ctx.L2_dag_S - ctx.V_2 * ctx.S;
    ctx.d_L2_dag_S_dtheta = ctx.L1L2_dag_S - ctx.V_1 * ctx.L2_dag_S;
    return ctx;
}

Complex compute_operator_nn0(const SourceContext& ctx) {
    const Complex d_rho_dth = -1.0i * ctx.a * ctx.rho2 * ctx.sin_th;
    const Complex d_rho3_dth = 3.0 * ctx.rho2 * d_rho_dth;
    const Complex term_inner_bracket =
        ctx.dS_dtheta + (ctx.V_2 - 3.0 * 1.0i * ctx.a * ctx.rho * ctx.sin_th) * ctx.S;
    const Complex inner = ctx.rho3 * term_inner_bracket;
    const Complex d_V_2_dth =
        static_cast<double>(ctx.m) * ctx.cos_th / ctx.sin2 +
        ctx.a_omega * ctx.cos_th -
        2.0 / ctx.sin2;
    const Complex d2S_dtheta2 =
        ctx.d_L2_dag_S_dtheta - d_V_2_dth * ctx.S - ctx.V_2 * ctx.dS_dtheta;
    const Complex d_rho_sin_dth =
        -1.0i * ctx.a * ctx.rho2 * ctx.sin2 + ctx.rho * ctx.cos_th;
    const Complex d_bracket_dth =
        d2S_dtheta2 +
        d_V_2_dth * ctx.S +
        ctx.V_2 * ctx.dS_dtheta -
        3.0 * 1.0i * ctx.a * (d_rho_sin_dth * ctx.S + ctx.rho * ctx.sin_th * ctx.dS_dtheta);
    const Complex d_inner_dth = d_rho3_dth * term_inner_bracket + ctx.rho3 * d_bracket_dth;
    return 4.0 * 1.0i * ctx.a * std::pow(ctx.rho, -3) * ctx.sin_th * inner +
           std::pow(ctx.rho, -4) * d_inner_dth +
           ctx.V_1 * std::pow(ctx.rho, -4) * inner;
}

SourceProjections compute_projections_refactored(const SourceContext& ctx) {
    SourceProjections proj;
    const double radial_bracket =
        ctx.E * (ctx.r * ctx.r + ctx.a * ctx.a) - ctx.a * ctx.Lz + ctx.Sigma * ctx.ur;
    const Complex angular_bracket =
        1.0i * ctx.sin_th * (ctx.a * ctx.E - ctx.Lz / ctx.sin2) + ctx.Sigma * ctx.uth;

    const Complex C_nn = (radial_bracket * radial_bracket) /
                         (4.0 * ctx.Sigma * ctx.Sigma * ctx.Sigma * ctx.ut);
    const Complex C_mbarn =
        -(ctx.rho * radial_bracket * angular_bracket) /
        (2.0 * std::sqrt(2.0) * ctx.Sigma * ctx.Sigma * ctx.ut);
    const Complex C_mbarmbar =
        (ctx.rho2 * angular_bracket * angular_bracket) /
        (2.0 * ctx.Sigma * ctx.ut);

    const Complex factor_sqrt_2pi = 1.0 / std::sqrt(2.0 * M_PI);
    const Complex factor_2_sqrt_pi_Delta = 2.0 / (std::sqrt(M_PI) * ctx.Delta);
    const Complex op_nn0 = compute_operator_nn0(ctx);

    proj.A_nn0 = (-2.0 * factor_sqrt_2pi / (ctx.Delta * ctx.Delta)) *
                 std::pow(ctx.rho, -2) * std::pow(ctx.rho_bar, -1) * C_nn * op_nn0;

    const Complex term_mbarn0 =
        ctx.L2_dag_S * (1.0i * ctx.K_div_Delta + ctx.rho + ctx.rho_bar) -
        ctx.a * ctx.sin_th * ctx.S * ctx.K_div_Delta * (ctx.rho_bar - ctx.rho);
    proj.A_mbarn0 =
        factor_2_sqrt_pi_Delta * std::pow(ctx.rho, -3) * C_mbarn * term_mbarn0;

    const Complex term_mbarn1 =
        ctx.L2_dag_S + 1.0i * ctx.a * ctx.sin_th * (ctx.rho_bar - ctx.rho) * ctx.S;
    proj.A_mbarn1 =
        factor_2_sqrt_pi_Delta * std::pow(ctx.rho, -3) * C_mbarn * term_mbarn1;

    const double dK_dr = 2.0 * ctx.r * ctx.omega;
    const double dDelta_dr = 2.0 * (ctx.r - kMass);
    const Complex d_K_div_Delta_dr =
        (dK_dr * ctx.Delta - ctx.K_val * dDelta_dr) / (ctx.Delta * ctx.Delta);
    const Complex term_mm0 =
        -1.0i * d_K_div_Delta_dr - ctx.K_div_Delta * ctx.K_div_Delta +
        2.0 * 1.0i * ctx.rho * ctx.K_div_Delta;

    proj.A_mbarmbar0 = -factor_sqrt_2pi * std::pow(ctx.rho, -3) * ctx.rho_bar *
                       C_mbarmbar * ctx.S * term_mm0;
    proj.A_mbarmbar1 = (-2.0 * factor_sqrt_2pi) * std::pow(ctx.rho, -3) * ctx.rho_bar *
                       C_mbarmbar * ctx.S * (1.0i * ctx.K_div_Delta + ctx.rho);
    proj.A_mbarmbar2 =
        -factor_sqrt_2pi * std::pow(ctx.rho, -3) * ctx.rho_bar * C_mbarmbar * ctx.S;
    return proj;
}

SourceProjections compute_projections_legacy(const SourceContext& ctx) {
    SourceProjections proj;
    const double term_rad =
        (ctx.E * (ctx.r * ctx.r + ctx.a * ctx.a) - ctx.a * ctx.Lz + ctx.Sigma * ctx.ur) /
        (2.0 * ctx.Sigma);
    const Complex term_ang =
        -ctx.rho * 1.0i * ctx.sin_th * (ctx.a * ctx.E - ctx.Lz / ctx.sin2) +
        ctx.Sigma * ctx.uth / std::sqrt(2.0);

    const Complex C_nn = (term_rad * term_rad) / (ctx.Sigma * ctx.ut);
    const Complex C_mbarn = term_rad * term_ang / (ctx.Sigma * ctx.ut);
    const Complex C_mbarmbar = (term_ang * term_ang) / (ctx.Sigma * ctx.ut);

    const Complex factor_sqrt_2pi = 1.0 / std::sqrt(2.0 * M_PI);
    const Complex factor_2_sqrt_pi_Delta = 2.0 / (std::sqrt(M_PI) * ctx.Delta);
    const Complex op_nn0 = compute_operator_nn0(ctx);

    proj.A_nn0 = (-2.0 * factor_sqrt_2pi / (ctx.Delta * ctx.Delta)) *
                 std::pow(ctx.rho, -2) * std::pow(ctx.rho_bar, -1) * C_nn * op_nn0;

    const Complex term_mbarn0 =
        ctx.L2_dag_S * (1.0i * ctx.K_div_Delta + ctx.rho + ctx.rho_bar) -
        ctx.a * ctx.sin_th * ctx.S * ctx.K_div_Delta * (ctx.rho_bar - ctx.rho);
    proj.A_mbarn0 =
        factor_2_sqrt_pi_Delta * std::pow(ctx.rho, -3) * C_mbarn * term_mbarn0;

    const Complex term_mbarn1 =
        ctx.L2_dag_S + 1.0i * ctx.a * ctx.sin_th * (ctx.rho_bar - ctx.rho) * ctx.S;
    proj.A_mbarn1 =
        factor_2_sqrt_pi_Delta * std::pow(ctx.rho, -3) * C_mbarn * term_mbarn1;

    const double dK_dr = 2.0 * ctx.r * ctx.omega;
    const double dDelta_dr = 2.0 * (ctx.r - kMass);
    const Complex d_K_div_Delta_dr =
        (dK_dr * ctx.Delta - ctx.K_val * dDelta_dr) / (ctx.Delta * ctx.Delta);
    const Complex term_mm0 =
        -1.0i * d_K_div_Delta_dr - ctx.K_div_Delta * ctx.K_div_Delta +
        2.0 * 1.0i * ctx.rho * ctx.K_div_Delta;

    proj.A_mbarmbar0 = -factor_sqrt_2pi * std::pow(ctx.rho, -3) * ctx.rho_bar *
                       C_mbarmbar * ctx.S * term_mm0;
    proj.A_mbarmbar1 = (-2.0 * factor_sqrt_2pi) * std::pow(ctx.rho, -3) * ctx.rho_bar *
                       C_mbarmbar * ctx.S * (1.0i * ctx.K_div_Delta + ctx.rho);
    proj.A_mbarmbar2 =
        -factor_sqrt_2pi * std::pow(ctx.rho, -3) * ctx.rho_bar * C_mbarmbar * ctx.S;
    return proj;
}

}  // namespace

TeukolskySource::TeukolskySource(Real a_spin, Real omega, int s, int l, int m)
    : m_a(a_spin), m_omega(omega), m_s(s), m_l(l), m_m(m) {}

SourceProjections TeukolskySource::ComputeProjections(
    const KerrGeo::State& st,
    const KerrGeo& geo_obj,
    SWSH& swsh) const {
    const SourceContext ctx = make_context(m_a, m_omega, m_m, st, geo_obj, swsh);
    return compute_projections_refactored(ctx);
}

SourceProjections TeukolskySource::ComputeProjectionsLegacy(
    const KerrGeo::State& st,
    const KerrGeo& geo_obj,
    SWSH& swsh) const {
    const SourceContext ctx = make_context(m_a, m_omega, m_m, st, geo_obj, swsh);
    return compute_projections_legacy(ctx);
}

Complex TeukolskySource::ComputeW(
    const KerrGeo::State& st,
    const KerrGeo& geo_obj,
    SWSH& swsh,
    Complex R_in,
    Complex dR_dr,
    Complex d2R_dr2) const {
    const SourceProjections proj = ComputeProjections(st, geo_obj, swsh);
    return R_in * (proj.A_nn0 + proj.A_mbarn0 + proj.A_mbarmbar0) -
           dR_dr * (proj.A_mbarn1 + proj.A_mbarmbar1) +
           d2R_dr2 * proj.A_mbarmbar2;
}

SourceIntegrand TeukolskySource::ComputeIntegrand(
    const KerrGeo::State& st,
    const KerrGeo& geo_obj,
    SWSH& swsh,
    Complex R_in,
    Complex dR_dr,
    Complex d2R_dr2) const {
    SourceIntegrand integrand;
    integrand.projections = ComputeProjections(st, geo_obj, swsh);
    integrand.W = R_in * (integrand.projections.A_nn0 +
                          integrand.projections.A_mbarn0 +
                          integrand.projections.A_mbarmbar0) -
                  dR_dr * (integrand.projections.A_mbarn1 +
                           integrand.projections.A_mbarmbar1) +
                  d2R_dr2 * integrand.projections.A_mbarmbar2;
    return integrand;
}
