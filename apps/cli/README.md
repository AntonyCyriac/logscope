# LogScope CLI

## Purpose

The CLI provides the command-line interface for LogScope.

It wires together configuration, source acquisition, analysis, investigation, and reporting.

## Commands

| Command | Description |
|---------|-------------|
| `analyze` | Analyze a log file |
| `config validate` | Validate configuration files and required keys |
| `help` | Show command help |

Legacy invocation is supported:

```bash
logscope [--config <file>] <log-file>
```

## Examples

```bash
logscope analyze samples/sample.log
logscope analyze --format json samples/sample.log
logscope --config samples/logscope.properties samples/sample.log
logscope config validate --config samples/logscope.properties --require log.level
```

## Output Formats

| Format | Description |
|--------|-------------|
| `text` | Human-readable report (default) |
| `json` | Structured JSON output |

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
