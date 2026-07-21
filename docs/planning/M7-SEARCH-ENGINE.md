# M7 – Search Engine

| Field | Value |
|-------|-------|
| Document | M7 – Search Engine |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | Approved |
| Created | 21-07-2026 |
| Last Updated | 21-07-2026 |

---

# 1. Purpose

This document defines **M7 – Search Engine**, the dedicated search subsystem built on [M6 – Log Format Intelligence](M6-LOG-FORMAT-INTELLIGENCE.md).

Target release: **`v1.2.0`**.

---

# 2. Dependency on M6

| M6 output | M7 dependency |
|-----------|---------------|
| M6.3 — Field extraction | Search indexes structured fields (timestamp, level, message) |
| M6.4 — Content-aware investigation | Foundation for full-text and field-based search |
| M6.5 — Format profiles | Configurable field mappings for search indexing |

---

# 3. Phased Delivery

| Phase | Focus | Status |
|-------|-------|--------|
| M7.1 | `scope_search` module, `SearchQuery`, text matching | ✅ Complete |
| M7.2 | Regex search mode | ✅ Complete |
| M7.3 | Boolean query parser (AND, OR, NOT) | ✅ Complete |
| M7.4 | CLI `--query`, `--regex`, `--case-sensitive`, `search` subcommand | ✅ Complete |
| M7.5 | Session v1.2 search history, saved searches, benchmarks, `v1.2.0` release | ✅ Complete |

---

# 4. Delivered Capabilities

## Core search

- Full-text search over indexed log lines (case-insensitive by default)
- Regex search via `--regex`
- Boolean queries via `--query` (AND, OR, NOT, quoted phrases, parentheses)
- Case-sensitive mode via `--case-sensitive`

## Filtering

- Time range and field filters (from M6.4) combine with search queries
- Investigation index bounds from `investigation.max_indexed_lines`

## User workflow

- `logscope search` subcommand (alias of investigate for search-focused output)
- Session serializer v1.2: `search.query`, `search.history`
- Named saved searches via `search.saved.<name>` configuration keys
- Truncation warning when indexed lines are capped

## Deferred

- Streaming re-read of source beyond index bounds (known limitation)
- SQL-like query language (M10)
- Persistent SQLite index (M11)

---

# 5. Engineering Conventions

| Convention | Value |
|------------|-------|
| Module | `core/search` (`scope_search`) |
| Tests | `scope_search_tests` + CLI/e2e coverage |
| Benchmark | `BM_SearchEngineTextQuery` |

---

# 6. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 0.1.0 | 21-07-2026 | Initial M7 scope stub. |
| 1.0.0 | 21-07-2026 | M7 complete; v1.2.0 released. |
