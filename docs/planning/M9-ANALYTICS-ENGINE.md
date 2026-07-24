# M9 – Analytics Engine

| Field | Value |
|-------|-------|
| Document | M9 – Analytics Engine |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | Approved |
| Created | 24-07-2026 |
| Last Updated | 24-07-2026 |

---

# 1. Purpose

This document defines **M9 – Analytics Engine**, extending LogScope with higher-level analysis beyond M6 aggregates and M6.4 basic correlation: frequency, error clustering, timeline/trend views, and time-series charts.

Target release: **`v1.3.1`**.

---

# 2. Dependency on M6–M8

| Prior output | M9 dependency |
|--------------|---------------|
| M6.3 — Field extraction | Time range for timeline bucket sizing |
| M6.4 — Line index | Primary analytics input (bounded index) |
| M7 — Search engine | No direct dependency |
| M8 — Reporting | Section registry and chart model for analytics sections |

---

# 3. Phased Delivery

| Phase | Focus | Status |
|-------|-------|--------|
| M9.1 | `scope_analytics` module, `FrequencyAnalyzer`, planning doc | ✅ Complete |
| M9.2 | `ErrorClusterer` with normalized message grouping | ✅ Complete |
| M9.3 | `TimelineAnalyzer`, `TrendAnalyzer` | ✅ Complete |
| M9.4 | `CorrelationAnalyzer`, `logscope analytics` CLI, config keys | ✅ Complete |
| M9.5 | Time-series charts, report sections, `v1.3.1` release | ✅ Complete |

---

# 4. Delivered Capabilities

## Core analytics

- `AnalyticsEngine` orchestrates frequency, clustering, timeline, trend, and correlation analysis over `LineIndex`
- `FrequencyAnalyzer` — per-level, message, and correlation-ID frequency tables
- `ErrorClusterer` — normalized signature grouping beyond exact message match
- `TimelineAnalyzer` — auto or configured time buckets with per-level counts
- `TrendAnalyzer` — half-window rate comparison and spike detection
- `CorrelationAnalyzer` — repeated errors and shared correlation IDs (refactored from investigation)

## CLI

- `logscope analytics` subcommand with `--bucket`, `--top`, `--format text|json`
- Config keys: `analytics.bucket_seconds`, `analytics.min_cluster_count`, `analytics.top_n`

## Reporting

- Sections: `analytics`, `timeline`, `clusters`
- Time-series charts in `charts` section when timeline data is available
- Config: `report.include_timeline`

---

# 5. Engineering Conventions

| Convention | Value |
|------------|-------|
| Module | `core/analytics` (`scope_analytics`) |
| Tests | `scope_analytics_tests` + CLI/e2e coverage |
| Benchmark | `BM_AnalyticsEngine` |

---

# 6. Deferred

- Root cause assistance / NL suggestions (M13)
- Analytics beyond `LineIndex` bounds (M11)
- SQL/DSL query language (M10)
- Dynamic analytics plugins (M12)
- Interactive dashboards / REST (M15)
