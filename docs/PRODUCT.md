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

LogScope **v1.4.2** delivers M11 bulk index write performance on top of the v1.4.1 storage layer. M0–M11 are complete.

```text
Configuration → Source → Analysis → Investigation → Reporting → CLI
```

Capabilities include analysis depth, stdin/directory sources, multi-format reporting, extensions, session persistence, multi-OS CI, benchmarks, fuzz testing, and installable/binary distribution.

See [Roadmap](ROADMAP.md) and [Changelog](../CHANGELOG.md) for milestone history.

## Initial Delivery

The first executable delivers basic log file analysis with text and JSON output. Long-term capabilities are defined in the functional requirements (FR-001 through FR-004).
