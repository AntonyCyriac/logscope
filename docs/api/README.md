# API Documentation

Generated C++ API reference for LogScope public headers.

## Prerequisites

- [Doxygen](https://www.doxygen.org/) 1.9 or later

On Ubuntu:

```bash
sudo apt-get install -y doxygen
```

## Generate HTML

```bash
cmake -S . -B build -DLOGSCOPE_DOCS=ON
cmake --build build --target docs
```

Output: `build/docs/api/html/index.html`

## CI

The [CI workflow](../../.github/workflows/ci.yml) `docs` job builds the site on Ubuntu and uploads `api-docs` as a workflow artifact on every push and pull request to `master`.

**Phase 1 policy (v1.5.2):** the `api-docs` CI artifact is the supported distribution channel. Public GitHub Pages is optional follow-up; consumers download the artifact or run `cmake --build build --target docs` locally.

## Scope

Doxygen scans `core/` and `apps/` headers (`.hpp`, `.h`, `.inl`), excluding test and fuzz directories. Only documented public APIs appear when `EXTRACT_ALL = NO`.

## REST API sketch (OpenAPI)

Human-maintained OpenAPI 3 stub for `logscope-web` routes: [openapi-v1.yaml](openapi-v1.yaml) (M15.1 + M15.3). Not a runtime dependency; update when REST surface changes.

See [Configuration Guide](../handbook/CONFIGURATION_GUIDE.md) and [Developer Setup](../handbook/DEVELOPER_SETUP.md) for broader onboarding.
