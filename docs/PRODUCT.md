# LogScope

LogScope is a generic log analysis platform designed to help engineers understand machine-generated log data through a consistent and reusable workflow. Over time it aims to support broader **system investigation** (answering “what happened?”), while remaining CLI-first and log-centric in the near term.

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

LogScope **v2.2.1** ships M15 Web Platform (complete): REST API + browser SPA, shared workspaces, async analyze, tail poll, and M15.4 thin auth (session TTL, upload cleanup). Delivery surfaces: **CLI**, **desktop** (`logscope-desktop`), and **web** (`logscope-web`). M0–M15 are complete; M12 plugins, M13 AI assistant, and M14 desktop shipped in earlier v1.x–v2.0.x releases.

```text
Source → Analysis → Investigation → Reporting
         ↑__________________________|
              CLI · Desktop · Web
```

Capabilities include format intelligence, search and query DSL, HTML/PDF reporting, analytics, SQLite persistent indexes, dynamic plugins, AI-assisted investigation, multi-OS CI, benchmarks, fuzz testing, and installable/binary distribution on Windows, macOS, and Linux.

See [Roadmap](ROADMAP.md) and [Changelog](../CHANGELOG.md) for milestone history.

## Initial Delivery

The first executable delivers basic log file analysis with text and JSON output. Long-term capabilities are defined in the functional requirements (FR-001 through FR-004).
