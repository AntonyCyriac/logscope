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

Development focus shifts to **M4 – Feature Expansion**. See [Roadmap](ROADMAP.md) and [Changelog](../CHANGELOG.md) for details.

## Initial Delivery

The first executable delivers basic log file analysis with text and JSON output. Long-term capabilities are defined in the functional requirements (FR-001 through FR-004).
