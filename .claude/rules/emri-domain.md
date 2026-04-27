---
name: emri-domain
description: EMRI-specific terminology, waveform chain, prior-art requirements, mode labeling conventions
scope:
  src: ["*.py"]
  cpp: ["*.cpp", "*.hpp", "*.h"]
  tests: ["*.py"]
  notes: ["*.md"]
  paper: ["*.tex", "*.md"]
---

# EMRI Domain Rules

## Waveform Chain
The canonical EMRI waveform chain that this package encodes:
```
Orbit(t) → Frequencies(Ω_r, Ω_θ, Ω_φ)
         → Source term (T_lmω)
         → Homogeneous radial solutions (R_in, R_up)
         → Asymptotic amplitudes (Z_lmω)
         → Gravitational waveform (h_+, h_×)
```
Any code that jumps a step or fakes an intermediate quantity must declare which step and why.

## Mode Labeling
- **Standard convention:** (s, l, m, k, n), where:
  - `s = -2` for gravitational waves (the only astrophysically relevant case)
  - `l ≥ 2, |m| ≤ l` are spheroidal indices
  - `k` is the polar harmonic number
  - `n` is the radial harmonic number
- Mode frequency: ω_mkn = m·Ω_φ + k·Ω_θ + n·Ω_r
- Never conflate spheroidal-harmonic `(l,m)` with spherical-harmonic `(l,m)` in Kerr — the distinction matters at a/M > 0.

## Prior Art Baseline
Before implementing any new EMRI feature, the following must be ruled out:
1. **Fujita & Hikida (2009):** Analytic geodesic frequencies and constants of motion.
2. **Schmidt (2002):** Analytic solution for circular equatorial Teukolsky modes.
3. **Detweiler (1978), Sasaki & Nakamura (1982):** Teukolsky homogeneous solutions.
4. **Drasco & Hughes (2006):** Adiabatic inspiral and kludge waveforms.
Check `literature/` for downloaded papers. If a referenced result exists in the literature but is not in `literature/`, add it before implementing.

## Terminology Discipline
| Term | Meaning in this project |
|------|------------------------|
| Geodesic | μ=0, exact Kerr trajectory |
| Inspiral | μ>0, adiabatic evolution |
| Adiabatic | Flux-driven, no self-force or GSF |
| Waveform | h(t) at future null infinity |
| Surrogate | ML-accelerated replacement for a numerically-expensive function (orbit, mode, or waveform) |
| Mode | (s,l,m,k,n) harmonic of the Teukolsky solution |
| Amplitude | Z_lmω, not |h| |
| Overlap | Normalized noise-weighted inner product between two waveforms |

## Mismatch & Detectability
- **Mismatch threshold:** 1 - overlap < 1/(2 SNR²) for indistinguishability (Flanagan & Hughes 1998).
- When comparing models, report mismatch at SNR=20, 50, 100. Don't just say "overlap > 0.99."
