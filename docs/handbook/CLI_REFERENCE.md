# CLI Reference

| Field | Value |
|-------|-------|
| Document | CLI Reference |
| Category | Handbook |
| Version | 1.1.0 |
| Status | Approved |
| Created | 18-07-2026 |
| Last Updated | 21-07-2026 |

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
| `--format text\|json\|csv\|markdown` | Output format (default: text) |
| `--log-format auto\|plain\|jsonl` | Input log format hint (default: auto) |
| `--profile <name>` | Built-in format profile: `generic-plain`, `generic-json` |
| `--sections <list>` | Report sections: `summary`, `levels`, `source`, or `all` |
| `--min-errors <n>` | Investigation filter: minimum error lines |
| `--min-warnings <n>` | Investigation filter: minimum warning lines |
| `--min-lines <n>` | Investigation filter: minimum total lines |
| `--max-lines <n>` | Investigation filter: maximum total lines |
| `--search <query>` | Investigation search query |

**Log sources:** file path, directory of `.log` files, or `-` for stdin.

```bash
logscope analyze samples/sample.log
logscope analyze --format json samples/sample.log
logscope analyze --log-format plain samples/sample.log
logscope analyze --profile generic-json samples/sample.jsonl
logscope analyze - < samples/sample.log
logscope analyze ./logs/
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
| `--search <query>` | Search indexed log line content |
| `--time-from <timestamp>` | Earliest timestamp (ISO-like) |
| `--time-to <timestamp>` | Latest timestamp (ISO-like) |
| `--level <name>` | Filter by line level: `error`, `warning`, `info`, `other` |
| `--message <text>` | Filter by message/content substring |
| `--json-key <key>` | Require a JSON top-level key on matching lines |

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
logscope session load <session-file>
```

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
