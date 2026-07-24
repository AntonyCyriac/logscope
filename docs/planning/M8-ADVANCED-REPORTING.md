# M8 – Advanced Reporting

| Field | Value |
|-------|-------|
| Document | M8 – Advanced Reporting |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | Approved |
| Created | 24-07-2026 |
| Last Updated | 24-07-2026 |

---

# 1. Purpose

This document defines **M8 – Advanced Reporting**, extending the reporting subsystem delivered in M3 with section architecture, new report sections, charts, HTML/PDF output, and plugin contributor hooks.

Target release: **`v1.3.0`**.

---

# 2. Dependency on M6/M7

| Prior output | M8 dependency |
|--------------|---------------|
| M6.3 — Field extraction | Executive/error summaries use `FieldSummary` time range and top messages |
| M6.4 — Line index | Error excerpts from indexed error lines |
| M7 — Search engine | No direct dependency (investigation correlation deferred) |

---

# 3. Phased Delivery

| Phase | Focus | Status |
|-------|-------|--------|
| M8.1 | Section registry, refactor `ReportFormatter`, planning doc, section parser | ✅ Complete |
| M8.2 | Executive and error summary sections (all formats) | ✅ Complete |
| M8.3 | Chart model, ASCII/SVG level bar charts | ✅ Complete |
| M8.4 | HTML format, `--output` file flag, CLI/e2e tests | ✅ Complete |
| M8.5 | ADR-003 PDF strategy, PDF renderer, contributor hook, `v1.3.0` release | ✅ Complete |

---

# 4. Delivered Capabilities

## Section architecture

- `ReportSectionRenderer` registry with `ReportFragment` intermediate content
- `FormatRenderer` assembles fragments into text, JSON, CSV, Markdown, HTML, and PDF
- Backward-compatible section names: `summary`, `levels`, `metadata`, `all`

## New sections

- `executive` — health verdict, rates, format, time range
- `errors` — error/warning counts, top patterns, indexed error excerpts
- `charts` — ASCII/Markdown bar charts and SVG for HTML
- `formats` — contributor footer listing available output formats

## New formats

- `html` — self-contained HTML document with embedded CSS and SVG charts
- `pdf` — native PDF via minimal in-house writer (ADR-003)

## CLI

- `--format html|pdf`
- `--output <file>` on `analyze` and `session load`
- Config keys: `report.format`, `report.include_charts`, `report.template`

## Extensions

- `report_section_contributor` hook with static registration from `reporting.multi-format`

---

# 5. Engineering Conventions

| Convention | Value |
|------------|-------|
| Module | `core/reporting` (`scope_reporting`) |
| Tests | `scope_reporting_tests` + CLI/e2e coverage |
| ADR | ADR-003 — PDF library selection |

---

# 6. Deferred

- M9 time-series analytics charts
- M12 dynamic plugin `.dll` loading for contributors
- Interactive dashboards and REST export (M15)
