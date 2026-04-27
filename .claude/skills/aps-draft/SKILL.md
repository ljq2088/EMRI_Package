---
name: aps-draft
description: Draft APS-style (PRD/PRL) sections — abstract, intro, methods, results, figure captions, related work
triggers:
  - "draft an abstract"
  - "write the introduction"
  - "write a figure caption"
  - "draft the related work section"
  - "write the methods section"
---

# APS Draft

## Purpose
Produce APS-compliant (PRD default, PRL optional) text for EMRI physics papers.
Every output block is structured, citable, and calibrated to the evidence level.

## When to Invoke
- Writing or revising any section of a paper draft
- Drafting figure captions
- Writing related work comparisons
- Preparing abstract for submission

## Input
- **Section type**: abstract / intro / methods / results / discussion / related-work / figure-caption / appendix
- **Key finding(s)**: 1-3 quantitative results to convey
- **Target journal**: PRD (default) or PRL
- **Evidence level** per finding: [verified] / [speculative] / [to-verify]
- **Relevant figure/table references** (if any)
- **Key citations**: prior work to compare against (with equation numbers)

## Output Structure Per Section Type

### Abstract (PRD, ~250 words)
```
Context: 1 sentence — why EMRI waveforms matter
Problem: 1 sentence — what gap this fills
Method:  1-2 sentences — orbit model + Teukolsky + what's new
Results: 2-3 sentences — one quantitative result each, with numbers
Impact:  1 sentence — implication for LISA/PE/surrogate modeling
```

### Introduction (PRD)
```
P1: EMRI science case + LISA context
P2: Waveform modeling landscape (cite key gaps)
P3: THIS paper's approach — what we do differently
P4: Roadmap of sections
```

### Figure Caption
```
Sentence 1: What the reader should conclude from this figure.
Sentence 2+: How it was generated (orbit params, M, a, p, e, duration, dt).
Parenthetical: "(s,l,m,k,n) = (-2,2,2,0,0) mode" — always declare mode labels.
```

### Related Work
For each prior work cited:
```
<Author (Year)> <approach>. <key result>. 
Difference from this work: <quantified discrepancy or regime difference>.
```

## Evidence Calibration (per writing-aps rule)
Match hedging to the label:
| Label | Allowed hedging | Forbidden |
|-------|----------------|-----------|
| [verified] | "we find", "we demonstrate" | "may", "could", "possibly" |
| [speculative] | "suggests", "indicates", "appears to" | "we prove", "we establish" |
| [to-verify] | "preliminary", "tentatively" | any definitive claim |

## Equation Formatting
- Every equation: number it if cited later. Define all symbols in the surrounding text.
- Orbit params: `(M, a, p, e, ι)` — declared once in Methods.
- Mode labels: `(s, l, m, k, n)` — declared once in Methods.
- Physical normalizations: `r/M`, `Ω_φ M`, `t/M`.

## Citation Format
Use `Author (Year), Eq. (N)` inline. Not `[12]`.

## Output Destination
- Paper text → `paper/sections/<section>-draft-v<N>.tex` or `.md`
- Captions → `paper/figures/captions.tex`
- Never write directly to a "final" manuscript file — always a draft version.
