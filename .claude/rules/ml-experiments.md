---
name: ml-experiments
description: ML experiment discipline — baselines, ablations, metrics, reproducibility, and what "better" actually means
scope:
  src: ["*.py"]
  tests: ["test_*.py"]
  benchmarks: ["*.py"]
  notes: ["*.md"]
---

# ML Experiment Rules

## Before Training
- **Baseline first.** A non-ML baseline (analytic fit, interpolated lookup, zero model) must exist and produce a number before any neural network is trained.
- **Ablation design.** Every architectural choice (depth, activation, input features, loss weight) must have a planned removal: "what number do we get if we take this out?"
- **Seed discipline.** Fix `np.random.seed(0)` and `torch.manual_seed(0)` (or framework equivalent). Report the seed. Run top model on 3 seeds and report spread.

## Metrics — Not Just Loss
- **Primary EMRI metrics (use at least one):**
  - **Overlap** M(h_pred, h_true) — normalized inner product, maximized
  - **Mismatch** 1 - max{M} over time/phase shifts
  - **Phase error** δφ(t) modulo 2π
  - **Amplitude error** δA(t)/A(t)
- **Loss is not a metric.** Training loss measures optimization progress, not physical fidelity. Never report loss as the sole evidence of quality.

## Reporting
- Every ML result must include:
  1. Model size (parameters, disk footprint)
  2. Inference wall time (ms per evaluation)
  3. Training wall time (GPU-hours)
  4. Dataset size and generation cost
  5. Metric on train / validation / test splits
- "The surrogate is 1000× faster than the numerical solver" is a claim — back it with a timed comparison, not an estimate.

## Reproducibility
- All training scripts must be checkpointed to `notes/` with date and git hash.
- Model weights are not tracked in git (use `.gitignore`). Link them in a `notes/model_registry.md` with hash and training parameters.
- Environment: record `pip freeze` or conda environment in the experiment note.

## Red Flags
- "Loss went down by 10% so the model is better" — insufficient. Show physical metric improvement.
- "We used a bigger network and it overfits" — expected, not a finding. Quantify the overfitting gap.
- Reporting only the best seed — cherry-picking. Report mean ± std over seeds.
