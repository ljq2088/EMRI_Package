import cmath

from emri_package import KerrGeo, KerrOrbit, SWSH, TeukolskySource


def is_finite_complex(z: complex) -> bool:
    return cmath.isfinite(z.real) and cmath.isfinite(z.imag)


def main():
    orbit = KerrOrbit(M=1.0e6, a=0.9, p=10.0, e=0.3, iota=1.0, mu=0.0)
    _, kin = orbit.evolve_with_kinematics(duration=20.0, dt=0.5)
    idx = len(kin.t) // 2

    geo = KerrGeo(orbit.a, float(kin.E[idx]), float(kin.Lz[idx]), float(kin.Q[idx]))
    state = KerrGeo.State()
    state.x = [float(kin.t[idx]), float(kin.r[idx]), float(kin.theta[idx]), float(kin.phi[idx])]
    state.u = [float(kin.ut[idx]), float(kin.ur[idx]), float(kin.utheta[idx]), float(kin.uphi[idx])]

    omega = 0.05
    swsh = SWSH(-2, 2, 2, orbit.a * omega)
    source = TeukolskySource(orbit.a, omega, -2, 2, 2)

    proj = source.ComputeProjections(state, geo, swsh)
    legacy = source.ComputeProjectionsLegacy(state, geo, swsh)
    R_in = 1.2 + 0.3j
    dR_dr = -0.4 + 0.1j
    d2R_dr2 = 0.05 - 0.02j
    integrand = source.ComputeIntegrand(state, geo, swsh, R_in, dR_dr, d2R_dr2)
    W_direct = source.ComputeW(state, geo, swsh, R_in, dR_dr, d2R_dr2)

    for value in [
        proj.A_nn0,
        proj.A_mbarn0,
        proj.A_mbarn1,
        proj.A_mbarmbar0,
        proj.A_mbarmbar1,
        proj.A_mbarmbar2,
    ]:
        assert is_finite_complex(value)

    diff = abs(proj.A_mbarn0 - legacy.A_mbarn0) + abs(proj.A_mbarmbar0 - legacy.A_mbarmbar0)
    assert diff > 1e-14
    assert abs(integrand.W - W_direct) < 1e-13
    assert is_finite_complex(integrand.W)

    print("teukolsky source test passed")
    print(f"A_nn0={proj.A_nn0}")
    print(f"A_mbarn0(refactored)={proj.A_mbarn0}")
    print(f"A_mbarn0(legacy)={legacy.A_mbarn0}")
    print(f"W={integrand.W}")


if __name__ == "__main__":
    main()
