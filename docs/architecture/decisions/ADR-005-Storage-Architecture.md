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

### Bulk index writes (v1.4.2)

`SqliteIndexStore` uses WAL mode, a reused prepared `INSERT`, and transaction batching (5,000 lines per commit). `HybridIndexWriter` logs progress every 10,000 persisted lines. `BM_IndexStoreAppend/100000` is gated in CI via `baseline.json`.

### Schema v2 and migration (v1.4.3 — M11.7 shipped)

Schema version bumps from **1** to **2**. New `meta` keys: `source_size`, `source_mtime`, `indexed_line_count`, `content_compression` (`none` | `zlib`).

**v1 → v2 policy:** opening a schema v1 index returns an error requiring rebuild; `SqliteIndexStore::create` removes any stale index file and builds schema v2 from the authoritative source log file. No in-place row migration.

**M11.7** creates stub v2 tables `line_json_fields` and `query_cache` (populated in M11.9 and M11.10). **`lines_fts`** is deferred to **M11.12** (requires `SQLITE_ENABLE_FTS5`).

```sql
-- lines table: content may be TEXT (plain) or BLOB (zlib) per meta.content_compression
-- optional column content_encoding TEXT DEFAULT 'plain' documents row-level override

CREATE TABLE line_json_fields (
  line_id INTEGER NOT NULL REFERENCES lines(id),
  field TEXT NOT NULL,
  value TEXT NOT NULL,
  PRIMARY KEY (line_id, field, value)
);
CREATE INDEX idx_ljf_field_value ON line_json_fields(field, value);

CREATE TABLE query_cache (
  cache_key TEXT PRIMARY KEY,
  line_ids_blob BLOB NOT NULL,
  created_at INTEGER NOT NULL,
  hit_count INTEGER NOT NULL DEFAULT 0
);

-- M11.12:
CREATE VIRTUAL TABLE lines_fts USING fts5(
  message, content, content='lines', content_rowid='id'
);
```

### Compression (v1.4.3 — design)

When `storage.compress_content=true`, lines at or above `storage.compress_threshold_bytes` store zlib-compressed `content` BLOBs. Reads decompress transparently. Toggling compression on an existing index requires rebuild.

### JSON field predicates (v1.4.3 — design)

During JSONL indexing, top-level key/value pairs populate `line_json_fields`. `QueryPlanner` maps `field == "value"` to `EXISTS (SELECT 1 FROM line_json_fields ...)`. Non-pushable predicates fall back to `QueryEvaluator`.

### Query cache (v1.4.3 — design)

`query_cache` stores serialized matching line IDs keyed by `SHA256(fingerprint + canonical_filter + schema_version)`. Invalidated on fingerprint change, source truncate, schema bump, or when disabled via `storage.query_cache.enabled=false`.

### Incremental append (v1.4.3 — design)

When `storage.incremental_append=true` (default) and source file **grows** (size increase, mtime newer) with matching path, reuse opens the store in append mode starting at `indexed_line_count + 1`. **Truncate** (size decrease) deletes the index and rebuilds.

### FTS5 (v1.4.3 — design)

SQLite built with `SQLITE_ENABLE_FTS5`. `lines_fts` maintained on insert. `contains(message|content, ...)` and M7 text search use FTS `MATCH` when a persisted store is attached; otherwise existing in-memory paths apply.

### Build

- SQLite amalgamation fetched at configure time; `LOGSCOPE_STORAGE=OFF` omits `scope_storage` for minimal builds (future)
- `scope_storage` links `scope_analysis`, `scope_foundation`, `scope_query`, `sqlite3`

---

## Non-goals (updated through v1.4.3 design)

Shipped in **`v1.4.2`:**

- Batched SQLite bulk writes and `BM_IndexStoreAppend/100000` benchmark SLA (see [M11-STORAGE-LAYER.md](../../planning/M11-STORAGE-LAYER.md))

**v1.4.3 (in progress — see [M11-V143-STORAGE-SCENARIOS.md](../../planning/M11-V143-STORAGE-SCENARIOS.md)):**

- Schema v2 migration, zlib compression, `line_json_fields`, `query_cache`, incremental append, FTS5

Deferred to **M12 (`v1.5.0`):**

- Storage provider plugins
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
