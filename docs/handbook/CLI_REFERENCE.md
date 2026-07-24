# CLI Reference

| Field | Value |
|-------|-------|
| Document | CLI Reference |
| Category | Handbook |
| Version | 1.6.0 |
| Status | Approved |
| Created | 18-07-2026 |
| Last Updated | 24-07-2026 |

---

# Synopsis

```text
logscope [options] <command> [arguments]
logscope <log-file>                    # legacy analyze
```

---

# Global Options

| Option | Description |
|--------|-------------|
| `--config <file>` | Load configuration from a properties file |
| `-h`, `--help` | Show help |

---

# Commands

## analyze

Analyze a log source and print a report.

```text
logscope analyze [options] <log-source>
```

| Option | Description |
|--------|-------------|
| `--format text\|json\|csv\|markdown\|html\|pdf` | Output format (default: text) |
| `--log-format auto\|plain\|jsonl` | Input log format hint (default: auto) |
| `--profile <name>` | Built-in format profile: `generic-plain`, `generic-json` |
| `--sections <list>` | Report sections: `executive`, `summary`, `levels`, `errors`, `analytics`, `timeline`, `clusters`, `charts`, `metadata`, `formats`, or `all` |
| `--output <file>` | Write report to file (required for PDF; creates parent directories) |
| `--min-errors <n>` | Investigation filter: minimum error lines |
| `--min-warnings <n>` | Investigation filter: minimum warning lines |
| `--min-lines <n>` | Investigation filter: minimum total lines |
| `--max-lines <n>` | Investigation filter: maximum total lines |
| `--search <query>` | Investigation search query |

Storage options (also on `investigate`, `session save`):

| Option | Description |
|--------|-------------|
| `--persist-index` | Persist indexed lines to SQLite |
| `--reuse-index` | Reuse an existing index when the source fingerprint matches |
| `--index-path <file>` | Explicit SQLite index file path |

**Log sources:** file path, directory of `.log` files, or `-` for stdin.

```bash
logscope analyze samples/sample.log
logscope analyze --format json samples/sample.log
logscope analyze --log-format plain samples/sample.log
logscope analyze --profile generic-json samples/sample.jsonl
logscope analyze - < samples/sample.log
logscope analyze --format html --output report.html samples/sample.log
logscope analyze --sections executive,errors,charts samples/sample.log
```

---

## investigate

Run content-aware investigation filters against a log source.

```text
logscope investigate [options] <log-source>
```

| Option | Description |
|--------|-------------|
| `--format text\|json` | Output format (default: text) |
| `--log-format auto\|plain\|jsonl` | Input log format hint (default: auto) |
| `--profile <name>` | Built-in format profile: `generic-plain`, `generic-json` |
| `--search <query>` | Simple text search over indexed log line content |
| `--query <expr>` | Boolean search expression (`AND`, `OR`, `NOT`, quotes) |
| `--filter <dsl>` | Field-aware filter expression (see `query` command) |
| `--regex` | Treat `--search` value as a regular expression |
| `--case-sensitive` | Disable default case-insensitive matching |
| `--time-from <timestamp>` | Earliest timestamp (ISO-like) |
| `--time-to <timestamp>` | Latest timestamp (ISO-like) |
| `--level <name>` | Filter by line level: `error`, `warning`, `info`, `other` |
| `--message <text>` | Filter by message/content substring |
| `--json-key <key>` | Require a JSON top-level key on matching lines |
| `--persist-index` | Persist indexed lines to SQLite |
| `--reuse-index` | Reuse an existing index when the source fingerprint matches |
| `--index-path <file>` | Explicit SQLite index file path |

---

## search

Search indexed log content (alias of `investigate` with text/json output only).

```text
logscope search [options] <log-source>
```

Supports the same search and filter options as `investigate`.

```bash
logscope search --query "error AND timeout" samples/sample.log
logscope search --search "code=\\d+" --regex samples/sample.jsonl
logscope investigate --filter 'level == ERROR' samples/sample.log
```

---

## query

Run field-aware filter DSL queries over a log source.

```text
logscope query [options] --filter <dsl> <log-source>
```

