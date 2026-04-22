#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../emri_params.hpp"
#include "../orbit/fundamental_frequencies.hpp"
#include "../orbit/nk_orbit.hpp"

namespace py = pybind11;
using namespace emri_package;

PYBIND11_MODULE(_orbit, m) {
    m.doc() = "EMRI orbit backend";

    py::class_<KerrConstants>(m, "KerrConstants")
        .def(py::init<>())
        .def_readwrite("E", &KerrConstants::E)
        .def_readwrite("Lz", &KerrConstants::Lz)
        .def_readwrite("Q", &KerrConstants::Q)
        .def_readwrite("r3", &KerrConstants::r3)
        .def_readwrite("r4", &KerrConstants::r4)
        .def_readwrite("r_p", &KerrConstants::r_p)
        .def_readwrite("r_a", &KerrConstants::r_a)
        .def_readwrite("z_minus", &KerrConstants::z_minus)
        .def_readwrite("z_plus", &KerrConstants::z_plus)
        .def_readwrite("beta", &KerrConstants::beta);

    py::class_<KerrFrequencies>(m, "KerrFrequencies")
        .def(py::init<>())
        .def_readwrite("Upsilon_r", &KerrFrequencies::Upsilon_r)
        .def_readwrite("Upsilon_theta", &KerrFrequencies::Upsilon_theta)
        .def_readwrite("Upsilon_phi", &KerrFrequencies::Upsilon_phi)
        .def_readwrite("Gamma", &KerrFrequencies::Gamma)
        .def_readwrite("Omega_r", &KerrFrequencies::Omega_r)
        .def_readwrite("Omega_theta", &KerrFrequencies::Omega_theta)
        .def_readwrite("Omega_phi", &KerrFrequencies::Omega_phi);

    py::class_<OrbitState>(m, "OrbitState")
        .def_readonly("t", &OrbitState::t)
        .def_readonly("p", &OrbitState::p)
        .def_readonly("e", &OrbitState::e)
        .def_readonly("iota", &OrbitState::iota)
        .def_readonly("psi", &OrbitState::psi)
        .def_readonly("chi", &OrbitState::chi)
        .def_readonly("phi", &OrbitState::phi)
        .def_readonly("r", &OrbitState::r)
        .def_readonly("theta", &OrbitState::theta);

    py::class_<OrbitKinematics>(m, "OrbitKinematics")
        .def_readonly("t", &OrbitKinematics::t)
        .def_readonly("p", &OrbitKinematics::p)
        .def_readonly("e", &OrbitKinematics::e)
        .def_readonly("iota", &OrbitKinematics::iota)
        .def_readonly("psi", &OrbitKinematics::psi)
        .def_readonly("chi", &OrbitKinematics::chi)
        .def_readonly("phi", &OrbitKinematics::phi)
        .def_readonly("r", &OrbitKinematics::r)
        .def_readonly("theta", &OrbitKinematics::theta)
        .def_readonly("E", &OrbitKinematics::E)
        .def_readonly("Lz", &OrbitKinematics::Lz)
        .def_readonly("Q", &OrbitKinematics::Q)
        .def_readonly("ut", &OrbitKinematics::ut)
        .def_readonly("ur", &OrbitKinematics::ur)
        .def_readonly("utheta", &OrbitKinematics::utheta)
        .def_readonly("uphi", &OrbitKinematics::uphi)
        .def_readonly("sigma", &OrbitKinematics::sigma)
        .def_readonly("delta", &OrbitKinematics::delta)
        .def_readonly("P", &OrbitKinematics::P);

    py::class_<BabakNKOrbit>(m, "BabakNKOrbitCPP")
        .def(py::init<double, double, double, double, double, double>(),
             py::arg("M"),
             py::arg("a"),
             py::arg("p"),
             py::arg("e"),
             py::arg("iota"),
             py::arg("mu") = 0.0)
        .def("evolve", &BabakNKOrbit::evolve, py::arg("duration"), py::arg("dt"))
        .def("get_current_state", &BabakNKOrbit::get_current_state)
        .def("get_current_kinematics", &BabakNKOrbit::get_current_kinematics)
        .def("get_state_kinematics", &BabakNKOrbit::get_state_kinematics, py::arg("state"))
        .def_static("get_conserved_quantities", &BabakNKOrbit::get_conserved_quantities,
                    py::arg("M"),
                    py::arg("a"),
                    py::arg("p"),
                    py::arg("e"),
                    py::arg("iota"),
                    py::arg("E_g") = 0.0,
                    py::arg("L_g") = 0.0,
                    py::arg("Q_g") = 0.0);

    m.def("get_conserved_quantities_cpp",
          [](double M, double a, double p, double e, double iota) {
              return BabakNKOrbit::get_conserved_quantities(M, a, p, e, iota);
          });

    m.def("compute_fundamental_frequencies_cpp",
          &KerrFundamentalFrequencies::compute,
          py::arg("a"),
          py::arg("E"),
          py::arg("Lz"),
          py::arg("Q"));
    m.def("compute_fundamental_frequencies_from_pei_cpp",
          &KerrFundamentalFrequencies::compute_from_pei,
          py::arg("a"),
          py::arg("p"),
          py::arg("e"),
          py::arg("iota"));
    m.def("compute_fundamental_frequencies_from_pei_constants_cpp",
          &KerrFundamentalFrequencies::compute_from_pei_constants,
          py::arg("a"),
          py::arg("p"),
          py::arg("e"),
          py::arg("iota"),
          py::arg("E"),
          py::arg("Lz"),
          py::arg("Q"));
}
