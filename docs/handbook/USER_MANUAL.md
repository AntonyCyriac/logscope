# User Manual

| Field | Value |
|-------|-------|
| Document | User Manual |
| Category | Handbook |
| Version | 1.2.0 |
| Status | Approved |
| Created | 24-07-2026 |
| Last Updated | 24-07-2026 |

---

# 1. Purpose

This manual describes how to use LogScope from the command line to analyze logs, investigate incidents, and produce reports.

It is written for engineers and operators who work with log files day to day. For exact flag syntax, see [CLI Reference](CLI_REFERENCE.md). For properties files, see [Configuration Guide](CONFIGURATION_GUIDE.md).

Phase 1 stabilization deliverable — see [Post-v1 Strategic Roadmap](../planning/POST_V1_STRATEGIC_ROADMAP.md#phase-1--stabilize-v1x).

---

# 2. Getting started

## 2.1 Install

**Pre-built binaries** — download the archive for your OS from [GitHub Releases](https://github.com/AntonyCyriac/logscope/releases) (`v1.4.2` or later) and add `logscope` to your `PATH`.

**Build from source** — see [Developer Setup](DEVELOPER_SETUP.md) or the [README](../../README.md) quick start.

## 2.2 First run

Analyze the included sample log:

```bash
logscope analyze samples/sample.log
```

Legacy shorthand (same as `analyze`):

```bash
logscope samples/sample.log
```

You should see a text report with level counts, error summaries, and metadata about the log source.

## 2.3 Log sources

LogScope accepts:

| Source | Example |
|--------|---------|
| Single file | `samples/sample.log` |
| JSON Lines file | `samples/sample.jsonl` |
| Directory of `.log` files | `/var/log/myapp/` |
| Standard input | `logscope analyze - < /var/log/app.log` |

Use `--log-format plain|jsonl` or `--profile generic-plain|generic-json` when auto-detection is not enough.

## 2.4 Help

```bash
logscope --help
logscope analyze --help
logscope investigate --help
```

Each subcommand accepts `-h` or `--help` for usage text. Full flag reference: [CLI Reference](CLI_REFERENCE.md).

---

# 3. Core concepts

## 3.1 Analysis vs investigation

| Mode | Command | What you get |
|------|---------|--------------|
| **Analysis** | `analyze` | Aggregated report: level breakdown, errors, charts, analytics sections |
| **Investigation** | `investigate`, `search`, `query` | Matching log lines with filters and search |
| **Analytics** | `analytics` | Frequency, clustering, timeline, and correlation summaries |

Start with `analyze` for an overview. Use `investigate` or `search` when you need to find specific lines.

## 3.2 Line index

LogScope indexes log lines in memory (default cap: 10,000 lines, configurable). Investigation and search run against this index.

For larger logs or repeat visits, use **persistent indexes** (`--persist-index`, `--reuse-index`) — see [§8](#8-large-logs-and-persistent-indexes).

## 3.3 Sessions

A **session** (`.logscope-session` file) saves analysis and investigation state so you can reload a report without re-reading the log. Sessions can reference a persisted SQLite index when the source file has not changed.

---

# 4. Workflow: analyze and report

## 4.1 Text report (default)

```bash
logscope analyze samples/sample.log
```

## 4.2 JSON for automation

```bash
logscope analyze --format json samples/sample.log
```

## 4.3 HTML or PDF for sharing

```bash
logscope analyze --format html --output reports/incident.html samples/sample.log
logscope analyze --format pdf --output reports/incident.pdf samples/sample.log
```

PDF and file output require `--output`. Parent directories are created automatically.

## 4.4 Focused sections

```bash
logscope analyze --sections executive,errors,charts samples/sample.log
```

Available sections: `executive`, `summary`, `levels`, `errors`, `analytics`, `timeline`, `clusters`, `charts`, `metadata`, `formats`, or `all`.

## 4.5 JSON Lines logs

```bash
logscope analyze --profile generic-json samples/sample.jsonl
```

Override field names in a config file if your JSON uses different keys — see [Configuration Guide](CONFIGURATION_GUIDE.md).

## 4.6 CSV and Markdown export

Export tabular or wiki-friendly reports:

```bash
logscope analyze --format csv --sections summary,levels samples/sample.log
logscope analyze --format markdown --sections executive,errors samples/sample.log
```

`csv` emits section/key/value rows for spreadsheets. `markdown` emits tables and ASCII charts suitable for wikis or paste into tickets.

## 4.7 Investigation thresholds on reports

The `analyze` command always reports on the full log source. To capture a session only when a source meets minimum error, warning, or line counts, use `session save` with `--min-errors`, `--min-warnings`, `--min-lines`, or `--max-lines` — see [§7.1](#71-session-save-with-filters-and-indexes).

## 4.8 Persistent index during analyze

Build a SQLite index while producing a report (same storage flags as `investigate`):

```bash
logscope analyze --persist-index samples/large-app.log
logscope analyze --persist-index --index-path /var/log/large-app.index.db samples/large-app.log
```

Reuse on a later run with `--reuse-index`. See [§8](#8-large-logs-and-persistent-indexes).

---

# 5. Workflow: investigate an incident

Typical flow when errors spike in production logs.

## 5.1 Filter by level

```bash
logscope investigate --level error /var/log/app.log
```

## 5.2 Text search

```bash
logscope search --search "connection refused" samples/sample.log
```

## 5.3 Boolean search

```bash
logscope search --query "error AND timeout" samples/sample.log
logscope search --query "(error OR warning) AND NOT debug" samples/sample.log
```

## 5.4 Regular expression

```bash
logscope search --search "code=\d+" --regex samples/sample.log
```

## 5.5 Field filter DSL

Use structured filters on parsed fields:

```bash
logscope investigate --filter 'level == ERROR' samples/sample.log
logscope query --filter 'contains(message, "refused")' samples/sample.log
logscope query --filter 'level == ERROR AND contains(message, "timeout")' samples/sample.log
```

DSL fields: `level`, `time`, `timestamp`, `message`, `content`, `correlationId`, `line`. Functions: `contains(field, "text")`, `hasKey("key")`.

## 5.6 Time range

```bash
logscope investigate \
  --time-from "2026-07-11T10:00:00" \
  --time-to "2026-07-11T10:01:00" \
  --level error \
  samples/sample.log
```

## 5.7 Combine search and filter

```bash
logscope investigate \
  --search "refused" \
  --filter 'level == ERROR' \
  samples/sample.log
```

## 5.8 Message, JSON key, and case sensitivity

Narrow matches by message substring, required JSON field, or case-sensitive search:

```bash
logscope investigate --message "connection refused" samples/sample.log
logscope investigate --json-key requestId samples/sample.jsonl
logscope search --search "ERROR" --case-sensitive samples/sample.log
```

`--message` and `--json-key` work on `investigate` and `search`. `--case-sensitive` applies to `--search` and `--query` (default matching is case-insensitive).

## 5.9 JSON output

```bash
logscope investigate --format json --filter 'level == ERROR' samples/sample.log
logscope search --format json --query "error AND timeout" samples/sample.log
```

---

# 6. Workflow: analytics

Get patterns without manually scanning lines:

```bash
logscope analytics samples/sample.log
logscope analytics --format json --bucket 60 samples/sample.log
```

Analytics includes:

- **Frequency** — most common messages or tokens
- **Clustering** — normalized error signatures
- **Timeline** — event rate over time buckets
- **Correlation** — co-occurring patterns

Include analytics in a full report with `analyze --sections analytics,timeline,clusters,charts`.

---

# 7. Workflow: sessions

## 7.1 Session save with filters and indexes

`session save` accepts the same investigation filters as `investigate`, plus report options. Use it to persist state when a log meets thresholds or when you want filters saved with the session:

```bash
# Save only when the source has at least one error line
logscope session save incident.session samples/sample.log --min-errors 1

# Save with content filters and a persisted index for large logs
logscope session save incident.session /var/log/large-app.log \
  --min-errors 10 \
  --level error \
  --content-search "timeout" \
  --persist-index \
  --sections executive,errors,charts

# Cap total lines considered
logscope session save incident.session samples/sample.log --max-lines 50000
```

Threshold flags: `--min-errors`, `--min-warnings`, `--min-lines`, `--max-lines`. Content filters: `--level`, `--message`, `--json-key`, `--time-from`, `--time-to`, `--content-search`. Storage: `--persist-index`, `--reuse-index`, `--index-path`.

## 7.2 Save and reload

Save state after analysis:

```bash
logscope session save incident.session samples/sample.log
```

Reload the report later (no re-analysis of the log):

```bash
logscope session load incident.session
logscope session load incident.session --output report.html
```

List session files in a directory:

```bash
logscope session list .
```

Sessions store format profile, filters, search queries, and index metadata. When the log file is unchanged, session load can **reuse** a persisted index (`--reuse-index` is the default on load).

---

# 8. Large logs and persistent indexes

For logs that exceed the in-memory index cap, or when you revisit the same file often:

```bash
# Build index on first run
logscope investigate --persist-index --filter 'level == ERROR' /var/log/large-app.log

# Explicit index path
logscope investigate \
  --persist-index \
  --index-path /var/log/large-app.index.db \
  --filter 'level == ERROR' \
  /var/log/large-app.log

# Skip rebuild when source file unchanged (fingerprint match)
logscope investigate --reuse-index --filter 'level == ERROR' /var/log/large-app.log
```

Default index directory:

| Platform | Location |
|----------|----------|
| Windows | `%LOCALAPPDATA%\logscope\indexes\` |
| Linux/macOS | `~/.logscope/indexes/` |

Configure defaults in `logscope.properties` — see [Configuration Guide](CONFIGURATION_GUIDE.md) (`storage.*` keys).

## 8.1 Index build performance (v1.4.2+)

From **v1.4.2**, `--persist-index` uses batched SQLite writes (WAL mode, prepared `INSERT`, 5,000 lines per commit) for faster first-time index builds on large logs.

Progress is written to **stderr** every 10,000 persisted lines when `log.level` is `info` or lower (default). Example:

```text
[INFO] [analysis] Indexed 10000 lines to persistent store
[INFO] [analysis] Indexed 20000 lines to persistent store
```

To see progress during long runs:

```bash
# Default — info level shows progress
logscope investigate --persist-index /var/log/large-app.log

# Explicit via environment or config
SCOPE_LOG_LEVEL=info logscope investigate --persist-index /var/log/large-app.log
log.level=info
```

Set `log.level=warn` or `SCOPE_LOG_LEVEL=warn` to suppress progress lines. See [Configuration Guide](CONFIGURATION_GUIDE.md) (`log.level`).

---

# 9. Configuration

Load a shared properties file:

```bash
logscope --config samples/logscope.properties analyze samples/sample.log
```

Validate before use:

```bash
logscope config validate --config samples/logscope.properties
logscope config validate --config my.properties --require profile,log.level
```

## 9.1 Extensions

List built-in extensions and whether they are enabled:

```bash
logscope extensions list
logscope extensions list --config samples/logscope.properties
```

Show metadata for one extension:

```bash
logscope extensions describe analysis.log-levels
```

Extension enable/disable is controlled through configuration — see [Plugin Development Guide](PLUGIN_DEVELOPMENT_GUIDE.md).

Common uses:

- Set default format profile (`profile=generic-json`)
- Save named searches (`search.saved.errors=error OR warning`)
- Save named filters (`query.saved.errors=level == ERROR`)
- Tune report and analytics defaults

Full key reference: [Configuration Guide](CONFIGURATION_GUIDE.md).

---

# 10. Piping and scripting

## 10.1 Stdin

```bash
kubectl logs deploy/myapp | logscope analyze -
zgrep ERROR /var/log/syslog* | logscope investigate --level error -
```

## 10.2 JSON output for jq

```bash
logscope analyze --format json samples/sample.log | jq '.sections.errors'
logscope investigate --format json --filter 'level == ERROR' samples/sample.log
```

## 10.3 Exit codes

| Code | Meaning |
|------|---------|
| `0` | Success |
| Non-zero | Invalid arguments, I/O failure, or analysis error |

Use in shell scripts: `logscope analyze ... || exit 1`

---

# 11. Troubleshooting

| Problem | What to try |
|---------|-------------|
| Wrong format detected | `--log-format plain` or `--log-format jsonl` |
| JSON field names differ | `--profile generic-json` plus `source.json.timestamp_field` / `source.json.level_field` in config |
| No matches in investigate | Confirm level spelling (`error`, not `ERROR` for `--level`; use `level == ERROR` in DSL) |
| Index cap hit | Raise `investigation.max_indexed_lines` in config, or use `--persist-index` |
| Slow persist on large logs | Use `--index-path` to reuse index on later runs; v1.4.2+ batches writes for faster first-time persist |
| Windows build SSL error | `CMAKE_TLS_VERIFY=0` at configure only — see [Developer Setup](DEVELOPER_SETUP.md#windows-sqlite-fetchcontent-tls) |
| Config rejected | `logscope config validate --config <file>` for the exact error |

---

# 12. Related documents

| Document | Use when |
|----------|----------|
| [CLI Reference](CLI_REFERENCE.md) | Exact syntax for every flag and subcommand |
| [Configuration Guide](CONFIGURATION_GUIDE.md) | Properties files and environment variables |
| [M10 – Query Language](../planning/M10-QUERY-LANGUAGE.md) | Filter DSL grammar details |
| [M11 – Storage Layer](../planning/M11-STORAGE-LAYER.md) | Persistent index design |
| [Plugin Development Guide](PLUGIN_DEVELOPMENT_GUIDE.md) | Built-in extensions and report hooks |
| [Developer Setup](DEVELOPER_SETUP.md) | Building and contributing |

---

# 13. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 24-07-2026 | Initial Phase 1 user manual. |
| 1.1.0 | 24-07-2026 | v1.4.2 persist-index performance, progress logging, and Windows build link fix. |
| 1.2.0 | 24-07-2026 | Complete CLI workflow coverage: CSV/Markdown, session thresholds, extensions, investigate flags. |
