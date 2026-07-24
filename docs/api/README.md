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

## Scope

Doxygen scans `core/` and `apps/` headers (`.hpp`, `.h`, `.inl`), excluding test and fuzz directories. Only documented public APIs appear when `EXTRACT_ALL = NO`.

See [Configuration Guide](../handbook/CONFIGURATION_GUIDE.md) and [Developer Setup](../handbook/DEVELOPER_SETUP.md) for broader onboarding.
