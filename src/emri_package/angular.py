from dataclasses import dataclass
from pathlib import Path
from typing import Protocol

from ._teukolsky import SWSH


@dataclass
class AngularModeResult:
    s: int
    ell: int
    m: int
    a_omega: float
    lambda_sep: float
    value: complex | None = None
    l2dag_value: complex | None = None
    l1dag_l2dag_value: complex | None = None


class AngularSolver(Protocol):
    def solve(self, s: int, ell: int, m: int, a_omega: float, x: float | None = None) -> AngularModeResult:
        ...


class SWSHLRRAdapter:
    """Thin wrapper over the compiled `SWSH_LRR` backend."""

    def __init__(self, source_root: str | Path = "/home/ljq/code/Teukolsky_based/GremLinEqRe"):
        self.source_root = Path(source_root)
        self.header = self.source_root / "include" / "SWSH_LRR.h"
        self.source = self.source_root / "src" / "SWSH_LRR.cpp"

    def available(self) -> bool:
        return self.header.exists() and self.source.exists()

    def solve(self, s: int, ell: int, m: int, a_omega: float, x: float | None = None) -> AngularModeResult:
        if not self.available():
            raise FileNotFoundError(f"SWSH_LRR source not found under {self.source_root}")
        swsh = SWSH(s, ell, m, a_omega)
        result = AngularModeResult(s=s, ell=ell, m=m, a_omega=a_omega, lambda_sep=swsh.m_lambda)
        if x is not None:
            result.value = swsh.evaluate_S(x)
            result.l2dag_value = swsh.evaluate_L2dag_S(x)
            result.l1dag_l2dag_value = swsh.evaluate_L1dag_L2dag_S(x)
        return result
