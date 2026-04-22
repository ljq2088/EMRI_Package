import cmath

import numpy as np

from emri_package import CallableRadialSolver, EMRIPipeline, KerrOrbit


def is_finite_complex(z: complex) -> bool:
    return cmath.isfinite(z.real) and cmath.isfinite(z.imag)


def main():
    orbit = KerrOrbit(M=1.0e6, a=0.9, p=10.0, e=0.3, iota=1.0, mu=0.0)

    radial_solver = CallableRadialSolver(
        rin_fn=lambda r: np.exp(1.0j * 0.02 * r),
        drin_fn=lambda r: 1.0j * 0.02 * np.exp(1.0j * 0.02 * r),
        d2rin_fn=lambda r: -(0.02**2) * np.exp(1.0j * 0.02 * r),
        binc=1.0 + 0.0j,
    )

    pipeline = EMRIPipeline(orbit=orbit, radial_solver=radial_solver)
    result = pipeline.compute_mode(s=-2, ell=2, m=2, omega=0.05, duration=30.0, dt=0.5)
    harmonic = pipeline.compute_harmonic_mode(s=-2, ell=2, m=2, k=0, n=1, duration=30.0, dt=0.5)

    assert len(result.t) > 10
    assert result.W.shape == result.t.shape
    assert result.integrand.shape == result.t.shape
    assert is_finite_complex(result.Z)
    assert np.all(np.isfinite(result.r))
    assert is_finite_complex(harmonic.Z)
    assert harmonic.omega > 0.0
    assert abs(harmonic.omega - (2.0 * harmonic.omega_phi + harmonic.omega_r)) < 1e-13

    print("mode amplitude test passed")
    print(f"points={len(result.t)}")
    print(f"Z={result.Z}")
    print(f"W0={result.W[0]}")
    print(f"omega_harmonic={harmonic.omega}")


if __name__ == "__main__":
    main()
