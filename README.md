# LogScope

[![CI](https://github.com/AntonyCyriac/logscope/actions/workflows/ci.yml/badge.svg)](https://github.com/AntonyCyriac/logscope/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

A generic, extensible, high-performance log analysis platform.

LogScope is an open-source framework for parsing, normalizing, and analyzing logs from any system or format. The goal is a modular platform that helps engineers investigate system behavior through a consistent workflow.

**Status:** v1.4.2 — M11 storage performance ([`CHANGELOG.md`](CHANGELOG.md)). M0–M11 complete — see [Roadmap](docs/ROADMAP.md).

## What works today

- **Foundation library** (`scope_foundation`) — `Status`, `Error`, `Result<T>`, `Version`, `Uuid`, `Time`, `Date`, `DateTime`, `Duration`, `Timestamp`, `Clock`, `Stopwatch`, `Random`, `String`, `Hash`, `Path`, `FileSystem`
- **Runtime library** (`scope_runtime`) — `Configuration`, `Diagnostics`, `PluginRegistry`, `ServiceRegistry`
- **Configuration library** (`scope_configuration`) — `ConfigurationManager` for file and environment config
- **Source library** (`scope_source`) — `SourceManager`, `FileLogSource`, and `SourceDataset`
- **Analysis library** (`scope_analysis`) — `AnalysisEngine` and `AnalysisModel`
- **Investigation library** (`scope_investigation`) — `InvestigationEngine`, `InvestigationView`, `LineCountFilter`, and `LogLevelFilter`
- **Reporting library** (`scope_reporting`) — `ReportGenerator`, section registry, text, JSON, CSV, Markdown, HTML, and PDF output; analytics sections
- **Analytics library** (`scope_analytics`) — frequency, clustering, timeline, trend, and correlation analysis
- **Query library** (`scope_query`) — field-aware filter DSL parser and evaluator
- **Extension library** (`scope_extension`) — `ExtensionManager` with configuration-based enablement
- **Workspace library** (`scope_workspace`) — `InvestigationSession` and `SessionStore` for `.logscope-session` persistence
- **CLI application** (`apps/cli`) — `analyze`, `analytics`, `query`, `config validate`, `extensions`, and `session` subcommands; `--filter` DSL on investigate/search; stdin and directory input; `--format text|json|csv|markdown|html|pdf`; `--output <file>`
- **CI** — multi-OS build, unit/integration/e2e tests, and Ubuntu CLI matrix (10k-line bulk logs, 22 command scenarios) on every push to `master`
- **Documentation** — requirements, architecture, standards, and developer handbook

## Requirements

- CMake 3.20 or later
- C++17 compiler (GCC, Clang, or MSVC)
- Git

GoogleTest is fetched automatically by CMake.

## Quick start

```bash
git clone https://github.com/AntonyCyriac/logscope.git
cd logscope

cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the CLI:

```bash
./build/apps/cli/logscope samples/sample.log
./build/apps/cli/logscope analyze --format json samples/sample.log
./build/apps/cli/logscope --config samples/logscope.properties samples/sample.log
./build/apps/cli/logscope config validate --config samples/logscope.properties --require log.level
```

Format source files (requires `clang-format`):

```bash
cmake --build build --target format
```

## Building on Windows

M11 downloads the SQLite amalgamation during **CMake configure** (`FetchContent`). On some Windows machines, Schannel may fail certificate revocation checks (`CRYPT_E_REVOCATION_OFFLINE`) when contacting `sqlite.org`. This affects **building from source only** — release binaries and normal CLI use do not need this workaround.

Set `CMAKE_TLS_VERIFY=0` for the configure step (PowerShell):

```powershell
git clone https://github.com/AntonyCyriac/logscope.git
cd logscope

$env:CMAKE_TLS_VERIFY = "0"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Run the CLI (Release output path on MSVC):

```powershell
.\build\apps\cli\Release\logscope.exe analyze samples\sample.log
```

### Persistent indexes on Windows

Use `--persist-index` for large logs or `--index-path` for an explicit SQLite file. Auto-generated indexes default to `%LOCALAPPDATA%\logscope\indexes\`. See [Configuration Guide](docs/handbook/CONFIGURATION_GUIDE.md) and [CLI Reference](docs/handbook/CLI_REFERENCE.md) for `storage.*` keys and CLI flags.

```powershell
.\build\apps\cli\Release\logscope.exe investigate `
  --persist-index `
  --index-path C:\logs\app.index.db `
  --filter "level == ERROR" `
  C:\logs\app.log
```

## Project layout

```text
apps/           Applications (CLI)
core/           Core libraries (foundation, runtime, configuration, source, analysis, investigation, reporting, extension, workspace)
docs/           Product, architecture, standards, and handbook
samples/        Sample log files and configuration
scripts/        Bulk-log generation and CLI matrix runners
tests/          Integration and end-to-end tests
```

## Documentation

| Topic | Location |
|-------|----------|
| Roadmap and milestones | [docs/ROADMAP.md](docs/ROADMAP.md) |
| M5 production readiness | [docs/planning/M5-PRODUCTION-READINESS.md](docs/planning/M5-PRODUCTION-READINESS.md) |
| Testing guide | [docs/testing/TESTING.md](docs/testing/TESTING.md) |
| CLI reference | [docs/handbook/CLI_REFERENCE.md](docs/handbook/CLI_REFERENCE.md) |
| User manual | [docs/handbook/USER_MANUAL.md](docs/handbook/USER_MANUAL.md) |
| Configuration guide | [docs/handbook/CONFIGURATION_GUIDE.md](docs/handbook/CONFIGURATION_GUIDE.md) |
| Plugin development guide | [docs/handbook/PLUGIN_DEVELOPMENT_GUIDE.md](docs/handbook/PLUGIN_DEVELOPMENT_GUIDE.md) |
| Release process | [docs/release/RELEASE.md](docs/release/RELEASE.md) |
| Changelog | [CHANGELOG.md](CHANGELOG.md) |
| Developer setup | [docs/handbook/DEVELOPER_SETUP.md](docs/handbook/DEVELOPER_SETUP.md) |
| Developer guide | [docs/handbook/DEVELOPER_GUIDE.md](docs/handbook/DEVELOPER_GUIDE.md) |
| Engineering principles | [docs/standards/ENGINEERING_PRINCIPLES.md](docs/standards/ENGINEERING_PRINCIPLES.md) |
| Foundation coding guidelines | [docs/standards/FOUNDATION_GUIDELINES.md](docs/standards/FOUNDATION_GUIDELINES.md) |
| Architecture overview | [docs/architecture/ARCHITECTURE_OVERVIEW.md](docs/architecture/ARCHITECTURE_OVERVIEW.md) |
| Full documentation index | [docs/DOCUMENT_MAP.md](docs/DOCUMENT_MAP.md) |

## Contributing

1. Read [Developer Setup](docs/handbook/DEVELOPER_SETUP.md) and [Foundation Guidelines](docs/standards/FOUNDATION_GUIDELINES.md).
2. Follow [Git Conventions](docs/handbook/GIT_CONVENTIONS.md) and the [Pull Request Guide](docs/handbook/PULL_REQUEST_GUIDE.md).
3. Ensure the project builds, tests pass, and code is formatted before opening a PR.

## License

[MIT](LICENSE) — Copyright (c) 2026 AntonyCyriac
