# M11 v1.4.3 — Storage Scenarios

| Field | Value |
|-------|-------|
| Document | M11 v1.4.3 Storage Scenarios |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | Draft (design) |
| Created | 24-07-2026 |
| Last Updated | 24-07-2026 |

---

# 1. Purpose

This document defines **acceptance scenarios** for **v1.4.3** — the M11 storage remainder. It is the implementation gate: every row must be implemented, tested, and checked off before release.

**Target release:** `v1.4.3`

**Features in scope:**

| ID | Feature |
|----|---------|
| S1 | Schema v2 migration framework |
| S2 | zlib compression on `content` column |
| S3 | `line_json_fields` EAV table + DSL pushdown |
| S4 | `query_cache` materialized filter results |
| S5 | Incremental append indexing |
| S6 | FTS5 full-text search pushdown |

**Out of scope (M12 / `v1.5.0`):** storage provider plugins, `.so`/`.dll` backends.

See [M11-STORAGE-LAYER.md](M11-STORAGE-LAYER.md), [ADR-005](../architecture/decisions/ADR-005-Storage-Architecture.md).

---

# 2. Scenario matrix legend

| Column | Meaning |
|--------|---------|
| **Regression** | Must have a dedicated regression or e2e guard |
| **Test layer** | U = unit, I = integration, E = e2e, B = benchmark |

Status: ⬜ planned · 🟡 in progress · ✅ complete

---

# 3. Schema v2 and migration (S1)

| ID | Scenario | Trigger | Expected behavior | Test | Reg | Status |
|----|----------|---------|-------------------|------|-----|--------|
| S1.1 | Fresh index | New `--persist-index` on empty path | `meta.schema_version=2`; all v2 tables created | U | | ✅ |
| S1.2 | Open v1 index | Open existing schema v1 `.db` | Rebuild index from source log (documented policy); no partial reads | U+I | Yes | ✅ |
| S1.3 | Corrupt database | Truncated or invalid SQLite file | Clear error; no crash; index file may be removed | U | | ✅ |
| S1.4 | Unsupported future schema | `schema_version` > app maximum | Fail closed with actionable error | U | | ✅ |
| S1.5 | Meta source snapshot | After finalize | `meta` stores `source_size`, `source_mtime`, `indexed_line_count` | U | | ✅ |

**Migration policy:** v1 → v2 always rebuilds from the authoritative source log file. In-place row migration is not attempted.

---

# 4. zlib compression (S2)

| ID | Scenario | Trigger | Expected behavior | Test | Reg | Status |
|----|----------|---------|-------------------|------|-----|--------|
| S2.1 | Compression enabled | `storage.compress_content=true` | `content` stored as zlib blob; `meta.content_compression=zlib` | U | | ⬜ |
| S2.2 | Compression disabled | `storage.compress_content=false` (default) | Plain TEXT `content` as today | U | | ⬜ |
| S2.3 | Small line skip | Line length < `storage.compress_threshold_bytes` | Store plain TEXT (avoid overhead) | U | | ⬜ |
| S2.4 | Read round-trip | Fetch compressed row | Decompressed content matches source line | U+I | Yes | ⬜ |
| S2.5 | Toggle on existing index | Enable compression on v2 plain index | Rebuild required; documented in USER_MANUAL | E | | ⬜ |
| S2.6 | Corrupt blob | Invalid zlib payload | Error on read; index marked unusable | U | | ⬜ |
| S2.7 | Disk size | 100k-line index | Measurable size reduction vs uncompressed v2 (benchmark) | B | | ⬜ |

---

# 5. line_json_fields (S3)

| ID | Scenario | Trigger | Expected behavior | Test | Reg | Status |
|----|----------|---------|-------------------|------|-----|--------|
| S3.1 | JSONL indexing | `--profile generic-json` + persist | EAV rows for each top-level string/number field | U | | ⬜ |
| S3.2 | Plain log | `generic-plain` source | No EAV rows; queries unaffected | U | | ⬜ |
| S3.3 | DSL pushdown | `filter 'service == "PCF"'` | SQL `EXISTS` on `line_json_fields`; no full scan | U+I | Yes | ⬜ |
| S3.4 | Unknown JSON field | Field not in EAV | `QueryEvaluator` fallback after fetch | U | | ⬜ |
| S3.5 | Combined filter | `service == "PCF" AND level == ERROR` | Pushdown when all leaves pushable | U | | ⬜ |
| S3.6 | CLI e2e | `logscope query --filter 'service == "PCF"'` | Matching lines on sample JSONL | E | Yes | ⬜ |

**Scope:** top-level JSON keys only in v1.4.3. Nested paths deferred.

---

# 6. query_cache (S4)

