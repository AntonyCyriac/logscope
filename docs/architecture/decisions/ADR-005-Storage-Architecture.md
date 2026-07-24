# ADR-005: Storage Architecture

- **Status:** Accepted
- **Date:** 24-07-2026

---

## Context

M6–M10 build a **bounded in-memory** `LineIndex` (default 10,000 lines, excerpt-only content). Investigation, analytics, and sessions re-scan or re-analyze sources when the index is missing or truncated. M11 must scale investigations **without** becoming a log storage platform—source log files remain authoritative.

Architectural guardrails require an ADR before SQLite implementation. M12 may add alternate storage provider plugins later.

---

## Decision

LogScope introduces **`scope_storage`** with:

1. **`IndexStore`** abstraction and **`SqliteIndexStore`** as the built-in backend (SQLite amalgamation via CMake `FetchContent`)
2. **Hybrid indexing**: in-memory `LineIndex` by default; spill or persist to SQLite when `--persist-index`, capacity exceeded, or session reuse requires it
3. **`IndexFingerprint`** (source path + file size + last-write time) for cache invalidation
4. **`IndexReader`** unified read path over memory index and/or SQLite store
5. **`QueryPlanner`** translating pushable M10 DSL predicates to SQL `WHERE` clauses
6. Session format **1.3** fields: `index.fingerprint`, `index.path`, `index.line_count`

### Storage modes

| Mode | Trigger | Behaviour |
|------|---------|-----------|
| `memory` | Default | Existing `LineIndex` only |
| `hybrid` | Lines exceed `maxIndexedLines` or `--persist-index` | First N lines in memory; all lines in SQLite |
| `persistent` | `--index-path` or `storage.index.path` | All lines written to SQLite |

### SQLite schema (v1)

```sql
CREATE TABLE meta (
  key TEXT PRIMARY KEY,
  value TEXT NOT NULL
);
CREATE TABLE lines (
  id INTEGER PRIMARY KEY,
  line_number INTEGER NOT NULL,
  level INTEGER NOT NULL,
  timestamp_unix INTEGER,
  message TEXT NOT NULL,
  content TEXT NOT NULL,
  correlation_id TEXT NOT NULL,
  top_level_keys_json TEXT NOT NULL
);
CREATE INDEX idx_lines_level ON lines(level);
CREATE INDEX idx_lines_timestamp ON lines(timestamp_unix);
CREATE INDEX idx_lines_correlation ON lines(correlation_id);
```

Persisted indexes store **full line content** (not excerpts). Default index directory: `~/.logscope/indexes/` (or `%LOCALAPPDATA%/logscope/indexes` on Windows).

### Query pushdown (v1.4.1)

`QueryPlanner` maps comparison predicates and `contains(field, "text")` to SQL. Complex boolean trees push down when all leaves are pushable; otherwise fetch from store and filter in memory with existing `QueryEvaluator`.

M7 text search remains in-memory over fetched rows until a future FTS milestone.

### Build

- SQLite amalgamation fetched at configure time; `LOGSCOPE_STORAGE=OFF` omits `scope_storage` for minimal builds (future)
- `scope_storage` links `scope_analysis`, `scope_foundation`, `scope_query`, `sqlite3`

---

## Non-goals (v1.4.1)

- Compression (v1.4.2)
- Cached query results table (v1.4.2)
- Incremental append indexing (v1.4.2)
- `line_json_fields` for arbitrary JSON predicates (v1.4.2)
- FTS5 full-text index
- M12 storage provider plugins
- Replacing source log files

---

## Consequences

### Positive

- Investigations scale beyond in-memory cap without re-reading source on session load when fingerprint matches
- Full-line content in persisted indexes improves search fidelity
- `IndexStore` interface prepares M12 plugin backends
- M10 `QueryEvaluator` unchanged; pushdown is additive

### Negative

- SQLite dependency increases binary size and build time
- Dual read paths (memory vs SQLite) require `IndexReader` discipline
- Fingerprint invalidation on partial file edits requires full re-index (documented)

---

## Compliance

- CLI-first: storage logic in `core/storage`, wired through analysis and investigation
- MIT license (project code; SQLite is public domain)
- CI: `scope_storage_tests`, e2e persist-index scenarios, release matrix smoke
