---
name: prior-art-check
description: Gate for new ideas — checks literature coverage, identifies differences, surfaces risks before implementation
triggers:
  - "I have a new idea"
  - "let's implement X"
  - "has anyone done this"
  - "should we try Y approach"
---

# Prior Art Check

## Purpose
Prevent wasted implementation effort by verifying an idea is genuinely novel (or
justifiably reimplemented) before a single line of code is written.

## When to Invoke (Mandatory Gate)
Per CLAUDE.md Scientific Workflow Contract: invoke this **before** implementing
any new scientific feature, algorithm, surrogate model, or numerical method.

## Input
- **Idea summary**: 1-3 sentences describing what you want to build
- **Expected novelty**: what makes it different from existing work
- **Assumptions about prior art**: what you believe already exists (even if vague)
- **Scope context**: which part of the waveform chain does it touch? (orbit / angular / radial / source / waveform / PE / surrogate)

## Procedure
1. **Run `emri-literature-scout`** for the topic. If no relevant papers found, flag `literature-gap`.
2. **Check the 5 domain anchors** (FH09, Schmidt, Detweiler/SN, Drasco+Hughes, Flanagan+Hughes) — does this idea replicate or extend any of them?
3. **Check `literature/`** for already-catalogued work.
4. **Identify the closest prior work** — the one paper most similar to the proposed idea.

## Output
Write a structured note to `notes/prior-art/<slug>-<date>.md`:

```markdown
# Prior Art: <idea name>

## Closest Prior Work
- **Paper:** <cite with equation number if relevant>
- **Similarity:** What they did that overlaps
- **Difference:** What we propose that they did NOT do

## Novelty Assessment
- [ ] Truly novel — no published work found
- [ ] Known approach, new regime/parameter range — justify
- [ ] Reimplementation for benchmarking — justify why needed
- [ ] Direct duplication — STOP, reconsider

## Risk Factors
- Numerics: convergence issues, stiff ODE, phase wrapping?
- Theory: approximation breaks down at high a/e?
- Data: surrogate needs training set — do we have a generator?
- Comparison: can we benchmark against an existing code?

## Decision
- [ ] Proceed (prior art gap confirmed)
- [ ] Proceed with caveats (list them)
- [ ] Blocked by prior work (stop, re-scope, or re-benchmark against existing)
```

## Red Flags (automatic BLOCK)
- Identical approach published with working code and no significant regime difference
- Idea touches the separatrix/plunge regime without a plan for the singular boundary
- Surrogate claims "speedup" without an existing slow-but-correct numerical solver to train on
