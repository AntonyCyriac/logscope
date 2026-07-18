# LogScope CLI

## Purpose

The CLI provides the command-line interface for LogScope.

It wires together configuration, source acquisition, analysis, investigation, and reporting.

## Commands

| Command | Description |
|---------|-------------|
| `analyze` | Analyze a log file, directory of `.log` files, or stdin (`-`) |
| `config validate` | Validate configuration files and required keys |
| `extensions list` | List registered extensions and their status |
| `extensions describe` | Show metadata for a single extension |
| `help` | Show command help |

Legacy invocation is supported:

```bash
logscope [--config <file>] <log-source>
```

## Examples

```bash
logscope analyze samples/sample.log
logscope analyze --format csv --sections summary,levels samples/sample.log
logscope analyze --format markdown samples
logscope extensions list
logscope extensions describe analysis.log-levels
logscope analyze -
logscope analyze --format json samples/sample.log
logscope --config samples/logscope.properties samples/sample.log
logscope config validate --config samples/logscope.properties --require log.level
```

## Output Formats

| Format | Description |
|--------|-------------|
| `text` | Human-readable report with sections (default) |
| `json` | Structured JSON with section objects |
| `csv` | Section/key/value rows for spreadsheets |
| `markdown` | Markdown tables for documentation |

## Components

| Component | Description |
|-----------|-------------|
| `CliApplication` | Top-level command dispatcher |
| `CliParser` | Argument parsing |
| `AnalyzeCommand` | Analyze workflow |
| `ConfigValidateCommand` | Configuration validation |
| `LogAnalyzer` | End-to-end analysis pipeline |

## Tests

- `logscope_cli_tests` — parser and output formatting unit tests
