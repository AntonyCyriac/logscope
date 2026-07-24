# GitHub Configuration

Documentation for the `.github/` directory. This file is intentionally not named `README.md` so it does not override the repository homepage.

## Purpose

This directory contains GitHub-specific configuration, automation, and workflows used to maintain the LogScope project.

These files support the project's engineering processes and help ensure consistent quality across all contributions.

---

## Default Branch

The default branch for the repository is:

```text
master
```

All pull requests should target the `master` branch.

---

## Build Status Badge

The CI badge is included in the root [README.md](../README.md):

```markdown
![CI](https://github.com/AntonyCyriac/logscope/actions/workflows/ci.yml/badge.svg)
```

The badge reflects the status of the default branch (`master`).

---

## Directory Structure

```text
.github/
│
├── GITHUB.md
├── workflows/
│   ├── ci.yml
│   └── release.yml
│
└── PULL_REQUEST_TEMPLATE.md
```

---

## Workflows

The `workflows/` directory contains GitHub Actions pipelines.

### `ci.yml` — Continuous Integration

| Job | Purpose |
|-----|---------|
| `build` | Configure, build, and test on Ubuntu, Windows, and macOS |
| `cli-matrix` | Ubuntu bulk-log CLI matrix (10k-line fixtures, 18 command scenarios) |
| `sanitize` | AddressSanitizer build and test on Ubuntu |
| `coverage` | `gcov`/`lcov` coverage capture and artifact upload |
| `benchmark` | Benchmark regression check against committed baseline |
| `fuzz` | libFuzzer smoke tests (Clang, Ubuntu) |
| `tidy` | clang-tidy static analysis (`continue-on-error`) |

Runs on every push and pull request to `master`.

### `release.yml` — Release Binaries

Triggered by tags matching `v*`.

1. Build and test on Ubuntu, Windows, and macOS
2. Run bulk-log CLI matrix (100k-line fixtures per OS)
3. Package per-OS archives with `samples/`
4. Publish GitHub Release assets

---

## Pull Request Template

The pull request template helps contributors provide consistent information when submitting changes.

The template should describe:

- Purpose of the change
- Related issue(s)
- Testing performed
- Documentation updates
- Additional notes

---

## Engineering Philosophy

GitHub automation should remain:

- Simple
- Incremental
- Reliable
- Easy to understand
- Easy to maintain

Automation should evolve alongside the project and support the engineering workflow without introducing unnecessary complexity.