| Option | Description |
|--------|-------------|
| `--format text\|json` | Output format (default: text) |
| `--filter <dsl>` | Field-aware filter expression (required) |
| `--search <query>` | Optional text search over indexed content |
| `--query <expr>` | Optional boolean text search expression |

```bash
logscope query --filter 'level == ERROR' samples/sample.log
logscope query --filter 'contains(message, "timeout")' samples/sample.log
logscope query --filter 'level == ERROR AND contains(message, "refused")' samples/sample.log
```

Filter DSL fields: `level`, `time`, `timestamp`, `message`, `content`, `correlationId`, `line`. Functions: `contains(field, "text")`, `hasKey("key")`.

---

## analytics

Run frequency, clustering, timeline, and correlation analytics over a log source.

```text
logscope analytics [options] <log-source>
```

| Option | Description |
|--------|-------------|
| `--format text\|json` | Output format (default: text) |
| `--log-format auto\|plain\|jsonl` | Input log format hint (default: auto) |
| `--profile <name>` | Built-in format profile: `generic-plain`, `generic-json` |
| `--bucket <seconds>` | Timeline bucket size (default: auto from time span) |
| `--top <n>` | Top frequency/cluster results (default: 10) |

```bash
logscope analytics samples/sample.log
logscope analytics --format json --bucket 60 samples/sample.log
```

---

## Configuration keys

Format-related keys in `logscope.properties` (validated by `config validate`):

| Key | Description |
|-----|-------------|
| `profile` | Built-in profile: `generic-plain`, `generic-json` |
| `source.format` | Format hint: `auto`, `plain`, `jsonl` |
| `source.json.timestamp_field` | JSON timestamp field name override |
| `source.json.level_field` | JSON level field name override |
| `investigation.max_indexed_lines` | Investigation index capacity (default: 10000, max: 1000000) |
| `search.saved.<name>` | Named saved search expression (validated by `config validate`) |
| `query.saved.<name>` | Named saved filter DSL expression (validated by `config validate`) |
| `analytics.bucket_seconds` | Timeline bucket size in seconds (optional; auto when unset) |
| `analytics.top_n` | Top frequency/cluster results (default: 10) |
| `analytics.min_cluster_count` | Minimum occurrences to surface a cluster (default: 2) |
| `report.include_timeline` | Include timeline chart in `charts` section (default: true) |
| `storage.mode` | Index storage mode: `memory`, `hybrid`, `persistent` (default: memory) |
| `storage.index.directory` | Workspace directory for auto-generated index files |
| `storage.index.path` | Explicit SQLite index file path |
| `storage.spill_threshold` | Optional spill threshold override (lines) |

CLI `--profile` and `--log-format` override configuration for a single run.

---

## config validate

Validate a configuration file.

```text
logscope config validate --config <file> [--require <key>...]
```

---

## extensions list

List registered extensions and their enabled state.

```text
logscope extensions list [--config <file>]
```

---

## extensions describe

Show metadata for a single extension.

```text
logscope extensions describe <extension-id> [--config <file>]
```

---

## session save

Run analysis and persist investigation state to a session file.

```text
logscope session save <session-file> <log-source> [analyze options]
```

---

## session load

Load a session file and print a report without re-analyzing.

```text
logscope session load <session-file> [--output <file>]
```

`--output` writes the reproduced report to a file instead of stdout.

---

## session list

List `.logscope-session` files in a directory.

```text
logscope session list [directory]
```

Default directory: current working directory.

---

# Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| Non-zero | Error (invalid arguments, I/O failure, analysis failure) |

---

# Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 18-07-2026 | Initial CLI reference. |
| 1.1.0 | 21-07-2026 | Added `--log-format` for M6.1 format detection. |
| 1.2.0 | 21-07-2026 | Added `--profile`, investigate command, and format configuration keys for M6.5. |
| 1.3.0 | 21-07-2026 | Added `search` command, boolean/regex query flags for M7. |
| 1.5.0 | 24-07-2026 | Added `analytics` command, analytics report sections, and M9 config keys. |
| 1.6.0 | 24-07-2026 | Added `query` command, `--filter` DSL, and `query.saved.*` config keys for M10. |
