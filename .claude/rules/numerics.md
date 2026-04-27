---
name: numerics
description: Numerical convergence, invariant preservation, golden cases, and tolerance discipline
scope:
  cpp: ["*.cpp", "*.hpp", "*.h"]
  src: ["*.py"]
  tests: ["test_*.py"]
  benchmarks: ["*.py"]
---

# Numerics Rules

## Convergence
- **Integration convergence must be demonstrated, not assumed.** When implementing an ODE/solver, run at 3+ values of dt (or tolerance) and show the quantity of interest converges at the expected order.
- For NK orbit integration: vary `dt` by factor 2× and verify that errors in E, Lz, Q scale accordingly.
- For frequency root-finding: vary the bracket tolerance and confirm Ω values stabilize within the declared tolerance.

## Invariant Preservation
- **E, Lz, Q** are the three motion constants. Monitor their fractional drift:
  ```
  δE = max_t |E(t) - E(0)| / |E(0)|
  δLz = max_t |Lz(t) - Lz(0)| / |Lz(0)|
  δQ = max_t |Q(t) - Q(0)| / (|Q(0)| + 1)   # Q can be zero
  ```
- Declaration requirement: any trajectory with δ > 1e-3 is [speculative]; δ > 1e-2 is a bug.

## Golden Cases (Must-Pass Tests)
Every numerical routine must validate against AT LEAST ONE of:

| Case | What it checks | Tolerance |
|------|---------------|-----------|
| Circular equatorial (a=0) | Schwarzschild Ω_φ = M^(1/2) / r^(3/2) | 1e-6 |
| Circular equatorial (a=0.9) | Analytic Ω_φ | 1e-3 |
| FH09 frequencies | Generic bound orbit Ω_r, Ω_θ, Ω_φ | 1e-3 |
| e → 0 limit | Orbits with e=1e-3 match e=0 analytic limit | 1e-2 |

New golden cases must be added when a new physical regime is entered (e.g., inclined orbits, high-e orbits).

## Tolerance Discipline
- Every numerical tolerance in the code must have a physical justification in a comment or docstring.
- "1e-12 because double precision" is acceptable at the machine-precision level.
- "1e-3 because that's what we used before" is **not** acceptable.
- State the observable consequence of violating the tolerance (e.g., "mismatch < 1e-3 at SNR=50").

## Complex Phase & Wrapping
- Phase = exp(i·ω·t - i·m·φ). Branch cuts at ±π are a known numerical hazard.
- When integrating oscillatory integrands over long durations, verify the phase is correctly unwrapped before computing Z_lmω.
- Check: `np.abs(np.diff(phase) / phase[:-1]) < threshold` to detect 2π jumps.
