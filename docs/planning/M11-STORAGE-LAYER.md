# M11 – Storage Layer

| Field | Value |
|-------|-------|
| Document | M11 – Storage Layer |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | Approved |
| Created | 24-07-2026 |
| Last Updated | 24-07-2026 |

---

# 1. Purpose

This document defines **M11 – Storage Layer**, introducing SQLite-backed persistent log line indexes with hybrid memory spill, session index reuse, and basic M10 query pushdown.

Target release: **`v1.4.1`** (core scope). Compression and query cache follow in **`v1.4.2`**.

See [ADR-005](../architecture/decisions/ADR-005-Storage-Architecture.md) for architecture and compatibility rules.

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

# 6. Deferred (v1.4.2+)

- zlib compression on `content` column
- `query_cache` materialized results
- `line_json_fields` for `service == "PCF"` predicates
- Incremental append indexing
- FTS5 full-text search
- M12 storage provider plugins
