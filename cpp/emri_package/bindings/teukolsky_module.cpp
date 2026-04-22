#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../include/KerrGeo.h"
#include "../include/SWSH_LRR.h"
#include "../include/TeukolskySource.h"

namespace py = pybind11;

PYBIND11_MODULE(_teukolsky, m) {
    m.doc() = "EMRI Teukolsky angular and source backend";

    py::class_<KerrGeo> geo(m, "KerrGeo");
    py::class_<KerrGeo::State>(geo, "State")
        .def(py::init<>())
        .def_readwrite("x", &KerrGeo::State::x)
        .def_readwrite("u", &KerrGeo::State::u);

    geo.def(py::init<Real, double, double, double>(),
            py::arg("a"),
            py::arg("E"),
            py::arg("Lz"),
            py::arg("Q"))
        .def("update_kinematics",
             &KerrGeo::update_kinematics,
             py::arg("state"),
             py::arg("r_direction") = 0.0,
             py::arg("theta_direction") = 0.0)
        .def_static("from_circular_equatorial",
                    &KerrGeo::from_circular_equatorial,
                    py::arg("a"),
                    py::arg("r"),
                    py::arg("is_prograde"))
        .def_property_readonly("energy", &KerrGeo::energy)
        .def_property_readonly("angular_momentum", &KerrGeo::angular_momentum)
        .def_property_readonly("carter_constant", &KerrGeo::carter_constant)
        .def_property_readonly("spin", &KerrGeo::spin)
        .def("potential_r", &KerrGeo::potential_r)
        .def("diff_potential_r", &KerrGeo::diff_potential_r)
        .def("diff2_potential_r", &KerrGeo::diff2_potential_r)
        .def("potential_theta", &KerrGeo::potential_theta);

    py::class_<SWSH>(m, "SWSH")
        .def(py::init<int, int, int, double>(),
             py::arg("s"),
             py::arg("l"),
             py::arg("m"),
             py::arg("a_omega"))
        .def_property_readonly("m_lambda", &SWSH::get_lambda)
        .def_property_readonly("E", &SWSH::get_E)
        .def_property_readonly("A", &SWSH::get_A)
        .def_property_readonly("l", &SWSH::get_l)
        .def_property_readonly("m", &SWSH::get_m)
        .def_property_readonly("s", &SWSH::get_s)
        .def_property_readonly("aw", &SWSH::get_aw)
        .def("evaluate_S", &SWSH::evaluate_S)
        .def("evaluate_L2dag_S", &SWSH::evaluate_L2dag_S)
        .def("evaluate_L1dag_L2dag_S", &SWSH::evaluate_L1dag_L2dag_S)
        .def_static("spin_weighted_Y",
                    &SWSH::spin_weighted_Y,
                    py::arg("s"),
                    py::arg("l"),
                    py::arg("m"),
                    py::arg("x"));

    py::class_<SourceProjections>(m, "SourceProjections")
        .def(py::init<>())
        .def_readwrite("A_nn0", &SourceProjections::A_nn0)
        .def_readwrite("A_mbarn0", &SourceProjections::A_mbarn0)
        .def_readwrite("A_mbarn1", &SourceProjections::A_mbarn1)
        .def_readwrite("A_mbarmbar0", &SourceProjections::A_mbarmbar0)
        .def_readwrite("A_mbarmbar1", &SourceProjections::A_mbarmbar1)
        .def_readwrite("A_mbarmbar2", &SourceProjections::A_mbarmbar2);

    py::class_<SourceIntegrand>(m, "SourceIntegrand")
        .def(py::init<>())
        .def_readwrite("projections", &SourceIntegrand::projections)
        .def_readwrite("W", &SourceIntegrand::W);

    py::class_<TeukolskySource>(m, "TeukolskySource")
        .def(py::init<Real, Real, int, int, int>(),
             py::arg("a"),
             py::arg("omega"),
             py::arg("s"),
             py::arg("l"),
             py::arg("m"))
        .def("ComputeProjections",
             &TeukolskySource::ComputeProjections,
             py::arg("geo_state"),
             py::arg("geo_obj"),
             py::arg("swsh"))
        .def("ComputeProjectionsLegacy",
             &TeukolskySource::ComputeProjectionsLegacy,
             py::arg("geo_state"),
             py::arg("geo_obj"),
             py::arg("swsh"))
        .def("ComputeW",
             &TeukolskySource::ComputeW,
             py::arg("geo_state"),
             py::arg("geo_obj"),
             py::arg("swsh"),
             py::arg("R_in"),
             py::arg("dR_dr"),
             py::arg("d2R_dr2"))
        .def("ComputeIntegrand",
             &TeukolskySource::ComputeIntegrand,
             py::arg("geo_state"),
             py::arg("geo_obj"),
             py::arg("swsh"),
             py::arg("R_in"),
             py::arg("dR_dr"),
             py::arg("d2R_dr2"));
}