| ID | Scenario | Trigger | Expected behavior | Test | Reg | Status |
|----|----------|---------|-------------------|------|-----|--------|
| S4.1 | Cache miss | First filter on persisted index | Results stored in `query_cache`; normal SQL path | U+I | | ⬜ |
| S4.2 | Cache hit | Same fingerprint + normalized filter + schema version | Skip SQL line fetch; return cached line IDs | U+I | Yes | ⬜ |
| S4.3 | Fingerprint change | Source file replaced | Cache entries invalidated | U | Yes | ⬜ |
| S4.4 | Source truncate | File size < indexed `source_size` | Cache cleared; full rebuild | U+I | Yes | ⬜ |
| S4.5 | Schema bump | v1 rebuild → v2 | Cache cleared | U | | ⬜ |
| S4.6 | Cache disabled | `storage.query_cache.enabled=false` | No cache reads or writes | U | | ⬜ |
| S4.7 | Eviction | Entries > `storage.query_cache.max_entries` | LRU eviction of oldest entries | U | | ⬜ |
| S4.8 | Session reuse | `session load` + same filter | Cache hit when index unchanged | E | | ⬜ |

**Cache key:** `SHA256(fingerprint + canonical_filter_ast + schema_version)`.

---

# 7. Incremental append (S5)

| ID | Scenario | Trigger | Expected behavior | Test | Reg | Status |
|----|----------|---------|-------------------|------|-----|--------|
| S5.1 | Source growth | File size increases; mtime newer; `--reuse-index` | Append lines after `indexed_line_count` only | U+I | Yes | ⬜ |
| S5.2 | Source truncate | File size decreases | Delete index; full rebuild from line 1 | U+I | Yes | ⬜ |
| S5.3 | Unchanged file | Same size and mtime | Reuse index; no rewrite | U | | ⬜ |
| S5.4 | Append disabled | `storage.incremental_append=false` | Full rebuild on any fingerprint mismatch | U | | ⬜ |
| S5.5 | Growth e2e | Append to temp log between runs | Second investigate returns new lines only in index | E | Yes | ⬜ |
| S5.6 | mtime-only change | Size unchanged, mtime changed | Documented: treat as unchanged if size matches | U | | ⬜ |

---

# 8. FTS5 full-text search (S6)

| ID | Scenario | Trigger | Expected behavior | Test | Reg | Status |
|----|----------|---------|-------------------|------|-----|--------|
| S6.1 | Index population | `--persist-index` | `lines_fts` synced on insert/batch commit | U | | ⬜ |
| S6.2 | contains pushdown | `contains(message, "timeout")` | FTS `MATCH` when index present | U+I | Yes | ⬜ |
| S6.3 | M7 search | `search --search "timeout"` on persisted index | FTS path when store attached | I+E | Yes | ⬜ |
| S6.4 | Memory-only index | No `--persist-index` | Existing in-memory search unchanged | U | | ⬜ |
| S6.5 | v1 index rebuild | Open v1 DB | FTS populated on rebuild | I | | ⬜ |
| S6.6 | Benchmark | 100k-line persisted index | `BM_FtsSearch` baseline in CI | B | | ⬜ |

**Build:** SQLite compiled with `SQLITE_ENABLE_FTS5`.

---

# 9. Cross-cutting scenarios

| ID | Scenario | Expected behavior | Test | Status |
|----|----------|-------------------|------|--------|
| X.1 | Release CLI matrix | 100k-line persist + filter + search on all OSes | E (release.yml) | ⬜ |
| X.2 | Windows zlib link | Build + test on Windows CI | CI | ⬜ |
| X.3 | Config validate | New `storage.*` keys accepted by `config validate` | U | ⬜ |
| X.4 | Backward CLI | v1.4.2 commands unchanged without new config | E | ⬜ |

---

# 10. Implementation phases

| Phase | Branch | Scenarios |
|-------|--------|-----------|
| M11.7 | `feat/v1.4.3-schema-v2` | S1.* |
| M11.8 | `feat/v1.4.3-compression` | S2.* |
| M11.9 | `feat/v1.4.3-json-fields` | S3.* |
| M11.10 | `feat/v1.4.3-query-cache` | S4.* |
| M11.11 | `feat/v1.4.3-incremental-append` | S5.* |
| M11.12 | `feat/v1.4.3-fts5` | S6.*, X.* |

**Gate:** `docs/v1.4.3-design` merges before any `feat/v1.4.3-*` branch.

---

# 11. Related documents

| Document | Use when |
|----------|----------|
| [M11-STORAGE-LAYER.md](M11-STORAGE-LAYER.md) | Phase table and release targets |
| [ADR-005](../architecture/decisions/ADR-005-Storage-Architecture.md) | Schema DDL and migration policy |
| [CONFIGURATION_GUIDE.md](../handbook/CONFIGURATION_GUIDE.md) | New `storage.*` keys |
| [USER_MANUAL.md](../handbook/USER_MANUAL.md) | End-user incremental append and cache behavior |
| [PERFORMANCE.md](../testing/PERFORMANCE.md) | Benchmark SLAs |

---

# 12. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 24-07-2026 | Initial v1.4.3 scenario matrix (design). |
