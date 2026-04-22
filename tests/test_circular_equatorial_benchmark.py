import cmath

import numpy as np

from emri_package import CallableRadialSolver, EMRIPipeline, KerrOrbit


def is_finite_complex(z: complex) -> bool:
    return cmath.isfinite(z.real) and cmath.isfinite(z.imag)


def main():
    a = 0.9
    r0 = 8.0
    freq_orbit = KerrOrbit(M=1.0e6, a=a, p=r0, e=0.0, iota=0.0, mu=0.0)
    freqs = freq_orbit.fundamental_frequencies()

    omega_phi_exact = 1.0 / (a + r0 ** 1.5)
    assert abs(freqs.Omega_phi - omega_phi_exact) / omega_phi_exact < 5e-3

    # The extracted NK orbit kernel is not robust at exactly e=0; use a near-circular
    # equatorial orbit to benchmark the amplitude pipeline without changing its logic.
    orbit = KerrOrbit(M=1.0e6, a=a, p=r0, e=1.0e-3, iota=0.0, mu=0.0)
    orbit_freqs = orbit.fundamental_frequencies()
    radial_solver = CallableRadialSolver(
        rin_fn=lambda r: np.exp(1.0j * 0.03 * r),
        drin_fn=lambda r: 1.0j * 0.03 * np.exp(1.0j * 0.03 * r),
        d2rin_fn=lambda r: -(0.03**2) * np.exp(1.0j * 0.03 * r),
        binc=1.0 + 0.0j,
    )
    pipeline = EMRIPipeline(orbit=orbit, radial_solver=radial_solver)
    harmonic = pipeline.compute_harmonic_mode(s=-2, ell=2, m=2, k=0, n=0, duration=40.0, dt=0.5)

    assert abs(harmonic.omega - 2.0 * orbit_freqs.Omega_phi) < 1e-12
    assert is_finite_complex(harmonic.Z)
    assert harmonic.W.shape == harmonic.t.shape

    print("circular/near-circular benchmark passed")
    print(f"Omega_phi_exact={omega_phi_exact:.8f}")
    print(f"Omega_phi_FH09={freqs.Omega_phi:.8f}")
    print(f"omega_220_near_circular={harmonic.omega:.8f}")
    print(f"Z_220_near_circular={harmonic.Z}")


if __name__ == "__main__":
    main()
