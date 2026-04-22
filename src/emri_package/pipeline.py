from dataclasses import dataclass

from .amplitude import (
    HarmonicModeAmplitudeResult,
    ModeAmplitudeResult,
    compute_mode_amplitude,
    compute_mode_amplitude_harmonic,
)
from .angular import AngularSolver
from .orbit import KerrOrbit, OrbitTrajectory
from .radial import RadialSolver


@dataclass
class EMRIPipeline:
    """Top-level assembly point for future Teukolsky waveform generation.

    Current implementation only exposes orbit generation. Angular and radial
    solvers are injected so the public composition point is fixed early.
    """

    orbit: KerrOrbit
    angular_solver: AngularSolver | None = None
    radial_solver: RadialSolver | None = None

    def generate_orbit(self, duration: float, dt: float) -> OrbitTrajectory:
        return self.orbit.evolve(duration=duration, dt=dt)

    def compute_mode(
        self,
        s: int,
        ell: int,
        m: int,
        omega: float,
        duration: float,
        dt: float,
    ) -> ModeAmplitudeResult:
        if self.radial_solver is None:
            raise ValueError("A radial solver is required to compute mode amplitudes")
        return compute_mode_amplitude(
            orbit=self.orbit,
            radial_solver=self.radial_solver,
            s=s,
            ell=ell,
            m=m,
            omega=omega,
            duration=duration,
            dt=dt,
        )

    def compute_harmonic_mode(
        self,
        s: int,
        ell: int,
        m: int,
        k: int,
        n: int,
        duration: float,
        dt: float,
    ) -> HarmonicModeAmplitudeResult:
        if self.radial_solver is None:
            raise ValueError("A radial solver is required to compute mode amplitudes")
        return compute_mode_amplitude_harmonic(
            orbit=self.orbit,
            radial_solver=self.radial_solver,
            s=s,
            ell=ell,
            m=m,
            k=k,
            n=n,
            duration=duration,
            dt=dt,
        )
