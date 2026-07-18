# LogScope

LogScope is a generic log analysis platform designed to help engineers understand machine-generated log data through a consistent and reusable workflow.

For the full product description, see [Product Overview](vision/PRODUCT_OVERVIEW.md).

## Problem

Engineers spend significant time manually searching large log files to identify recurring errors, patterns, and important events — often using different tools for different log formats and systems.

## Goal

LogScope provides a technology-independent approach to:

- Parse and normalize logs from any supported format
- Search, filter, and investigate log data
- Generate reports and insights
- Extend capabilities through a plugin architecture

## Current Status

M3 – Architecture Realization is **complete** as of [`v0.3.0`](../CHANGELOG.md). The core pipeline is operational:

```text
Configuration → Source → Analysis → Investigation → Reporting → CLI
```

**M4 – Feature Expansion** is in progress. Phased plan:

| Phase | Focus |
|-------|-------|
| M4.1 | Analysis depth (log level stats, content-aware investigation) |
| M4.2 | Additional source types (stdin, directories) |
| M4.3 | Advanced reporting (sections, CSV, Markdown) |
| M4.4 | Extension ecosystem |
| M4.5 | Session / workspace |

See [M4 – Feature Expansion](planning/M4-FEATURE-EXPANSION.md) and [Roadmap](ROADMAP.md) for details.

## Initial Delivery

The first executable delivers basic log file analysis with text and JSON output. Long-term capabilities are defined in the functional requirements (FR-001 through FR-004).
