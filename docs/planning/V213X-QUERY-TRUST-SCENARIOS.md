# v2.13.x Query Trust — Scenario Matrix

| Field | Value |
|-------|-------|
| Document | Query Trust Scenario Matrix |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | **G1 approved** — Wave 2 chartered |
| Created | 19-08-2026 |
| Milestone | [v2.13.x — Quality & Integrity (deferred)](https://github.com/AntonyCyriac/logscope/milestone/1) |
| ADR | [ADR-005-M10.1](../architecture/decisions/ADR-005-M10.1-Query-Trust.md) (Proposed) |

---

## Purpose

Acceptance matrix for **Query Trust Wave 2** ([#198](https://github.com/AntonyCyriac/logscope/issues/198), [#197](https://github.com/AntonyCyriac/logscope/issues/197)).

**Binding principle:**

> **The persisted index is an optimization, not a second query language.**

**G0 question:**

> Can an investigator trust that a query returns the same correct evidence regardless of whether it is evaluated in-memory or through the persisted index?

---

## G0 Wave 2 — in scope

| Issue | Subsystem | Summary | Gate |
|-------|-----------|---------|------|
| [#198](https://github.com/AntonyCyriac/logscope/issues/198) | query | JSON field `>` / `<` always match nothing | G3 QT.1–QT.3 |
| [#197](https://github.com/AntonyCyriac/logscope/issues/197) | query | FTS5 `contains()` misses CJK | G3 QT.4–QT.6 |

```text
G0 ✅ → G1 ✅ → G2 → G3 → G4 → G5 v2.13.x
```

---

## Scenario matrix

| ID | Issue | Path | Memory | Persist-index | Test hook |
|----|-------|------|--------|---------------|-----------|
| QT.1 | #198 | `code > 10` on JSONL | Must match `code:20` line | **Same line numbers as memory** | `QueryEvaluatorTest` + `sqlite_index_store_json_fields_test` |
| QT.2 | #198 | `code == 10`, `code != 10` | Unchanged | Same as memory | Regression |
| QT.3 | #198 | `service > "PCF"` | **Error** (explicit reject) | **Error** (explicit reject) | `QueryEvaluatorTest` |
| QT.4 | #197 | `contains(message, "日本語")` | 1 match | **Same count as memory** | `sqlite_index_store_fts_test` |
| QT.5 | #197 | `contains(message, "timeout")` | 1 match | 1 match (FTS pushdown OK) | Regression |
| QT.6 | — | `level == ERROR AND contains(message, "日本語")` | Composite match | **Same as memory** | Integration |

### QT.1 fixture (#198)

```jsonl
{"level":"ERROR","message":"a","code":10}
{"level":"INFO","message":"b","code":20}
{"level":"ERROR","message":"c","code":10}
```

```bash
logscope query --log-format jsonl --filter 'code > 10' eav.jsonl
logscope analyze --persist-index --index-path eav.db eav.jsonl
logscope query --persist-index --reuse-index --index-path eav.db \
  --log-format jsonl --filter 'code > 10' eav.jsonl
```

**Expected:** 1 match (line with `code:20`) on both paths.

### QT.4 fixture (#197)

```text
2026-01-01 00:00:00 INFO hello-world
2026-01-01 00:00:02 INFO 日本語トークン
2026-01-01 00:00:03 INFO punctuation!!!timeout???
```

```bash
logscope query --filter 'contains(message, "日本語")' fts.log
logscope analyze --persist-index --index-path fts.db fts.log
logscope query --persist-index --reuse-index --index-path fts.db \
  --filter 'contains(message, "日本語")' fts.log
```

**Expected:** 1 match on both paths.

---

## Integrity gates (blocking G3)

**#198 gate:**

```text
JSON numeric ordered comparison
  → correct matches OR explicit reject
  → never exit 0 with silent zero when matches exist
```

**#197 gate:**

```text
contains() with CJK needle
  → in-memory count == persist-index count
```

---

## Non-goals (NG)

| ID | Item |
|----|------|
| NG.1 | #196 query cache |
| NG.2 | P4 IQL / new query syntax |
| NG.3 | `contains()` on fields other than message/content |
| NG.4 | Investigation UX / domain model |

## Deferred (same milestone)

All other #185–#201, #199 — see [V213-QUALITY-INTEGRITY-SCENARIOS.md](V213-QUALITY-INTEGRITY-SCENARIOS.md).

---

## Revision history

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 19-08-2026 | G1 approved — Query Trust matrix for Wave 2 |
