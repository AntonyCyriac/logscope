# Configuration Guide

| Field | Value |
|-------|-------|
| Document | Configuration Guide |
| Category | Handbook |
| Version | 1.0.0 |
| Status | Approved |
| Created | 24-07-2026 |
| Last Updated | 24-07-2026 |

---

# 1. Purpose

This document describes how to configure LogScope through properties files, environment variables, and CLI overrides.

It complements the command reference in [CLI Reference](CLI_REFERENCE.md) with file format rules, validation behavior, and a complete key catalog.

Phase 1 stabilization deliverable — see [Post-v1 Strategic Roadmap](../planning/POST_V1_STRATEGIC_ROADMAP.md#phase-1--stabilize-v1x).

---

# 2. Configuration sources

LogScope merges configuration from these sources (later sources override earlier ones where applicable):

| Source | How |
|--------|-----|
| Built-in defaults | Applied when a key is unset |
| Properties file | `--config <file>` on any command |
| Environment variables | `SCOPE_<KEY>` mapped to dotted keys (see [§3](#3-environment-variables)) |
| CLI flags | Per-command options such as `--profile`, `--log-format`, `--persist-index` |

Example properties file: [`samples/logscope.properties`](../../samples/logscope.properties).

```bash
logscope --config samples/logscope.properties analyze samples/sample.log
```

---

# 3. Properties file format

- Java-style `key=value` lines
- `#` starts a comment (full-line or after content)
- Keys use dot notation: `section.subsection.name`
- Values are strings; booleans accept `true`, `false`, `1`, `0`, `yes`, `no`, `on`, `off` where noted
- Blank lines are ignored
- Keys and values are trimmed of surrounding whitespace

Invalid syntax (missing `=`, empty key) fails at load time with a line number.

---

# 4. Environment variables

Variables prefixed with `SCOPE_` map to configuration keys by lowercasing and replacing `_` with `.`:

| Environment variable | Configuration key |
|---------------------|-------------------|
| `SCOPE_LOG_LEVEL` | `log.level` |
| `SCOPE_PROFILE` | `profile` |
| `SCOPE_STORAGE_MODE` | `storage.mode` |

Environment variables are applied when the configuration manager loads (after the properties file).

---

# 5. Validation

Use `config validate` to check a file before running workflows:

```bash
logscope config validate --config samples/logscope.properties
logscope config validate --config my.properties --require profile,log.level
```

Validation runs:

- Required-key presence (`--require`)
- Analysis keys (`profile`, `source.*`, `investigation.max_indexed_lines`)
- Saved search expressions (`search.saved.*`)
- Saved filter DSL (`query.saved.*`)
- Storage keys (`storage.mode`, `storage.spill_threshold`)

On success the command prints `Configuration is valid.`

---

# 6. Configuration keys

## 6.1 Runtime and diagnostics

| Key | Values | Default | Description |
|-----|--------|---------|-------------|
| `log.level` | `debug`, `info`, `warn`, `warning`, `error` | `info` | Internal diagnostic log level |
| `log.timestamps` | boolean | `true` | Prefix diagnostic messages with timestamps |

## 6.2 Source and format

| Key | Values | Default | Description |
|-----|--------|---------|-------------|
| `profile` | `generic-plain`, `generic-json` | — | Built-in format profile |
| `source.format` | `auto`, `plain`, `jsonl` | `auto` | Input format hint |
| `source.json.timestamp_field` | field name | profile default | JSON timestamp field override |
| `source.json.level_field` | field name | profile default | JSON level field override |

CLI `--profile` and `--log-format` override these for a single run.

## 6.3 Investigation index

| Key | Values | Default | Description |
|-----|--------|---------|-------------|
| `investigation.max_indexed_lines` | integer (1–1000000) | `10000` | In-memory line index capacity before spill/persistence |

## 6.4 Search

| Key | Values | Description |
|-----|--------|-------------|
| `search.saved.<name>` | search expression | Named saved search (boolean/regex syntax per M7) |

Example:

```properties
search.saved.errors=error OR warning
search.saved.timeouts=timeout AND error
```

## 6.5 Query filter DSL

| Key | Values | Description |
|-----|--------|-------------|
| `query.saved.<name>` | filter DSL expression | Named saved filter (M10 grammar) |

Example:

```properties
query.saved.errors=level == ERROR
query.saved.refused=contains(message, "refused")
```

## 6.6 Reporting and analytics

| Key | Values | Default | Description |
|-----|--------|---------|-------------|
| `report.format` | `text`, `json`, `csv`, `markdown`, `html`, `pdf` | `text` | Default report output format |
| `report.sections` | comma-separated section ids or `all` | `all` | Sections to include |
| `report.include_charts` | boolean | `true` | Include chart sections when data is available |
| `report.include_timeline` | boolean | `true` | Include timeline chart in `charts` section |
| `report.template` | template name | `default` | Report template selector |
| `analytics.bucket_seconds` | positive integer | auto | Timeline bucket size in seconds |
| `analytics.top_n` | positive integer | `10` | Top frequency/cluster results |
| `analytics.min_cluster_count` | positive integer | `2` | Minimum occurrences to surface a cluster |

CLI `--format`, `--sections`, and analytics flags override report/analytics options per run. See [CLI Reference](CLI_REFERENCE.md).

## 6.7 Storage (M11)

| Key | Values | Default | Description |
|-----|--------|---------|-------------|
| `storage.mode` | `memory`, `hybrid`, `persistent` | `memory` | Index storage behavior |
| `storage.index.directory` | path | platform workspace | Directory for auto-generated SQLite indexes |
| `storage.index.path` | file path | — | Explicit SQLite index file (implies persistent mode) |
| `storage.spill_threshold` | line count | memory cap | Optional spill threshold override |

Default index directories:

| Platform | Path |
|----------|------|
| Windows | `%LOCALAPPDATA%\logscope\indexes\` |
| Unix | `~/.logscope/indexes/` |
| Fallback | `.logscope/indexes/` |

CLI flags `--persist-index`, `--reuse-index`, and `--index-path` override storage settings per run.

## 6.8 Extensions

| Key | Values | Default | Description |
|-----|--------|---------|-------------|
| `extensions.<id>.enabled` | boolean | extension default | Enable or disable a registered extension |

Example:

```properties
extensions.reporting.multi-format.enabled=true
```

List extensions with `logscope extensions list --config <file>`.

---

# 7. Example configuration

```properties
# Runtime
log.level=info
log.timestamps=true

# Format
profile=generic-json
source.format=auto

# Investigation
investigation.max_indexed_lines=10000

# Saved workflows
search.saved.errors=error OR warning
query.saved.errors=level == ERROR

# Reporting
report.format=text
report.include_charts=true

# Storage (optional — enable for large or repeat investigations)
# storage.mode=hybrid
# storage.index.directory=~/.logscope/indexes
```

---

# 8. Related documents

| Document | Purpose |
|----------|---------|
| [CLI Reference](CLI_REFERENCE.md) | Commands, flags, and per-command options |
| [Developer Setup](DEVELOPER_SETUP.md) | Build environment and workflow |
| [M6 – Log Format Intelligence](../planning/M6-LOG-FORMAT-INTELLIGENCE.md) | Format profiles and field extraction |
| [M10 – Query Language](../planning/M10-QUERY-LANGUAGE.md) | Filter DSL grammar |
| [M11 – Storage Layer](../planning/M11-STORAGE-LAYER.md) | Persistent index architecture |
| [ADR-005 – Storage Architecture](../architecture/decisions/ADR-005-Storage-Architecture.md) | Storage design decisions |

---

# 9. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 24-07-2026 | Initial Phase 1 configuration guide. |
