# EMRI_Package — Project Brain

## Project Mission

Build a minimal, verifiable EMRI waveform pipeline by integrating three independently
validated layers: **orbit** (geodesic/inspiral), **angular** (spin-weighted spheroidal
harmonics), and **radial** (homogeneous Teukolsky solutions). Current phase prioritizes
integration over reimplementation — wrap and compose existing stable layers first.
Solver, surrogate, and ML modules may reimplement or replace layers once benchmarks
validate the replacement.

## Domain Priorities

1. **Correctness over speed.** Every computation must be traceable to a known analytic
   or numerical reference before optimization is considered.
2. **Kerr spacetime.** The default astrophysical background is Kerr; Schwarzschild and
   flat-space limits exist only as validation checkpoints.
3. **EMRI regime.** Mass ratio μ/M ∈ [1e-7, 1e-4]. Adiabatic inspiral (μ→0) is the
   zero-order baseline; finite-μ corrections are secondary.
4. **Reference standard.** Fujita & Hikida (2009) analytic solutions for bound
   timelike geodesics in Kerr are the primary orbit benchmark.

## Scientific Workflow Contract

### New idea → prior-art check
Before writing any code for a new scientific feature:
1. Search the literature (inspirehep, arXiv) for existing implementations or published
   results that address the same problem.
2. Check `literature/` for already-catalogued relevant papers.
3. If nothing found, write a one-paragraph note in `notes/` summarizing what was
   searched and why the idea is novel (or why reimplementation is justified).

### New implementation → benchmark first
1. Identify or derive the analytic / semi-analytic limit case that the new code must
   reproduce (e.g., circular equatorial, Schwarzschild limit, FH09 frequencies).
2. Write the benchmark test in `tests/` or `benchmarks/` **before** the
   implementation.
3. The benchmark must pass within the agreed tolerance before the feature is
   considered working.

### Every conclusion → labeled
All results, assertions, and claims in code comments, docstrings, notes, and papers
must carry one of three labels:
- `[verified]` — reproduced against a known reference with quantified error
- `[speculative]` — plausible but not yet checked against a reference
- `[to-verify]` — known check that hasn't been run yet; state what check is needed

## Coding Contract

### Language boundaries
- **C++ (C++17):** Numerical kernels (orbit integration, Teukolsky source evaluation).
  Lives under `cpp/emri_package/`. Header-only where possible; `.cpp` for the
  compilation unit.
- **Python (3.10+):** High-level orchestration, data containers, adapters, tests.
  Lives under `src/emri_package/`.
- **pybind11:** The only bridge between C++ and Python. Bindings live in
  `cpp/emri_package/bindings/`.

### Package structure
- `src/emri_package/__init__.py` is the public API surface. Everything importable from
  `emri_package` must be re-exported there.
- Dataclasses for result containers (no logic in containers).
- Protocol classes for injectable solver interfaces (AngularSolver, RadialSolver).
- Adapter classes wrap external dependencies (SWSHLRRAdapter, PINNRadialAdapter).
- No new external Python dependencies without explicit discussion.

### Numerics discipline
- GSL for special functions and root-finding in C++.
- numpy for array operations in Python; no loops over large arrays in pure Python.
- All tolerance thresholds must be documented with their physical justification
  (not arbitrary epsilons).
- `mu=0.0` means pure geodesic; `mu>0` means adiabatic inspiral. Never conflate.

### Build
- `python setup.py build_ext --inplace` for development builds.
- GSL and Eigen paths are configured via environment variables (`EMRI_GSL_ROOT`,
  `EMRI_EIGEN_ROOT`) with hardcoded fallbacks to the author's conda paths.

## Literature Contract

- Primary reference papers live in `literature/` as PDFs or BibTeX entries.
- Every numerical constant or formula traceable to a specific paper must cite that
  paper in a docstring or comment with equation number.
- Key reference: Fujita & Hikida (2009) — analytic solutions for bound timelike
  geodesic orbits in Kerr.
- The `emri-literature-scout` skill handles systematic literature searches.

## Writing Contract

- Paper drafts and notes live in `paper/` and `notes/` respectively.
- Use the `aps-draft` skill for PRD/PRL-formatted writing.
- Before claiming a result in writing, run `citation-audit` to verify all referenced
  equations are correctly cited and reproduced.

## Delegation Policy

The `.claude/` directory contains skill, agent, rule, and hook stubs that will be
populated as the project matures. Currently available skills:

| Skill | When to invoke |
|---|---|
| `prior-art-check` | Before implementing any new scientific feature |
| `benchmark-gate` | Before claiming a feature works |
| `emri-literature-scout` | When searching for relevant papers |
| `theorem-to-code` | When translating an equation from literature to code |
| `aps-draft` | When writing or editing paper drafts |
| `citation-audit` | Before finalizing any written claim |
| `experiment-design` | When designing computational experiments |

**Currently empty stubs** (`.claude/agents/`, `.claude/rules/`, `.claude/hooks/`,
`.claude/skills/*/SKILL.md`): populate only when a concrete, recurring need arises.
Do not expand the `.claude` system speculatively.

## Definition of Done

A feature or fix is **done** when all of the following hold:

1. **Builds clean** — `python setup.py build_ext --inplace` succeeds with no warnings.
2. **Benchmark passes** — the pre-registered benchmark test passes within the stated
   tolerance.
3. **Existing tests pass** — no regression in `tests/`.
4. **Claims are labeled** — all new assertions in code/comments carry
   `[verified]`/`[speculative]`/`[to-verify]`.
5. **API surface updated** — new public symbols appear in `__init__.py` and `__all__`.
6. **No dead code** — adapter stubs and placeholders are deliberate (marked with
   `NotImplementedError` or explicit comments), not abandoned half-implementations.
