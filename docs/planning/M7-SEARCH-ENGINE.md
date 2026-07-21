# M7 – Search Engine

| Field | Value |
|-------|-------|
| Document | M7 – Search Engine |
| Category | Project Planning |
| Version | 0.1.0 |
| Status | Draft |
| Created | 21-07-2026 |
| Last Updated | 21-07-2026 |

---

# 1. Purpose

This document defines the planned scope for **M7 – Search Engine**, the first major product-feature milestone after [M6 – Log Format Intelligence](M6-LOG-FORMAT-INTELLIGENCE.md).

M7 delivers a dedicated search subsystem that lets users find information in log data quickly and progressively. This is the #1 product priority in the [Post-v1 Strategic Roadmap](POST_V1_STRATEGIC_ROADMAP.md).

Target release: **`v1.2.0`**.

---

# 2. Dependency on M6

M7 builds directly on M6 deliverables:

| M6 output | M7 dependency |
|-----------|---------------|
| M6.3 — Field extraction | Search indexes structured fields (timestamp, level, message) |
| M6.4 — Content-aware investigation | Foundation for full-text and field-based search |
| M6.5 — Format profiles | Configurable field mappings for search indexing |

M7 should not begin until M6 is complete and `v1.1.0` is released.

---

# 3. Planned Capabilities

## Core search

- Full-text search over indexed log lines
- Regex search
- Boolean queries (AND, OR, NOT)
- Case-sensitive and case-insensitive modes

## Filtering

- Time range filtering (requires M6 timestamp extraction)
- Field filtering (level, service, correlation ID, custom JSON keys)
- Combined filter + search queries

## User workflow

- Saved searches (persisted in workspace/session)
- Search history
- CLI subcommands or flags for interactive search
- Progressive refinement without re-analysis (FR-002.5)

---

# 4. Non-Goals for M7

The following are explicitly deferred beyond M7:

- SQL-like or full query language (see M10)
- Persistent SQLite index (see M11)
- AI-assisted search suggestions (see M13)
- GUI search panels (see M14)
- REST API search endpoints (see M15)

---

# 5. Engineering Considerations

- **Indexing strategy:** In-memory index from M6 line store; streaming fallback for files exceeding index bounds.
- **Performance:** Benchmark search latency against M5 baselines; target sub-second search on indexed fixtures.
- **Memory bounds:** Reuse M6 `investigation.max_indexed_lines` configuration.
- **Tests:** Unit tests for query parsing; integration tests with JSON and plain-text fixtures; e2e CLI search scenarios.

Full phased breakdown (M7.1–M7.N), acceptance criteria, and branch naming will be added when M6 nears completion.

---

# 6. Traceability

| Source artifact | Relationship |
|-----------------|--------------|
| [FR-002 – Investigate Logs](../requirements/functional/FR-002-Investigate-Logs.md) | Search, filter, progressive investigation |
| [M6 – Log Format Intelligence](M6-LOG-FORMAT-INTELLIGENCE.md) | Prerequisite milestone |
| [Post-v1 Strategic Roadmap](POST_V1_STRATEGIC_ROADMAP.md) | Strategic context and version target |
| [Roadmap](../ROADMAP.md) | Milestone tracking |

---

# 7. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 0.1.0 | 21-07-2026 | Initial M7 scope stub; full plan deferred until M6 completion. |
