---
name: benchmark-gate
description: Pre-completion audit — verifies benchmarks, tests, tolerances, regressions, and label completeness before declaring "done"
triggers:
  - "this feature is done"
  - "ready to merge"
  - "implementation complete"
  - before git merge or PR
---

# Benchmark Gate

## Purpose
Ensure every feature passes quantitative validation before it is declared "done."
This is the executable form of CLAUDE.md's Definition of Done.

## When to Invoke (Mandatory Gate)
- Before claiming any feature/fix is "complete"
- Before merging a feature branch
- Before writing a paper section that depends on the feature's results

## Input
- **Feature description**: what was implemented
- **Expected golden case(s)**: which analytic/numerical limit(s) should it reproduce
- **Declared tolerance(s)**: the numeric threshold(s) for the golden case(s)
- **Affected files**: list of changed files (from `git diff --name-only main..HEAD`)

## Procedure

### 1. Build Check
```bash
python setup.py build_ext --inplace
```
Must compile with **zero errors**. Warnings from external headers (Eigen, GSL) are
acceptable; warnings from `cpp/emri_package/*` are not.

### 2. Golden Case Validation
Run each pre-registered benchmark. For every golden case listed in
`.claude/rules/numerics.md` (or the feature's own benchmark file), verify:
- The benchmark script exits 0
- The measured error is within the declared tolerance
- No `assert` or `raise` fires on the validated path

Minimum gate command:
```bash
PYTHONPATH=src python tests/test_circular_equatorial_benchmark.py
```

### 3. Regression Check
```bash
PYTHONPATH=src python -m pytest tests/ -v 2>&1
```
All existing tests must pass. Any pre-existing failure must be documented in
`notes/known-failures.md` before the current branch touches that test's domain.

### 4. Label Audit
Scan the diff for claims. Every new assertion in code, comments, or docstrings must
carry one of: `[verified]`, `[speculative]`, `[to-verify]`.

Run: `git diff main..HEAD | grep -E 'assert|raise|# |"""' | grep -v '\[verified\]|\[speculative\]|\[to-verify\]'`
If this returns non-empty, flag unlabeled claims.

### 5. API Surface Check
If the diff touches `src/emri_package/`, verify all new public symbols appear in
`__init__.py` and `__all__`.

## Output
A `notes/benchmark-gate/<feature>-<date>.md` gate report:

```markdown
# Benchmark Gate: <feature name>

## 1. Build
- [ ] PASS / FAIL: <build output summary>

## 2. Golden Cases
| Case | Expected | Measured | Tolerance | Status |
|------|----------|----------|-----------|--------|
| ... | ... | ... | ... | PASS/FAIL |

## 3. Regression
- [ ] PASS / FAIL: <N tests passed, M failed>

## 4. Labels
- [ ] PASS / FAIL: all claims labeled
- Unlabeled: <count or "none">

## 5. API
- [ ] PASS / N/A: new symbols in __init__.py and __all__

## Gate Result
- [ ] ALL PASS — feature is [verified]
- [ ] PARTIAL — feature is [speculative], see notes
- [ ] FAILED — do not merge
```

## Minimum Bar for Merge
Golden cases pass, regression passes, labels present. No exceptions.
