# Developer Guide

| Field | Value |
|-------|-------|
| Document | Developer Guide |
| Category | Handbook |
| Version | 1.0.0 |
| Status | Approved |
| Created | 24-07-2026 |
| Last Updated | 24-07-2026 |

---

# 1. Purpose

This guide describes how to contribute features, fixes, and documentation to LogScope.

It complements [Developer Setup](DEVELOPER_SETUP.md), which covers environment installation and local build. This document focuses on **repository structure**, **engineering workflow**, **testing expectations**, and **how to land changes**.

Phase 1 stabilization deliverable — see [Post-v1 Strategic Roadmap](../planning/POST_V1_STRATEGIC_ROADMAP.md#phase-1--stabilize-v1x).

---

# 2. Before you start

| Step | Document |
|------|----------|
| Environment ready | [Developer Setup](DEVELOPER_SETUP.md) |
| Engineering mindset | [Project Context](PROJECT_CONTEXT.md) |
| Coding rules | [C++ Coding Standard](../standards/CPP_CODING_STANDARD.md), [Foundation Guidelines](../standards/FOUNDATION_GUIDELINES.md) |
| Architecture context | [Architecture Overview](../architecture/ARCHITECTURE_OVERVIEW.md), [Component Catalog](../architecture/COMPONENT_CATALOG.md) |
| Git workflow | [Git Conventions](GIT_CONVENTIONS.md), [Pull Request Guide](PULL_REQUEST_GUIDE.md) |

---

# 3. Repository map

```text
apps/cli/           CLI executable and subcommands
core/               Libraries (one CMake target per module, scope_<name>)
  foundation/       Errors, Result<T>, paths, time, strings
  runtime/          Configuration, diagnostics
  configuration/    Properties file loading
  source/           Log readers (file, directory, stdin)
  analysis/         AnalysisEngine, line index, format detection
  search/           Text and boolean search
  query/            Filter DSL
  storage/          SQLite indexes (M11)
  analytics/        Frequency, clustering, timeline
  investigation/    Investigation engine
  reporting/        Report formats and sections
  extension/        ExtensionManager
  workspace/        Sessions
docs/               Product, architecture, handbook, planning
samples/            Sample logs and logscope.properties
scripts/            Bulk-log generation and CLI matrix
tests/
  integration/      Multi-module pipeline tests
  end_to_end/       CLI executable tests
  regression/       Known-issue regression guards
  benchmarks/       Optional performance baselines
  fuzz/             Optional libFuzzer targets (Clang)
```

**Rule:** CLI-first until v2.0 — business logic lives in `core/`; `apps/cli` is a thin command layer.

---

# 4. Architecture-first workflow

```text
Requirement (FR/NFR) or milestone plan (Mn)
        │
        ▼
ADR (when required) + planning doc
        │
        ▼
Module implementation in core/
        │
        ▼
CLI integration + configuration keys
        │
        ▼
Tests (unit → integration → e2e → regression)
        │
        ▼
Handbook / CHANGELOG / DOCUMENT_MAP
```

### ADR gates

Require an Architecture Decision Record before coding when the strategic roadmap or component catalog calls for it:

- SQLite storage (ADR-005 — shipped)
- Query DSL grammar (ADR-004 — shipped)
- Dynamic plugins (M12 — planned)
- Qt GUI, AI integration (future)

ADR templates live in `docs/architecture/decisions/`.

### Traceability

When implementing, identify:

```text
Component:  C03 – Analysis Engine
Implements: FR-001 – Analyze Logs
Constrained: NFR-001 – Quality Attributes
```

---

# 5. Adding a feature

Typical milestone delivery (M6–M11 pattern):

| Step | Action |
|------|--------|
| 1 | Planning doc `docs/planning/Mn-*.md` and ADR if needed |
| 2 | New or extended `core/<module>/` with `scope_<module>` CMake target |
| 3 | Unit tests in `core/<module>/tests/` |
| 4 | Wire into pipeline (`AnalysisEngine`, `InvestigationEngine`, etc.) |
| 5 | CLI flags and `config validate` rules |
| 6 | Integration and e2e tests |
| 7 | Update `CLI_REFERENCE.md`, `CONFIGURATION_GUIDE.md`, `USER_MANUAL.md` as needed |
| 8 | `CHANGELOG.md` entry under `[Unreleased]` |

### Built-in extensions

For discoverable capabilities and report hooks, see [Plugin Development Guide](PLUGIN_DEVELOPMENT_GUIDE.md).

### CMake conventions

- C++17 minimum (`CMAKE_CXX_STANDARD 17`)
- One static library per module: `add_library(scope_<name> ...)`
- Tests: `scope_<name>_tests` with `gtest_discover_tests`
- Optional: `LOGSCOPE_BENCHMARKS`, `LOGSCOPE_FUZZING`, `LOGSCOPE_DOCS`, `LOGSCOPE_SANITIZE`

---

# 6. Code conventions (summary)

| Topic | Convention |
|-------|------------|
| Errors | `foundation::Result<T>` for operational failures; avoid exceptions for expected paths |
| Types | Strong types (`Path`, `Timestamp`, …) in `scope_foundation` |
| Headers | Public API in `.hpp`; optional `.inl` for templates |
| Comments | Doxygen on public APIs ([C++ Coding Standard](../standards/CPP_CODING_STANDARD.md)) |
| Formatting | `clang-format` via `.clang-format`; `cmake --build build --target format` |
| Static analysis | `cmake --build build --target tidy` (clang-tidy, CI optional) |

---

# 7. Testing

See [Testing Guide](../testing/TESTING.md) for commands and CI mapping.

| Layer | When to add | Location |
|-------|-------------|----------|
| Unit | Every new public API or edge case | `core/*/tests/` |
| Integration | Cross-module pipeline behaviour | `tests/integration/` |
| End-to-end | CLI commands and exit codes | `tests/end_to_end/` |
| Regression | Fixed bugs that must not return | `tests/regression/` |
| Benchmark | Performance-sensitive paths | `tests/benchmarks/` (`LOGSCOPE_BENCHMARKS=ON`) |
| Fuzz | Parsers and serializers | `tests/fuzz/` (`LOGSCOPE_FUZZING=ON`, Clang) |

### Local verification

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

### Sanitizer build (Linux/macOS, GCC/Clang)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DLOGSCOPE_SANITIZE=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

CI runs the sanitizer job on Ubuntu for every push and PR to `master`.

### CLI matrix (bulk logs)

```bash
python3 scripts/generate_bulk_log.py --lines 10000 --format plain --output /tmp/bulk.log
python3 scripts/generate_bulk_log.py --lines 10000 --format jsonl --output /tmp/bulk.jsonl
python3 scripts/run_cli_matrix.py --logscope build/apps/cli/logscope \
  --plain-log /tmp/bulk.log --jsonl-log /tmp/bulk.jsonl
```

---

# 8. Documentation expectations

| Change type | Update |
|-------------|--------|
| New CLI flag | `CLI_REFERENCE.md`, often `USER_MANUAL.md` |
| New config key | `CONFIGURATION_GUIDE.md`, `samples/logscope.properties` |
| New extension | `PLUGIN_DEVELOPMENT_GUIDE.md` |
| Public C++ API | Doxygen comments; rebuild `docs` target |
| Milestone complete | Planning doc status, `ROADMAP.md`, `CHANGELOG.md`, `PROJECT_CONTEXT.md` |
| New handbook doc | `DOCUMENT_MAP.md` reading order |

---

# 9. Pull request workflow

1. Branch from `master` — `feature/...`, `fix/...`, `docs/...` per [Git Conventions](GIT_CONVENTIONS.md).
2. Keep commits focused; use Conventional Commits.
3. Complete the [Pull Request Guide](PULL_REQUEST_GUIDE.md) checklist.
4. Ensure CI passes: build matrix (Windows/Linux/macOS), CLI matrix, sanitizers, coverage, benchmarks, docs.
5. Request review; reviewers use [Code Review Checklist](CODE_REVIEW_CHECKLIST.md).

Release tagging and strategy sync: [Release process](../release/RELEASE.md).

---

# 10. CI quality gates

| Job | Purpose |
|-----|---------|
| Build and Test | Release build, full `ctest` on three OSes |
| CLI Matrix | 10k-line bulk log, 18+ CLI scenarios |
| Sanitizer | ASan + UBSan (+ LeakSanitizer on Linux) |
| Coverage | lcov report artifact |
| Benchmark | Regression check against `baseline.json` |
| Docs | Doxygen HTML artifact |
| Fuzz | libFuzzer smoke (Clang) |
| tidy | clang-tidy (`continue-on-error`) |

---

# 11. Related documents

| Document | Purpose |
|----------|---------|
| [Developer Setup](DEVELOPER_SETUP.md) | Install, build, debug |
| [Plugin Development Guide](PLUGIN_DEVELOPMENT_GUIDE.md) | Extensions and report hooks |
| [Testing Guide](../testing/TESTING.md) | Test layers and scripts |
| [Workspace Model](../implementation/WORKSPACE_MODEL.md) | Repository layout detail |
| [Document Map](../DOCUMENT_MAP.md) | Full documentation index |

---

# 12. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 24-07-2026 | Initial Phase 1 developer guide. |
