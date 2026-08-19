# Failure-Shape Coverage — Evidence Capability Charter

| Field | Value |
|-------|-------|
| Status | **Proposed** (G0 input) |
| Date | 19-08-2026 |
| Scope owner | Planning (G0) |
| Related | [ADR-012 Agent-Facing Analysis Contract](../architecture/decisions/ADR-012-Agent-Facing-Analysis-Contract.md), [ADR-013 Evidence Ingestion Integrity](../architecture/decisions/ADR-013-Evidence-Ingestion-Integrity.md) |

---

## Context

LogScope's investigation surfaces were built around one class of failure: something crashed or logged an error. Automated debugging workflows classify an incoming failure into one of six distinct shapes **before** choosing a heuristic, because the shape decides which evidence is even relevant. Measured against that taxonomy, LogScope covers one shape well.

This charter records the coverage gap, the dependency order between the missing pieces, and the slice proposed for the first wave. It exists so the two G1 ADRs have a stated problem to answer.

| Failure shape | Symptom pattern | Coverage today |
|---------------|-----------------|----------------|
| hard-failure | crash, assert, exception, timeout, explicit error | **Covered** |
| sporadic / N-of-M | k of n instances fail | Not covered |
| non-event | an expected signal never occurred | Not covered |
| config-only | fails on one profile, not another | Not covered |
| performance | correctness intact, latency or throughput regressed | Not covered |
| scale-only | reproduces only under load | Not covered |

---

## Per-shape analysis

### hard-failure — covered, one refinement

Present: crash analysis with fault-thread scoring, alternate stack-dump dialects, crash-summary timeline placement, error clusters, repeated errors, level breakdown, timeline buckets.

Gap: no trigger-versus-cascade ranking. Clusters are ordered by frequency, so a cascade of identical downstream errors outranks the single line that caused them.

Closes it: a causal-ordering heuristic over existing timeline and cluster data — first error in a burst, the error opening a new cluster pattern, the error preceding a level escalation — emitted as a ranked trigger-candidate list. **Size M**, no new dependencies.

### sporadic / N-of-M — not covered, highest value

Present: nothing usable. Directory ingestion concatenates files and renumbers lines; a match reports a merged line number with no file attribution.

Closes it: per-line file identity (ADR-013 §5), an instance grouping key, then per-group analytics with a divergence report naming how one group differs from the rest. **Size L**, prerequisite already specified.

### non-event — not covered

Present: a query DSL that asserts presence only. There is no absence primitive. A missing event yields an empty result, today indistinguishable from "could not look" (ADR-012 §4).

Closes it: run comparison, where signals present only in the healthy run surface the disappearance; then declared expected-sequence assertions. **Size M + M**, the second depending on the first.

### config-only — not covered, and outside current identity

Present: LogScope validates its own configuration; it does not diff two external ones.

Closes it: treat a configuration dump as an artifact the investigation holds and diffs, rather than adding a configuration analyser. **Size S** scoped to properties and environment dumps, **L** if extended to layered deployment templates. Recommend deferring or delegating to a plugin.

### performance — not covered, cheaper than it appears

Present: numeric filtering on structured fields works. The data path already exists.

Gap: no aggregation over field values — no percentile, histogram, median, or quantile — no duration extraction, and the trend verdict is computed from error rate over bucket volumes only.

Closes it: a numeric aggregation layer, duration extraction for common line patterns, and generalising the trend verdict to any numeric series. **Size M** — an aggregation layer over existing plumbing, not a new pipeline.

### scale-only — not covered, collapses into performance

Present: nothing. No resource or metric series ingestion.

Closes it: the aggregation layer above, plus ingesting a metric series as an artifact aligned to the log timeline. **Size M on top of performance.** Treating these as one investment rather than two is the central finding of this charter.

---

## Dependency edges

- **Ingestion integrity gates everything.** Every figure above is unreliable while discovery can silently omit most of the input (ADR-013).
- **Per-line file identity gates N-of-M grouping** and independently gates ADR-012 §7 evidence identity, so it pays for itself twice.
- **Run comparison gates non-event detection.**
- **One numeric aggregation layer closes two shapes.**

---

## Proposed sequence

| # | Work | Shape effect |
|---|------|--------------|
| 1 | Ingestion integrity (ADR-013) | makes all existing output trustworthy |
| 2 | Per-line file identity + instance grouping | closes N-of-M |
| 3 | Run comparison | closes baseline diff, half of non-event |
| 4 | Trigger-versus-cascade ranking | sharpens the covered shape |
| 5 | Numeric aggregation | closes performance and scale-only together |
| 6 | Expected-sequence assertions | completes non-event |
| 7 | Configuration diff | defer or delegate |

Coverage moves from one of six today to roughly three and a half of six after step 3.

---

## Relationship to the existing backlog

[`NEXT-VALUE-ADD.md`](NEXT-VALUE-ADD.md) already carries a **deferred hardening backlog** (remaining #185–#202) as a later H0 wave, and holds `#144-B`, that backlog, and P3 as three tracks that must not be combined. This charter does **not** propose reordering those tracks.

The gap it records is separate: the shapes below are missing capabilities rather than defects, and the ingestion defect that motivates ADR-013 is a new finding not present in #185–#202. If chartered, this would be a further H0 wave (Waves 1 and 2 shipped as `v2.13.0` and `v2.13.1`), sequenced against the existing tracks by G0 — not alongside them.

Per `NEXT-VALUE-ADD.md` §5, tactical planning docs are authored when an item is chartered for release, and P3 guidance is explicit that ADR work is not pre-chartered speculatively. The two linked ADRs are therefore **drafts offered as input to a G0 decision**, not designs requesting acceptance.

## Proposed scope gate

If chartered, the wave owns **steps 1 and 2 only**. Product UX, query DSL surface, and report presentation stay frozen. Steps 3 onward require their own G0.

## Out of scope

- New timestamp dialects (ADR-013 §6 reports the gap only)
- In-process archive extraction (ADR-013 §7)
- Sensitive-field redaction — orthogonal to coverage, gates adoption independently, needs its own charter
- Packet captures and other non-text evidence
