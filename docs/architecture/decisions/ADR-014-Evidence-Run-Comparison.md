# ADR-014 — Evidence Run Comparison

| Field | Value |
|-------|-------|
| Status | **Accepted** (G1 — 20-08-2026) |
| Date | 20-08-2026 |
| Deciders | Architecture (G1) — approved Wave 4 step 3 |
| Related | [ADR-013 Evidence Ingestion Integrity](ADR-013-Evidence-Ingestion-Integrity.md), [ADR-012 Agent-Facing Analysis Contract](ADR-012-Agent-Facing-Analysis-Contract.md), [FAILURE-SHAPE-COVERAGE](../../planning/FAILURE-SHAPE-COVERAGE.md) |

---

## Context

[ADR-013](ADR-013-Evidence-Ingestion-Integrity.md) (shipped `v2.13.2`) made **discovered input** and **per-line file identity** trustworthy. Investigators and pipelines can now attribute evidence to files and instances.

The next failure shape gap is **contrast between runs**. Today an investigator must run `analyze` twice and diff mentally (or with external tools). That fails because:

- Stream `lineNumber` differs when file layout differs — not a stable join key.
- Query filters assert **presence** only; an expected signal that never fired is indistinguishable from “could not look” ([ADR-012](ADR-012-Agent-Facing-Analysis-Contract.md) §4).
- Multi-instance bundles have no per-instance contrast primitive.

[FAILURE-SHAPE-COVERAGE](../../planning/FAILURE-SHAPE-COVERAGE.md) step **3 — run comparison** closes baseline diff and the **first half** of non-event detection. Steps 4–7 remain separate G0 decisions.

**G0 question:**

> When two runs are expected to represent the same incident scenario, can an investigator reliably identify what evidence changed, appeared, disappeared, or failed to occur?

---

## Decision

### 1. Comparison is a first-class analysis operation

A new CLI command **`logscope compare`** compares two ingested sources with explicit roles:

| Role | Flag | Meaning |
|------|------|---------|
| Baseline | positional `baseline` | Known-good, before, or reference run |
| Candidate | positional `candidate` | Failing, after, or suspect run |

Both paths use the same discovery + analysis pipeline as `analyze` (ADR-013). Comparison operates on **analysis results**, not raw bytes.

```text
DISCOVER+ANALYZE(baseline)  →  RunSnapshot A
DISCOVER+ANALYZE(candidate) →  RunSnapshot B
ALIGN(A, B)                 →  AlignmentPlan (or incomparable)
DIFF(AlignmentPlan)         →  ComparisonResult
```

### 2. Alignment uses Wave 3 identity — not stream line numbers

**Instance alignment:** match on `instanceKey` from discovery census. Instances present on only one side are reported; they do not silently merge into `"default"`.

**File alignment:** within a matched instance, match files on **normalized relative path** (`sourceFileRelative`). Rotation groups are compared as a single logical stream keyed by `rotationGroupId` when present; otherwise per physical file.

**Binding rule:**

```text
Comparison keys MUST NOT use streamLineNumber across runs.
Join keys: instanceKey + relativePath (+ rotationGroupId when set).
```

### 3. Evidence unit — normalized error signature

Wave 4 compares **error-line signatures** using the existing `normalizeClusterSignature()` heuristic (`core/analytics/message_signature.hpp`). This reuses the same normalization as `ErrorClusterer` without chartering standalone “signature extraction” as a product surface.

Each signature entry carries:

| Field | Meaning |
|-------|---------|
| `signature` | Normalized cluster key |
| `count` | Occurrences in that run |
| `firstSeen` | Earliest timestamp (if parsed) |
| `sampleMessage` | Representative excerpt |
| `sampleLocation` | `{ sourceFile, fileLineNumber, instanceKey }` |

**In scope:** error-level lines (same level filter as clustering). **Out of scope:** full-text line diff of every line; trigger-vs-cascade ranking; numeric aggregation.

