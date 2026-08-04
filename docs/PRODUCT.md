# LogScope

LogScope is a generic log analysis platform designed to help engineers understand machine-generated log data through a consistent and reusable workflow. Over time it aims to support broader **system investigation** (answering “what happened?”), while remaining CLI-first and log-centric in the near term.

**Living product:** LogScope evolves every day — continuous delivery on `master`, tagged releases for stability, and public [Roadmap](ROADMAP.md) / [Changelog](../CHANGELOG.md) for traceability.

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

LogScope **v2.3.0** ships **Story 1: Create an Investigation** — portable incident containers with log + note artifacts (CLI, REST, web). M15 Web Platform complete through `v2.2.2`. Delivery surfaces: **CLI**, **desktop** (`logscope-desktop`), and **web** (`logscope-web`).

**Next:** **`v2.4.0`** — Story 2: Understand Everything (multi-source correlation).

```text
Source → Analysis → Investigation → Reporting
         ↑__________________________|
              CLI · Desktop · Web
```

Capabilities include format intelligence, search and query DSL, HTML/PDF reporting, analytics, SQLite persistent indexes, dynamic plugins, AI-assisted investigation, multi-OS CI, benchmarks, fuzz testing, and installable/binary distribution on Windows, macOS, and Linux.

See [Roadmap](ROADMAP.md) and [Changelog](../CHANGELOG.md) for milestone history.

## Initial Delivery

The first executable delivers basic log file analysis with text and JSON output. Long-term capabilities are defined in the functional requirements (FR-001 through FR-004).
