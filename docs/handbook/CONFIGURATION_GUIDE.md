# Configuration Guide

| Field | Value |
|-------|-------|
| Document | Configuration Guide |
| Category | Handbook |
| Version | 1.3.0 |
| Status | Approved |
| Created | 24-07-2026 |
| Last Updated | 31-07-2026 |

---

# 1. Purpose

This document describes how to configure LogScope through properties files, environment variables, and CLI overrides.

It complements the command reference in [CLI Reference](CLI_REFERENCE.md) with file format rules, validation behavior, and a complete key catalog. End-user workflows are in [User Manual](USER_MANUAL.md).

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

## 6.7.1 Storage v1.4.3

| Key | Values | Default | Description |
|-----|--------|---------|-------------|
| `storage.compress_content` | boolean | `false` | zlib-compress persisted `content` column |
| `storage.compress_threshold_bytes` | integer | `256` | Minimum line length to compress |

**Compression toggle:** enabling compression on an existing plain index requires a full rebuild (see USER_MANUAL §8).

### Shipped (v1.4.3)

| Key | Values | Default | Description |
|-----|--------|---------|-------------|
| `storage.query_cache.enabled` | boolean | `true` | Cache filter results on persisted indexes |
| `storage.query_cache.max_entries` | integer | `64` | LRU cap for `query_cache` rows |
| `storage.incremental_append` | boolean | `true` | Append new lines when source file grows |

## 6.8 Extensions

| Key | Values | Default | Description |
|-----|--------|---------|-------------|
| `extensions.<id>.enabled` | boolean | extension default | Enable or disable a registered extension |

Example:

```properties
extensions.reporting.multi-format.enabled=true
```

List extensions with `logscope extensions list --config <file>`.

See [Plugin Development Guide](../handbook/PLUGIN_DEVELOPMENT_GUIDE.md) for adding built-in extensions.

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

# Dynamic plugins (v1.5.0+)
# plugins.enabled=true
# plugins.paths=/opt/logscope/plugins

# AI assistant (v1.5.1+)
# ai.enabled=false
# ai.provider=noop
# ai.endpoint=
# ai.model=
# ai.max_context_lines=200
# analysis.plugin_format=pipe-delimited
# investigation.search_provider=sample.search
# storage.backend=sqlite
```

---

# 8. Plugin configuration (v1.5.0+)

| Key | Default | Description |
|-----|---------|-------------|
| `plugins.enabled` | `false` | When `true`, scan `plugins.paths` and `LOGSCOPE_PLUGIN_PATH` for `.so`/`.dll` libraries. |
| `plugins.paths` | (unset) | Semicolon-separated (Windows) or colon-separated (Unix) directories or library paths. |
| `analysis.plugin_format` | (unset) | Format id registered by a parser plugin (e.g. `pipe-delimited`). |
| `investigation.search_provider` | (unset) | Plugin search provider id for investigation/search paths. |
| `storage.backend` | `sqlite` | `sqlite` or `plugin:<backend-id>` for plugin storage backends. |

Environment: `LOGSCOPE_PLUGIN_PATH` augments `plugins.paths`.

---

# 9. AI configuration (v1.5.1+)

| Key | Default | Description |
|-----|---------|-------------|
| `ai.enabled` | `false` | When `false`, the noop provider is used and no outbound AI calls occur. |
| `ai.provider` | `noop` | `noop` or `http` (OpenAI-compatible API). |
| `ai.endpoint` | (unset) | Base URL for HTTP provider (required when `ai.provider=http`). |
| `ai.model` | (unset) | Model name for HTTP provider (required when `ai.provider=http`). |
| `ai.max_context_lines` | `200` | Maximum log lines included in AI context. |

Environment: `LOGSCOPE_AI_API_KEY` — bearer token for HTTP provider (never store in properties files).

Sample configs:

| File | Use case |
|------|----------|
| `samples/ai-noop.properties` | Offline noop provider (CI, no network) |
| `samples/ai-ollama.properties` | Local [Ollama](https://ollama.com) at `http://localhost:11434/v1`; set `LOGSCOPE_AI_API_KEY` to any non-empty value |
| `samples/ai-openai.properties.example` | OpenAI-compatible cloud API template — copy locally; set `LOGSCOPE_AI_API_KEY` in the environment |

See [samples/README.md](../../samples/README.md) for full examples.

### Example — local Ollama

```properties
ai.enabled=true
ai.provider=http
ai.endpoint=http://localhost:11434/v1
ai.model=llama3.2
```

```bash
export LOGSCOPE_AI_API_KEY=ollama
logscope agent investigate --config samples/ai-ollama.properties --summarize samples/sample.log
```

### Example — OpenAI

```properties
ai.enabled=true
ai.provider=http
ai.endpoint=https://api.openai.com/v1
ai.model=gpt-4o-mini
```

```bash
export LOGSCOPE_AI_API_KEY="sk-..."
logscope agent investigate --config my-openai.properties --summarize samples/sample.log
```

---

# 10. Web platform (`logscope-web`)

