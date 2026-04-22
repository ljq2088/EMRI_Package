#pragma once

#include "../emri_params.hpp"

namespace emri_package {

struct KerrFrequencies {
    double Upsilon_r;
    double Upsilon_theta;
    double Upsilon_phi;
    double Gamma;
    double Omega_r;
    double Omega_theta;
    double Omega_phi;
};

class KerrFundamentalFrequencies {
public:
    static KerrFrequencies compute(double a, double E, double Lz, double Q);
    static KerrFrequencies compute_from_pei(double a, double p, double e, double iota);
    static KerrFrequencies compute_from_pei_constants(
        double a,
        double p,
        double e,
        double iota,
        double E,
        double Lz,
        double Q);
};

}  // namespace emri_package
