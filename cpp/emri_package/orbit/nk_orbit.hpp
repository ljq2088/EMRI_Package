#pragma once

#include <array>
#include <vector>

#include "../emri_params.hpp"

namespace emri_package {

struct NKFluxes {
    double dE_dt;
    double dLz_dt;
    double dQ_dt;
    double dp_dt;
    double de_dt;
    double diota_dt;
};

struct OrbitState {
    double t;
    double p;
    double e;
    double iota;
    double psi;
    double chi;
    double phi;
    double r;
    double theta;
};

struct OrbitKinematics {
    double t;
    double p;
    double e;
    double iota;
    double psi;
    double chi;
    double phi;
    double r;
    double theta;

    double E;
    double Lz;
    double Q;

    double ut;
    double ur;
    double utheta;
    double uphi;

    double sigma;
    double delta;
    double P;
};

struct GSLParams {
    double M;
    double a;
    double mu;
    bool do_inspiral;
    void* orbit_ptr;
};

class BabakNKOrbit {
public:
    BabakNKOrbit(double M, double a, double p, double e, double iota, double mu);

    static KerrConstants get_conserved_quantities(
        double M,
        double a,
        double p,
        double e,
        double iota,
        double E_g = 0.0,
        double L_g = 0.0,
        double Q_g = 0.0);

    static NKFluxes compute_gg06_fluxes(double p, double e, double iota, double a, double M, double mu);
    OrbitKinematics get_state_kinematics(const OrbitState& state) const;
    OrbitKinematics get_current_kinematics() const;

    std::vector<OrbitState> evolve(double duration, double dt_sampling);
    OrbitState get_current_state() const;

    double p0;
    double e0;
    double iota0;
    double cached_E;
    double cached_Lz;
    double cached_Q;

private:
    static int gsl_derivs(double t, const double y[], double dydt[], void* params);
    static double radial_potential(double r, double M, double a, double E, double Lz, double Q);
    static double radial_potential_deriv(double r, double M, double a, double E, double Lz, double Q);

    double M_phys;
    double mu_phys;
    double a_spin;
    bool do_inspiral;

    double m_t;
    double m_p;
    double m_e;
    double m_iota;
    double m_psi;
    double m_chi;
    double m_phi;
};

}  // namespace emri_package
