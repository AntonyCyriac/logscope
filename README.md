# LogScope

[![CI](https://github.com/AntonyCyriac/logscope/actions/workflows/ci.yml/badge.svg)](https://github.com/AntonyCyriac/logscope/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

Analyze any log format through one CLI workflow — parse, investigate, report, and persist indexes without custom scripts.

**Status:** [`v2.1.0`](CHANGELOG.md) — M15 Web Platform (`logscope-web` + browser MVP). See [Roadmap](docs/ROADMAP.md) and [Changelog](CHANGELOG.md).

---

## Overview

LogScope is an open-source, modular log analysis platform. It helps engineers understand system behavior from plain-text or JSONL logs using a consistent pipeline:

```text
Configuration → Source → Analysis → Investigation → Reporting → CLI
```

Component diagram (Mermaid): [Component Catalog §4](docs/architecture/COMPONENT_CATALOG.md#component-structure-diagram).

| Audience | Start here |
|----------|------------|
| **Users** | [User Manual](docs/handbook/USER_MANUAL.md) · [CLI Reference](docs/handbook/CLI_REFERENCE.md) · [Releases](https://github.com/AntonyCyriac/logscope/releases) |
| **Contributors** | [Developer Setup](docs/handbook/DEVELOPER_SETUP.md) · [Tutorials](docs/tutorials/README.md) · [Developer Guide](docs/handbook/DEVELOPER_GUIDE.md) · [Testing](docs/testing/TESTING.md) |
| **Architecture** | [Component diagram](docs/architecture/COMPONENT_CATALOG.md#component-structure-diagram) · [Architecture Overview](docs/architecture/ARCHITECTURE_OVERVIEW.md) · [Component Catalog](docs/architecture/COMPONENT_CATALOG.md) · [Product](docs/PRODUCT.md) |

Full documentation index: [Document Map](docs/DOCUMENT_MAP.md).

---

## Quick start

**Install** — download pre-built binaries for your OS from [GitHub Releases](https://github.com/AntonyCyriac/logscope/releases) (`v2.0.1`+ includes `logscope-desktop` on Linux, Windows, and macOS; `v2.0.2` adds export section picker, index toggles, and stats dialog), or build from source below.

```bash
git clone https://github.com/AntonyCyriac/logscope.git
cd logscope
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure

./build/apps/cli/logscope analyze samples/sample.log
```

More examples (formats, config, investigation, sessions): [User Manual §2](docs/handbook/USER_MANUAL.md#2-getting-started).

### Web platform (v2.1.0+)

```bash
cmake -S . -B build -DLOGSCOPE_WEB=ON
cmake --build build --target logscope-web
./build/apps/web/logscope-web --config samples/web.properties
# Open http://127.0.0.1:8080/ (or https when TLS is configured)
```

**Bind / TLS** (also `web.bind_host`, `web.bind_port`, `web.tls_cert`, `web.tls_key` in properties):

| Flag / env | Purpose |
|------------|---------|
| `--bind-host` / `LOGSCOPE_WEB_BIND_HOST` | Listen address (default `127.0.0.1`) |
| `--bind-port` / `LOGSCOPE_WEB_BIND_PORT` | Listen port (default `8080`) |
| `--tls-cert` / `LOGSCOPE_WEB_TLS_CERT` | HTTPS certificate (PEM; requires OpenSSL build) |
| `--tls-key` / `LOGSCOPE_WEB_TLS_KEY` | HTTPS private key (PEM) |

CORS origins default from bind host/port (http + https when TLS enabled). Override with `web.cors_origins`.

REST API: [ADR-009](docs/architecture/decisions/ADR-009-Web-Platform-REST.md). Optional API key via `web.api_key` or `LOGSCOPE_WEB_API_KEY`.

---

## Build from source

| Requirement | Version |
|-------------|---------|
| CMake | 3.20+ |
| Compiler | C++17 (GCC, Clang, or MSVC) |
| Git | any recent |

GoogleTest and SQLite (amalgamation) are fetched automatically by CMake.

| Topic | Document |
|-------|----------|
| Windows / MSYS2, debugging, formatting | [Developer Setup](docs/handbook/DEVELOPER_SETUP.md) |
| Properties and `storage.*` keys | [Configuration Guide](docs/handbook/CONFIGURATION_GUIDE.md) |
| `--persist-index`, large logs | [User Manual §8](docs/handbook/USER_MANUAL.md#8-large-logs-and-persistent-indexes) |

---

## Repository layout

```text
apps/       CLI, desktop (optional), web (optional)
core/       Libraries (foundation, source, analysis, investigation, reporting, …)
docs/       Product, architecture, handbook, planning
samples/    Example logs and configuration (see [samples/README.md](samples/README.md))
scripts/    Bulk-log fixtures and CLI matrix runners
tests/      Unit, integration, e2e, regression, benchmarks
```

---

## Contributing

1. [Developer Setup](docs/handbook/DEVELOPER_SETUP.md) → [Developer Guide](docs/handbook/DEVELOPER_GUIDE.md)
2. [Git Conventions](docs/handbook/GIT_CONVENTIONS.md) · [Pull Request Guide](docs/handbook/PULL_REQUEST_GUIDE.md)
3. Build, test, and format before opening a PR (`cmake --build build --target format`)

---

## License

[MIT](LICENSE) — Copyright (c) 2026 AntonyCyriac

