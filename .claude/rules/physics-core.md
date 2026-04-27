---
name: physics-core
description: Symbol conventions, validity domains, physical sanity checks for EMRI/Kerr computations
scope:
  src: ["*.py"]
  cpp: ["*.cpp", "*.hpp", "*.h"]
  tests: ["*.py"]
  notes: ["*.md"]
---

# Physics Core Rules

## Units & Conventions
- **Default units:** G = c = 1. State explicitly when using physical units (e.g., solar masses, seconds).
- **Metric signature:** (- + + +). Any sign flip is a bug until proven otherwise.
- **Boyer-Lindquist coordinates** are the canonical coordinate system for Kerr. Other coordinates (ingoing/outgoing Kerr, harmonic) must be explicitly named and documented.

## Kerr Parameters
- **M** is the total mass. Normalization M=1 is allowed **only** for pure-geodesic computations; state `M=1` explicitly.
- **a** is spin parameter: 0 ≤ |a| ≤ M. Computations with |a| > M are nonsense.
- **Spin anchor:** when in doubt, use a=0.9M as the non-trivial Kerr test case. Schwarzschild (a=0) validates correctness of the limit; a=0.9 validates the Kerr logic.

## Motion Constants
- **E, Lz, Q** are the three constants of geodesic motion in Kerr.
- Every orbit evolution must monitor drift in these quantities. If `mu=0`, drift is purely numerical.
- If any constant drifts by more than 1% over the integration window, the trajectory is [to-verify].

## Validity Domains
- **Bound orbits:** p > p_separatrix(a, e, iota). Orbits at or inside the separatrix are plunging — flag as [speculative] unless plunge logic is explicitly coded.
- **μ regime:** μ = 0 is geodesic. μ ∈ [1e-7, 1e-4] is EMRI adiabatic inspiral. μ > 1e-3 is intermediate mass ratio — flag as [speculative].
- **Eccentricity:** e ∈ [0, 1). e=0 is analytically convenient but may trigger numerical degeneracies — prefer e=1e-3 as the "effectively circular" test case.

## Sanity Checks
- **Horizon:** r(t) > r_+(a) = M + sqrt(M² - a²) for all t.
- **θ range:** θ ∈ [0, π]. No θ wrapping, no reflection at poles — the ODE stays in the fundamental domain.
- **Finite results:** `assert np.all(np.isfinite(r))` on every trajectory before passing it to downstream modules.
- **Frequency ordering:** Ω_φ > Ω_θ > Ω_r for generic bound Kerr orbits. Violations signal root-finding or mapping bugs.
