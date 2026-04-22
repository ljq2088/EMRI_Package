#pragma once

#include <string>
#include <vector>

namespace emri_package {

struct KerrConstants {
    double E;
    double Lz;
    double Q;
    double r3;
    double r4;
    double r_p;
    double r_a;
    double z_minus;
    double z_plus;
    double beta;
};

struct EMRIParams {
    double M;
    double mu;
    double a;
    double p0;
    double e0;
    double iota0;
    double T;
    double dt;
    bool use_eccentric;
    bool use_equatorial;

    EMRIParams();
};

struct WaveformConfig {
    bool return_orbit;
    bool return_polarizations;
    bool return_tdi_channels;
    std::string tdi_mode;

    WaveformConfig();
};

}  // namespace emri_package