### 4. Comparison result — absence is first-class

JSON payload under `data.comparison` (minimal envelope for Wave 4 — full ADR-012 envelope deferred):

```json
{
  "comparable": true,
  "complete": true,
  "alignment": {
    "matchedInstances": ["pod-a", "pod-b"],
    "onlyInBaseline": { "instances": [], "files": [] },
    "onlyInCandidate": { "instances": [], "files": [] }
  },
  "onlyInBaseline": [
    { "signature": "connection reset", "count": 3, "firstSeen": "...", "sampleLocation": { } }
  ],
  "onlyInCandidate": [
    { "signature": "timeout waiting for ack", "count": 12, "firstSeen": "..." }
  ],
  "countDeltas": [
    { "signature": "handler error", "baseline": 4, "candidate": 91 }
  ],
  "warnings": []
}
```

**Non-event (first half):** signatures in `onlyInBaseline` are evidence that **vanished** in the candidate — as diagnostic as new errors in `onlyInCandidate`.

### 5. Incomparable runs — explicit, fail-closed

When alignment cannot produce a meaningful contrast, `comparable: false` and **no diff lists presented as empty equality**.

| Code | When |
|------|------|
| `BASELINE_INDETERMINATE` | Baseline discovery/analysis indeterminate (e.g. all skipped) |
| `CANDIDATE_INDETERMINATE` | Candidate indeterminate |
| `NO_INSTANCE_OVERLAP` | Zero shared `instanceKey` values |
| `NO_FILE_OVERLAP` | Shared instances but zero matched file paths |
| `BASELINE_UNSUPPORTED` / `CANDIDATE_UNSUPPORTED` | Archive or unsupported root |

Exit codes:

| Code | Meaning |
|------|---------|
| `0` | Comparable comparison produced |
| `1` | Invalid arguments / unsupported path |
| `2` | Incomparable or indeterminate (same class as `analyze` indeterminate) |

### 6. Partial ingest on either side

If either run has `analysis.complete: false` (skips, unknown dialect, limits):

- Comparison **may proceed** when `comparable: true`.
- Result sets `complete: false` and adds warning `COMPARISON_BOUNDED` with per-side discovery/analysis summary.
- Comparison **must not** claim full coverage.

If a side is indeterminate (zero analyzed lines), comparison is **refused** (`comparable: false`).

### 7. Per-instance mode (bundles)

When both runs expose multiple instances, default output includes:

- **Aggregate** diff across all matched instances (union of signatures).
- **`perInstance[]`** entries with the same shape for each matched `instanceKey`.

Unmatched instances appear under `alignment.onlyInBaseline.instances` / `onlyInCandidate.instances` and do not produce signature diffs for missing peers.

---

## Consequences

### Positive

- Pipelines can branch on `onlyInBaseline` / `onlyInCandidate` without manual diff scripts.
- Non-event failures become visible when a baseline signature disappears.
- Builds on ADR-013 identity without new investigation UX.

### Negative

- Signature normalization can collapse distinct errors — documented limitation; line-level divergence deferred.
- New command and test matrix; comparison cost ≈ 2× analyze for the same inputs.

### Out of scope

- Trigger vs cascade ranking (step 4)
- Numeric aggregation / percentiles (step 5)
- Expected-sequence assertions (step 6)
- Configuration diff (step 7)
- Full ADR-012 envelope on all commands
- Web/desktop UI, AI narrative, #144-B, #185–#201 cluster

---

## Compliance

G3 scenario matrix: [`V213Z-RUN-COMPARISON-SCENARIOS.md`](../../planning/V213Z-RUN-COMPARISON-SCENARIOS.md).

Contract tests MUST include: comparable pair produces parseable JSON; vanished baseline signature appears in `onlyInBaseline`; instance mismatch yields `comparable: false` + exit `2`; indeterminate baseline refuses comparison.
