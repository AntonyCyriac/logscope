# CLI Reference

| Field | Value |
|-------|-------|
| Document | CLI Reference |
| Category | Handbook |
| Version | 1.0.0 |
| Status | Approved |
| Created | 18-07-2026 |
| Last Updated | 18-07-2026 |

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
logscope analyze - < samples/sample.log
logscope analyze ./logs/
```

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
