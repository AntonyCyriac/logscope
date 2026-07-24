# M11 – Storage Layer

| Field | Value |
|-------|-------|
| Document | M11 – Storage Layer |
| Category | Project Planning |
| Version | 1.2.0 |
| Status | Approved |
| Created | 24-07-2026 |
| Last Updated | 24-07-2026 |

---

# 1. Purpose

This document defines **M11 – Storage Layer**, introducing SQLite-backed persistent log line indexes with hybrid memory spill, session index reuse, and basic M10 query pushdown.

Target releases: **`v1.4.1`** (core scope), **`v1.4.2`** (bulk index write performance), **`v1.4.3`** (compression, cache, JSON fields, incremental append, FTS5).

See [ADR-005](../architecture/decisions/ADR-005-Storage-Architecture.md) and [M11 v1.4.3 Scenarios](M11-V143-STORAGE-SCENARIOS.md).

---

# 2. Dependency on M6–M10

| Prior output | M11 dependency |
|--------------|----------------|
| M6.4 — Line index | `IndexedLine` schema and bounded memory index |
| M7 — Search engine | Text search over fetched lines (unchanged) |
| M9 — Analytics | `IndexReader` for analytics beyond memory cap |
| M10 — Query language | `QueryPlanner` pushdown; `QueryEvaluator` fallback |

---

# 3. Phased Delivery

| Phase | Focus | Status |
|-------|-------|--------|
| M11.0 | ADR-005, planning doc, document map | ✅ Complete |
| M11.1 | `scope_storage` module, SQLite store, fingerprint, unit tests | ✅ Complete |
| M11.2 | `HybridIndexWriter`, analysis integration, storage config | ✅ Complete |
| M11.3 | `IndexReader`, investigation pushdown, analytics adapter | ✅ Complete |
| M11.4 | Session 1.3 index metadata, fingerprint reuse on load | ✅ Complete |
| M11.5 | CLI flags, e2e/matrix/benchmarks, `v1.4.1` release | ✅ Complete |
| M11.6 | Batched SQLite writes, indexing progress, `v1.4.2` release | ✅ Complete |
| M11.7 | Schema v2 migration framework | 🟡 Design (`v1.4.3`) |
| M11.8 | zlib `content` compression | 🟡 Design (`v1.4.3`) |
| M11.9 | `line_json_fields` + DSL pushdown | 🟡 Design (`v1.4.3`) |
| M11.10 | `query_cache` materialized results | 🟡 Design (`v1.4.3`) |
| M11.11 | Incremental append indexing | 🟡 Design (`v1.4.3`) |
| M11.12 | FTS5 full-text search pushdown | 🟡 Design (`v1.4.3`) |

---

# 4. Delivered Capabilities (v1.4.1)

## Core storage

- `IndexStore` / `SqliteIndexStore` with schema migration
- `IndexFingerprint` from source path, size, mtime
- `HybridIndexWriter` in analysis pipeline
- `IndexReader` unified iteration for investigation and analytics
- `QueryPlanner` SQL pushdown for M10 comparison and `contains()` predicates

## CLI

- `--persist-index` on `investigate`, `analyze`, `session save`
- `--index-path <file>` explicit SQLite index file
- `--reuse-index` skip rebuild when fingerprint matches (default on session load)
- Config: `storage.mode`, `storage.index.directory`, `storage.spill_threshold`

## Session

- Session version `1.3`: `index.fingerprint`, `index.path`, `index.line_count`
- Load reuses persisted index when source fingerprint matches

---

# 5. Engineering Conventions

| Convention | Value |
|------------|-------|
| Module | `core/storage` (`scope_storage`) |
| Tests | `scope_storage_tests` + CLI/e2e coverage |
| Benchmark | `BM_IndexStoreAppend`, `BM_QueryPushdown` |

---

# 6. Follow-on delivery

## v1.4.2 — Storage performance (shipped)

**Bulk index build performance** — product-facing work for `--persist-index` and hybrid spill on large logs:

| Item | Status |
|------|--------|
| Batched SQLite writes | ✅ WAL, prepared `INSERT`, 5k-line commits |
| Indexing feedback | ✅ Progress log every 10k lines |
| Benchmark SLA | ✅ `BM_IndexStoreAppend/100000` + `baseline.json` |

## v1.4.3 — Storage remainder (in progress)

**M11 completion** — compression, query cache, JSON field predicates, incremental append, FTS5. See [M11-V143-STORAGE-SCENARIOS.md](M11-V143-STORAGE-SCENARIOS.md).

| Item | Status |
|------|--------|
| Schema v2 migration | 🟡 Design |
| zlib compression on `content` | 🟡 Design |
| `line_json_fields` EAV + pushdown | 🟡 Design |
| `query_cache` | 🟡 Design |
| Incremental append | 🟡 Design |
| FTS5 | 🟡 Design |

## Deferred to M12 (`v1.5.0`)

- Storage provider plugins (`.so`/`.dll` backends)

---

# 7. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 24-07-2026 | Initial M11 planning document. |
| 1.1.0 | 24-07-2026 | v1.4.1 shipped; v1.4.2 bulk write perf. |
| 1.2.0 | 24-07-2026 | v1.4.3 phases M11.7–M11.12; scenario matrix link. |