Keys apply to `logscope-web` (M15 / ADR-009). Load via `--config` or environment (`LOGSCOPE_WEB_*`). See [Securing logscope-web](SECURING_LOGSCOPE_WEB.md) for deployment guidance.

| Key | Default | Description |
|-----|---------|-------------|
| `web.bind_host` | `127.0.0.1` | Listen address |
| `web.bind_port` | `8080` | Listen port |
| `web.api_key` | empty | Legacy plaintext key (startup warning in v2.2.2+); prefer `web.api_key_hash` |
| `web.api_key_hash` | empty | Salted SHA-256 hash at rest (`sha256:<salt>:<digest>`); generate with `logscope-web --hash-api-key` |
| `web.health_requires_api_key` | `false` | When `true` and `api_key` set, `GET /api/v1/health` requires the key |
| `web.cors_origins` | (derived) | Comma-separated allowed browser origins |
| `web.tls_cert` / `web.tls_key` | empty | PEM paths for embedded HTTPS (OpenSSL build) |
| `web.max_upload_bytes` | 268435456 | Max multipart upload size (bytes) |
| `web.upload_temp_dir` | (system temp) | Directory for staged upload files |
| `web.allow_server_paths` | `false` | Allow `POST /api/v1/sources/open` with server paths |
| `web.allowed_path_roots` | empty | Allowlist for server path open |
| `web.request_timeout_seconds` | `300` | HTTP read/write timeout (sync analyze) |
| `web.workspace_dir` | `{data_dir}/workspaces` | Shared workspace persistence root (M15.3) |
| `web.workspaces_list_limit` | `100` | Max workspaces in list response |
| `web.async_analyze_threshold_bytes` | `10485760` | Async analyze threshold (10 MiB) |
| `web.job_ttl_seconds` | `3600` | Completed/failed job record TTL |
| `web.job_max_concurrent_per_session` | `1` | Max in-flight analyze jobs per session |
| `web.session_ttl_seconds` | `0` | Idle session TTL; `0` disables eviction (M15.4) |
| `web.max_sessions` | `0` | Max in-memory sessions; `0` unlimited (M15.4) |

Environment overrides:

| Variable | Maps to |
|----------|---------|
| `LOGSCOPE_WEB_API_KEY` | `web.api_key` (plaintext override) |
| `LOGSCOPE_WEB_API_KEY_HASH` | `web.api_key_hash` |
| `LOGSCOPE_WEB_BIND_HOST` | `web.bind_host` |
| `LOGSCOPE_WEB_BIND_PORT` | `web.bind_port` |
| `LOGSCOPE_WEB_TLS_CERT` / `LOGSCOPE_WEB_TLS_KEY` | TLS paths |
| `LOGSCOPE_WEB_WORKSPACE_DIR` | `web.workspace_dir` |
| `LOGSCOPE_WEB_WORKSPACES_LIST_LIMIT` | `web.workspaces_list_limit` |
| `LOGSCOPE_WEB_ASYNC_ANALYZE_THRESHOLD_BYTES` | `web.async_analyze_threshold_bytes` |
| `LOGSCOPE_WEB_JOB_TTL_SECONDS` | `web.job_ttl_seconds` |
| `LOGSCOPE_WEB_JOB_MAX_CONCURRENT_PER_SESSION` | `web.job_max_concurrent_per_session` |
| `LOGSCOPE_WEB_SESSION_TTL_SECONDS` | `web.session_ttl_seconds` |
| `LOGSCOPE_WEB_MAX_SESSIONS` | `web.max_sessions` |
| `LOGSCOPE_WEB_HEALTH_REQUIRES_API_KEY` | `web.health_requires_api_key` (`true`/`1`/`yes`) |

Sample: `samples/web.properties`.

---

# 11. Related documents

| Document | Purpose |
|----------|---------|
| [Securing logscope-web](SECURING_LOGSCOPE_WEB.md) | `logscope-web` API key, bind, TLS, session TTL |
| [Developer Setup](DEVELOPER_SETUP.md) | Build environment and workflow |
| [M6 – Log Format Intelligence](../planning/M6-LOG-FORMAT-INTELLIGENCE.md) | Format profiles and field extraction |
| [M10 – Query Language](../planning/M10-QUERY-LANGUAGE.md) | Filter DSL grammar |
| [M11 – Storage Layer](../planning/M11-STORAGE-LAYER.md) | Persistent index architecture |
| [M11 v1.4.3 Scenarios](../planning/M11-V143-STORAGE-SCENARIOS.md) | v1.4.3 acceptance scenarios |
| [ADR-005 – Storage Architecture](../architecture/decisions/ADR-005-Storage-Architecture.md) | Storage design decisions |

---

# 9. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 24-07-2026 | Initial Phase 1 configuration guide. |
| 1.2.0 | 25-07-2026 | M12 plugin configuration keys (`plugins.*`, provider selectors). |
| 1.3.0 | 31-07-2026 | §10 Web platform (`web.*`) + M15.4 thin auth keys. |
