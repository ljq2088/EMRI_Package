from dataclasses import dataclass
from pathlib import Path
from typing import Protocol

import numpy as np


@dataclass
class RadialModeResult:
    s: int
    ell: int
    m: int
    a: float
    omega: float
    r: object | None = None
    rin: object | None = None
    drin: object | None = None
    d2rin: object | None = None
    binc: complex | None = None


class RadialSolver(Protocol):
    def evaluate_rin(self, s: int, ell: int, m: int, a: float, omega: float, r) -> RadialModeResult:
        ...


class CallableRadialSolver:
    """Lightweight radial adapter backed by Python callables.

    Useful for tests and for wiring in external radial solvers before a native
    backend is finalized.
    """

    def __init__(self, rin_fn, drin_fn, d2rin_fn, binc: complex):
        self._rin_fn = rin_fn
        self._drin_fn = drin_fn
        self._d2rin_fn = d2rin_fn
        self._binc = binc

    def evaluate_rin(self, s: int, ell: int, m: int, a: float, omega: float, r) -> RadialModeResult:
        r_arr = np.asarray(r)
        return RadialModeResult(
            s=s,
            ell=ell,
            m=m,
            a=a,
            omega=omega,
            r=r_arr,
            rin=np.asarray(self._rin_fn(r_arr), dtype=complex),
            drin=np.asarray(self._drin_fn(r_arr), dtype=complex),
            d2rin=np.asarray(self._d2rin_fn(r_arr), dtype=complex),
            binc=complex(self._binc),
        )


class PINNRadialAdapter:
    """Adapter placeholder for future PINN-based `R_in` inference.

    Intended target source:
    `/home/ljq/code/PINN/SolvingTeukolsky`
    """

    def __init__(self, project_root: str | Path = "/home/ljq/code/PINN/SolvingTeukolsky"):
        self.project_root = Path(project_root)

    def available(self) -> bool:
        return self.project_root.exists()

    def evaluate_rin(self, s: int, ell: int, m: int, a: float, omega: float, r) -> RadialModeResult:
        if not self.available():
            raise FileNotFoundError(f"PINN project not found under {self.project_root}")
        raise NotImplementedError(
            "PINN radial inference is not wired into EMRI_Package yet. "
            "This adapter defines the interface that the Teukolsky radiation layer will call."
        )
