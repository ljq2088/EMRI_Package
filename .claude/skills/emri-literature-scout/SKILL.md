---
name: emri-literature-scout
description: Systematic EMRI literature scanner — searches, catalogues, and produces structured summaries
triggers:
  - "search literature for X"
  - "what's the state of X in EMRI"
  - "find papers about X"
  - "has anyone done X for Kerr/Teukolsky/waveform/surrogate"
---

# EMRI Literature Scout

## Purpose
Scan the literature for EMRI-related work and return structured, diffable summaries
suitable for `literature/` cataloguing and prior-art checks.

## When to Invoke
- Before starting research on any new EMRI topic (orbit model, waveform model, surrogate, PE method)
- When the `prior-art-check` skill requests literature coverage for a specific claim
- When refreshing the baseline literature coverage for a domain

## Input
- **Topic**: e.g. "EMRI surrogate models", "Teukolsky homogeneous solutions", "NK inspiral"
- **Scope**: "last 5 years" / "foundational only" / "comprehensive"
- **Sources**: default `inspirehep + arXiv`; optionally `ADS, Google Scholar`
- **Known anchors** (optional): papers to use as citation seeds

## Procedure
1. Query inspirehep for the topic; extract title, author, year, arXiv ID, citation count.
2. Query arXiv for recent preprints on the same topic.
3. For each paper found: classify relevance as `core` / `related` / `background`.
4. Cross-check against `literature/` — flag duplicates and gaps.

## Output Structure
For each paper, produce a structured entry:

```
### [FirstAuthor et al. (Year)](arXiv link)
**Relevance:** core / related / background
**What:** One-sentence summary of the main result.
**Method:** Orbit model / PN / GSF / Teukolsky / surrogate / etc.
**Regime:** Kerr/Schwarzschild, circular/eccentric/inclined, adiabatic/GSF
**Benchmark potential:** Yes/No — if Yes, specify the check (e.g. "circular equatorial Omega_phi Table I")
**File:** literature/<arxiv_id>.pdf (if downloaded)
```

## Output Destination
- New papers → `literature/README.md` catalogue (append)
- Detailed notes → `notes/literature-notes/<topic>-<date>.md`
- Flag truly novel gaps for `prior-art-check`

## Domain Anchors (always check against these first)
1. Fujita & Hikida (2009) — analytic geodesic orbits in Kerr
2. Schmidt (2002) — circular equatorial Teukolsky modes
3. Detweiler (1978), Sasaki & Nakamura (1982) — Teukolsky homogeneous solutions
4. Drasco & Hughes (2006) — adiabatic inspiral, kludge waveforms
5. Flanagan & Hughes (1998) — mismatch/detectability thresholds
