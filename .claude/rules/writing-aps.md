---
name: writing-aps
description: APS-style (PRD/PRL) writing conventions — claim calibration, figure narrative, equation discipline
scope:
  paper: ["*.tex", "*.md", "*.pdf"]
  notes: ["*.md"]
---

# APS Writing Rules

## Claim Calibration
- **One strong quantitative claim per paragraph.** If a paragraph introduces two new claims, split it.
- Every claim must be traceable to either: (a) a specific equation in the manuscript, (b) a figure panel, or (c) a cited reference with equation number.
- Hedging vocabulary is fine — but assign strength: "likely" ≠ "we find" ≠ "we prove." Match the hedge to the evidence level.

## Equation Discipline
- Every symbol in every equation must be defined **before** or **immediately after** its first appearance.
- Physical quantities carry units. "r=10" is ambiguous; "r=10M" or "r=10 (in units where G=c=M=1)" is not.
- Number equations that are referenced later. Display equations are not decoration.

## Figure Narrative
- Every figure panel must earn its space. If a panel doesn't convey a specific, named conclusion, merge it or cut it.
- Caption style: first sentence = what the reader should conclude. Remaining sentences = how the figure was generated.
- Axes labels: `r/M`, `Ω_φ M`, `t/M` — use physical normalizations, not raw code variable names.

## Citation Discipline
- Cite the equation, not just the paper. `Fujita & Hikida (2009), Eq. (27)` is correct. `[12]` without equation is not.
- Prior work that you directly compare against must be cited in the introduction AND where the comparison occurs.
- If a result contradicts a prior paper, state the contradiction explicitly and quantify the disagreement.

## PRD vs PRL
| Aspect | PRD | PRL |
|--------|-----|-----|
| Length | ~15-20 pages | 4 pages + supplements |
| Depth | Full derivations, extensive appendices | Key results, derivations in supplement |
| Figures | 10-15 figures with multipanel | 3-4 figures |
| Claim density | Comprehensive, systematic | One headline result |
| This project's default | Yes — main publication | Possible — letter on key result |

Default target: PRD. Use `aps-draft` skill for template-compliant writing.

## Before Submission Checklist
- [ ] `citation-audit` passes — all cited equations are correct and reproducible
- [ ] All figures have physically-labeled axes
- [ ] All units stated explicitly or via normalization convention
- [ ] Every `[verified]` claim in code has a corresponding figure or table
- [ ] All `[speculative]` claims are clearly hedged in the text
