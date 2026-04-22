from dataclasses import dataclass

import numpy as np

from .orbit import KerrOrbit
from .radial import RadialSolver
from .source import KerrGeo, SWSH, TeukolskySource


@dataclass
class ModeAmplitudeResult:
    s: int
    ell: int
    m: int
    omega: float
    t: np.ndarray
    r: np.ndarray
    phi: np.ndarray
    W: np.ndarray
    phase: np.ndarray
    integrand: np.ndarray
    Z: complex
    binc: complex


@dataclass
class HarmonicModeAmplitudeResult(ModeAmplitudeResult):
    k: int
    n: int
    omega_r: float
    omega_theta: float
    omega_phi: float


def harmonic_mode_frequency(orbit: KerrOrbit, m: int, k: int, n: int) -> tuple[float, object]:
    freqs = orbit.fundamental_frequencies()
    omega = m * freqs.Omega_phi + k * freqs.Omega_theta + n * freqs.Omega_r
    return omega, freqs


def compute_mode_amplitude(
    orbit: KerrOrbit,
    radial_solver: RadialSolver,
    s: int,
    ell: int,
    m: int,
    omega: float,
    duration: float,
    dt: float,
) -> ModeAmplitudeResult:
    traj, kin = orbit.evolve_with_kinematics(duration=duration, dt=dt)
    if len(traj.t) == 0:
        raise ValueError("Orbit evolution returned no samples")

    radial = radial_solver.evaluate_rin(s=s, ell=ell, m=m, a=orbit.a, omega=omega, r=traj.r)
    if radial.rin is None or radial.drin is None or radial.d2rin is None:
        raise ValueError("Radial solver must provide rin, drin, and d2rin")
    if radial.binc is None:
        raise ValueError("Radial solver must provide B_inc for Eq. (43)")

    source = TeukolskySource(orbit.a, omega, s, ell, m)
    swsh = SWSH(s, ell, m, orbit.a * omega)

    W = np.empty(len(traj.t), dtype=complex)
    for i in range(len(traj.t)):
        geo = KerrGeo(orbit.a, float(kin.E[i]), float(kin.Lz[i]), float(kin.Q[i]))
        state = KerrGeo.State()
        state.x = [float(traj.t[i]), float(traj.r[i]), float(traj.theta[i]), float(traj.phi[i])]
        state.u = [float(kin.ut[i]), float(kin.ur[i]), float(kin.utheta[i]), float(kin.uphi[i])]
        W[i] = source.ComputeW(
            state,
            geo,
            swsh,
            complex(radial.rin[i]),
            complex(radial.drin[i]),
            complex(radial.d2rin[i]),
        )

    phase = np.exp(1.0j * omega * traj.t - 1.0j * m * traj.phi)
    integrand = phase * W
    Z = np.trapezoid(integrand, traj.t) / (2.0j * omega * complex(radial.binc))

    return ModeAmplitudeResult(
        s=s,
        ell=ell,
        m=m,
        omega=omega,
        t=traj.t,
        r=traj.r,
        phi=traj.phi,
        W=W,
        phase=phase,
        integrand=integrand,
        Z=Z,
        binc=complex(radial.binc),
    )


def compute_mode_amplitude_harmonic(
    orbit: KerrOrbit,
    radial_solver: RadialSolver,
    s: int,
    ell: int,
    m: int,
    k: int,
    n: int,
    duration: float,
    dt: float,
) -> HarmonicModeAmplitudeResult:
    omega, freqs = harmonic_mode_frequency(orbit=orbit, m=m, k=k, n=n)
    base = compute_mode_amplitude(
        orbit=orbit,
        radial_solver=radial_solver,
        s=s,
        ell=ell,
        m=m,
        omega=omega,
        duration=duration,
        dt=dt,
    )
    return HarmonicModeAmplitudeResult(
        s=base.s,
        ell=base.ell,
        m=base.m,
        omega=base.omega,
        t=base.t,
        r=base.r,
        phi=base.phi,
        W=base.W,
        phase=base.phase,
        integrand=base.integrand,
        Z=base.Z,
        binc=base.binc,
        k=k,
        n=n,
        omega_r=freqs.Omega_r,
        omega_theta=freqs.Omega_theta,
        omega_phi=freqs.Omega_phi,
    )
