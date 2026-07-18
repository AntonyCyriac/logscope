# LogScope



[![CI](https://github.com/AntonyCyriac/logscope/actions/workflows/ci.yml/badge.svg)](https://github.com/AntonyCyriac/logscope/actions/workflows/ci.yml)

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)



A generic, extensible, high-performance log analysis platform.



LogScope is an open-source framework for parsing, normalizing, and analyzing logs from any system or format. The goal is a modular platform that helps engineers investigate system behavior through a consistent workflow.



**Status:** M3 complete ([`v0.3.0`](CHANGELOG.md)); **M4 feature expansion in progress** — see [M4 plan](docs/planning/M4-FEATURE-EXPANSION.md) and [Roadmap](docs/ROADMAP.md).



## What works today



- **Foundation library** (`scope_foundation`) — `Status`, `Error`, `Result<T>`, `Version`, `Uuid`, `Time`, `Date`, `DateTime`, `Duration`, `Timestamp`, `Clock`, `Stopwatch`, `Random`, `String`, `Hash`, `Path`, `FileSystem`

- **Runtime library** (`scope_runtime`) — `Configuration`, `Diagnostics`, `PluginRegistry`, `ServiceRegistry`

- **Configuration library** (`scope_configuration`) — `ConfigurationManager` for file and environment config

- **Source library** (`scope_source`) — `SourceManager`, `FileLogSource`, and `SourceDataset`

- **Analysis library** (`scope_analysis`) — `AnalysisEngine` and `AnalysisModel`

- **Investigation library** (`scope_investigation`) — `InvestigationEngine`, `InvestigationView`, and `LineCountFilter`

- **Reporting library** (`scope_reporting`) — `ReportGenerator` and `Report`

- **CLI application** (`apps/cli`) — subcommands (`analyze`, `config validate`), `--format text|json`, optional `--config` flag

- **CI** — build, unit tests, integration tests, and CLI end-to-end tests on every push to `master`

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



## Project layout



```text

apps/           Applications (CLI)

core/           Core libraries (foundation, runtime, configuration, source, analysis, investigation, reporting)

docs/           Product, architecture, standards, and handbook

samples/        Sample log files and configuration

tests/          Integration and end-to-end tests

```



## Documentation



| Topic | Location |

|-------|----------|

| Roadmap and milestones | [docs/ROADMAP.md](docs/ROADMAP.md) |

| Changelog | [CHANGELOG.md](CHANGELOG.md) |

| Developer setup | [docs/handbook/DEVELOPER_SETUP.md](docs/handbook/DEVELOPER_SETUP.md) |

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


