#include "emri_params.hpp"

namespace emri_package {

EMRIParams::EMRIParams()
    : M(1.0e6),
      mu(0.0),
      a(0.0),
      p0(10.0),
      e0(0.0),
      iota0(0.0),
      T(1.0e4),
      dt(1.0),
      use_eccentric(true),
      use_equatorial(false) {}

WaveformConfig::WaveformConfig()
    : return_orbit(true),
      return_polarizations(false),
      return_tdi_channels(false),
      tdi_mode("none") {}

}  // namespace emri_package
