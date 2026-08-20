# v2.13.x Run Comparison — Scenario Matrix

| Field | Value |
|-------|-------|
| Document | Run Comparison Scenario Matrix |
| Category | Project Planning |
| Version | 1.2.0 |
| Status | **G4 complete** — Wave 4 step 3 (`b177073` / `v2.13.3`, PR [#216](https://github.com/AntonyCyriac/logscope/pull/216)) |
| ADR | [ADR-014](../architecture/decisions/ADR-014-Evidence-Run-Comparison.md) (**Accepted**) |

---

## Purpose

Acceptance matrix for **Run Comparison (H0 Wave 4, failure-shape step 3)** — baseline/candidate selection, instance/file alignment, signature diff, non-event absence, incomparable handling.

**Binding invariant:**

```text
Comparison MUST use instanceKey + relativePath identity (ADR-013).
Absence in candidate MUST appear in onlyInBaseline — not as silent empty query.
Incomparable runs MUST set comparable:false — never exit 0 with empty diff implying equality.
```

**G0 question:**

> When two runs are expected to represent the same incident scenario, can an investigator reliably identify what evidence changed, appeared, disappeared, or failed to occur?

---

## G0 Wave 4 — in scope

| Step | Theme | Gate |
|------|-------|------|
| 3 | Run comparison — align, diff, absence, incomparable | G3 RC.1–RC.8 |

```text
G0 ✅ → G1 ✅ → G2 ✅ → G3 ✅ → G4 ✅ → G5 v2.13.3
```

**Out of scope:** failure-shape steps 4–7; full ADR-012 envelope; #144-B; #185–#201; investigation UX.

---

## Scenario matrix

| ID | Theme | Path / fixture | Expected | Test hook |
|----|-------|----------------|----------|-----------|
| RC.1 | Single-file diff | `good.log` vs `bad.log` — disjoint error signatures | `onlyInCandidate` and `onlyInBaseline` populated; `comparable: true` | `RunComparisonTest.SingleFileSignatureDiff` |
| RC.2 | Count delta | Same signature, different counts (4 vs 91) | `countDeltas[]` entry with baseline/candidate counts | `RunComparisonTest.CountDeltaReported` |
| RC.3 | Non-event absence | Error only in baseline | Signature in `onlyInBaseline`; not empty success | `RunComparisonTest.VanishedSignatureInOnlyInBaseline` |
| RC.4 | Instance bundle | `inst-a/app.log` + `inst-b/app.log` on both sides | `perInstance[]` for matched keys; aggregate diff | `RunComparisonTest.TwoInstanceBundleCompare` |
| RC.5 | Instance mismatch | Baseline `inst-a` only; candidate `inst-c` only | `comparable: false`; `NO_INSTANCE_OVERLAP`; exit **2** | `RunComparisonTest.InstanceMismatchIncomparable` |
| RC.6 | Unrelated trees | No shared relative paths under matched default instance | `comparable: false`; `NO_FILE_OVERLAP` | `RunComparisonTest.NoFileOverlapIncomparable` |
| RC.7 | Partial skip | Candidate has binary skip; some files analyzed | `comparable: true`; `complete: false`; `COMPARISON_BOUNDED` warning | `RunComparisonTest.PartialSkipBoundedCompare` |
| RC.8 | Indeterminate side | Baseline all-binary directory | `comparable: false`; `BASELINE_INDETERMINATE`; exit **2** | `RunComparisonTest.IndeterminateBaselineRefused` |

---

## Fixtures

### RC.1 — single-file good/bad

```text
fixtures/rc1/good.log   # ERROR: healthy heartbeat ok
fixtures/rc1/bad.log    # ERROR: timeout waiting for ack
```

```bash
logscope compare fixtures/rc1/good.log fixtures/rc1/bad.log --format json
```

**Expected:** `onlyInCandidate` contains `timeout`; `onlyInBaseline` contains `heartbeat` (normalized signatures).

### RC.3 — vanished signature (non-event)

```text
fixtures/rc3/good.log   # ERROR: expected scheduler tick
fixtures/rc3/bad.log    # (no error lines)
```

**Expected:** `onlyInBaseline` non-empty; **not** `countDeltas` only with zero candidate.

### RC.5 — instance mismatch

```text
baseline/inst-a/app.log
candidate/inst-z/app.log
```

**Expected:** `comparable: false`; reason `NO_INSTANCE_OVERLAP` (or unmatched instance policy per ADR-014 §5).

---

## Integrity gates (G3 blocking)

- [x] `logscope compare` exists; roles baseline/candidate explicit
- [x] JSON `data.comparison.comparable` present on `--format json`
- [x] `onlyInBaseline` / `onlyInCandidate` / `countDeltas` on comparable runs
- [x] Incomparable → exit **2**, not exit **0** with empty arrays implying match
- [x] Alignment uses `instanceKey` + `sourceFileRelative` (no stream-line join)

---

## Non-goals (NG)

| ID | Scenario |
|----|----------|
| NG.1 | Trigger vs cascade ranking |
| NG.2 | Numeric aggregation / percentiles |
| NG.3 | Expected-sequence DSL |
| NG.4 | Configuration diff |
| NG.5 | Full ADR-012 envelope on all commands |
| NG.6 | Line-by-line text diff of entire files |
| NG.7 | Web/desktop investigation UI |
| NG.8 | N-of-M divergence narrative report |

---

## Revision history

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 20-08-2026 | G1 scenario matrix — Wave 4 step 3 |
| 1.1.0 | 20-08-2026 | G3 integrity gates checked — RC.1–RC.8 pass |
| 1.2.0 | 20-08-2026 | G4 — release notes + docs delta for `v2.13.3` |
