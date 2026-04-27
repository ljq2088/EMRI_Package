---
name: repo-ops
description: Git discipline, benchmark gate enforcement, file protection, and change scope control
scope:
  "": ["*"]   # repo-wide
---

# Repo Ops Rules

## Git Discipline
- **One logical change per commit.** If the commit message needs "and" to join two unrelated things, split.
- Commit messages in imperative mood: "add mode amplitude computation" not "added mode amplitude."
- Never commit: `.so` files, `__pycache__/`, build artifacts, `.ipynb_checkpoints/`, model weights.
- Before committing: `git diff --staged` to review what's actually going in.

## Benchmark Gate
- The benchmark gate is not optional. Every feature touching numerics, physics, or ML must have a corresponding benchmark that passes before merge.
- Run: `PYTHONPATH=src python tests/test_circular_equatorial_benchmark.py` as the minimum gate. If this fails, the branch is not mergeable.
- New features without a benchmark test are [to-verify] by definition.

## Protected Files
The following files define the project's contract and MUST NOT be casually modified:
- `setup.py` — build system and extension module definitions
- `cpp/emri_package/include/*.h` — public C++ API headers
- `src/emri_package/__init__.py` — public Python API surface

Changes to protected files require: (a) an explicit reason in the commit message, and (b) passing the full test suite.

## Change Scope
- **Do not expand the scope of a task beyond what was asked.**
- A bug fix does not need surrounding cleanup. A one-shot script does not need a helper module.
- If you spot an unrelated issue during work, note it in `notes/` — do NOT fix it in the same branch.
- `.claude/` stubs (agents, hooks, skills) are populated on-demand only. No speculative population.

## Build Must Stay Green
- `python setup.py build_ext --inplace` must succeed with zero warnings.
- If an extension fails to compile, fix it before touching any Python code that depends on it.
- Cross-platform: the package is developed on Linux. macOS compilation is aspirational, not required.
