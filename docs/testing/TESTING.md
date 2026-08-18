# Testing Guide

| Field | Value |
|-------|-------|
| Document | Testing Guide |
| Category | Testing |
| Version | 1.19.0 |
| Status | Approved |
| Created | 18-07-2026 |
| Last Updated | 13-08-2026 |

---

# Purpose

This document describes LogScope test layers, how to run them, and how they map to release quality gates (M5 production readiness through ongoing milestone delivery).

**Current baseline:** **536+** automated tests through **`v2.12.0`** (includes `logscope_desktop_parity_test` P2 + P2.1 Story Gate). Coverage includes `scope_application_tests` (M14 + `InvestigationControllerTest` link/suggestion APIs), `logscope_desktop_tests` (M14 GUI headless), `logscope_desktop_parity_test` (P2/P2.1 desktop Story Gate), web integration tests (M15), Playwright Story Gate E2E (Stories 1–6 + Crash Timeline), `scope_workspace_tests` (`crash.summary` timeline cases), `scope_ai_tests` (M13), `scope_plugin_tests` (M12), `scope_storage_tests`, persist-index/session-reuse e2e cases, CLI matrix scenarios (including `agent investigate`), AI/plugin regression guards, `query_filter_fuzz`, desktop CI smoke (`LOGSCOPE_DESKTOP`), and CI `license-scan`.

---

# Test Layers

| Layer | Target | Location |
|-------|--------|----------|
| Unit | Individual modules | `core/*/tests/`, `apps/cli/tests/`, `apps/common/tests/` |
| Integration | Core pipeline | `tests/integration/` |
| End-to-end | CLI executable | `tests/end_to_end/` |
| Regression | Fixed-bug guards | `tests/regression/` |
| Benchmark | Performance baselines | `tests/benchmarks/` (optional) |
| Fuzz | Parser hardening | `tests/fuzz/` (optional, Clang) |

---

# Running All Tests

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

---

# Unit Tests

Each core module has a `scope_<module>_tests` target:

```bash
cmake --build build --target scope_foundation_tests
./build/core/foundation/tests/scope_foundation_tests
```

CLI parser tests:

```bash
cmake --build build --target logscope_cli_tests
./build/apps/cli/tests/logscope_cli_tests
```

Application service tests (M14):

```bash
cmake --build build --target scope_application_tests
ctest --test-dir build --output-on-failure -L scope_application_tests
```

Desktop build smoke (Ubuntu CI; requires Qt6):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLOGSCOPE_DESKTOP=ON
cmake --build build --target logscope-desktop scope_application_tests logscope_desktop_tests
ctest --test-dir build --output-on-failure -L logscope_desktop_tests
```

Headless desktop GUI tests use `QT_QPA_PLATFORM=offscreen` (set automatically by CTest). Covers open/analyze table population and noop AI Ask (`errors` → 4 matches) — guards v2.0.2 desktop table/Ask regression (fixed in `v2.0.3`).

---

# Playwright Web E2E

Browser automation for the web SPA Story Gate paths (investigation, timeline, crash, investigate, AI):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLOGSCOPE_WEB=ON
cmake --build build --target logscope-web

cd tests/e2e/web
npm ci
npx playwright install chromium
npm test
```

The test harness starts `logscope-web` with `samples/demo-story-gate.properties` and `LOGSCOPE_WEB_UI_DIR=apps/web/ui/dist`. See [`tests/e2e/web/README.md`](../../tests/e2e/web/README.md).

CI runs Playwright in the `web` job on Ubuntu after web integration tests.

---

# Integration and End-to-End Tests

```bash
cmake --build build --target logscope_integration_tests logscope_e2e_tests
./build/tests/integration/logscope_integration_tests
./build/tests/end_to_end/logscope_e2e_tests
```

Integration tests run with the repository root as the working directory.

---

# Regression Tests

Guards for fixed bugs that must not return (Phase 1 `tests/regression/`):

- AI summarize isolation (`ai_regression_test.cpp`)
- Plugin bad-path isolation (`plugin_regression_test.cpp`)
- Storage incremental-append flake fix (Windows CI)

```bash
cmake --build build --target logscope_regression_tests
ctest --test-dir build --output-on-failure -R regression
```

See [`tests/regression/README.md`](../../tests/regression/README.md).

---

# CLI Matrix

Cross-platform Python scripts exercise many CLI command combinations against generated bulk logs:

```bash
python3 scripts/generate_bulk_log.py --lines 10000 --format plain --output /tmp/bulk.log
python3 scripts/generate_bulk_log.py --lines 10000 --format jsonl --output /tmp/bulk.jsonl
python3 scripts/run_cli_matrix.py \
  --logscope build/apps/cli/logscope \
  --plain-log /tmp/bulk.log \
  --jsonl-log /tmp/bulk.jsonl
```

CI runs this matrix on Ubuntu with 10,000-line fixtures. Release builds use 100,000-line fixtures on every OS before publishing binaries (pass `--lines 100000` locally to match release).

---

# Malformed-Input Tests

Cross-platform reliability tests (no libFuzzer required):

- `session_serializer_malformed_test.cpp`
- `configuration_malformed_test.cpp`
- `cli_parser_malformed_test.cpp`
- Negative cases in `pipeline_integration_test.cpp`

---

# Benchmarks

See [`PERFORMANCE.md`](PERFORMANCE.md).

```bash
cmake -S . -B build -DLOGSCOPE_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --target logscope_benchmarks
./build/tests/benchmarks/logscope_benchmarks
```

---

# Fuzz Testing

Requires Clang:

```bash
cmake -S . -B build -DLOGSCOPE_FUZZING=ON -DCMAKE_CXX_COMPILER=clang++
cmake --build build --target session_serializer_fuzz configuration_fuzz query_filter_fuzz
./build/tests/fuzz/session_serializer_fuzz -runs=10000
./build/tests/fuzz/query_filter_fuzz -runs=10000
```

---

# Sanitizer Builds

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DLOGSCOPE_SANITIZE=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Supported on Clang and GCC.

---

# Coverage

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage -fprofile-update=atomic" -DCMAKE_EXE_LINKER_FLAGS="--coverage"
cmake --build build
ctest --test-dir build
lcov --capture --directory build --output-file coverage.info \
  --rc geninfo_unexecuted_blocks=1 \
  --ignore-errors mismatch,inconsistent,gcov,negative,unused \
  --quiet
lcov --remove coverage.info '/usr/*' '*/_deps/*' '*/tests/*' --output-file coverage.info \
  --ignore-errors unused \
  --quiet
```

CI generates and uploads `coverage.info` as an artifact.

---

# Static Analysis

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --target tidy
```

Requires `clang-tidy` on PATH. Checks and `WarningsAsErrors` are defined in `.clang-tidy` (`clang-analyzer-*`, `bugprone-unused-result` for `[[nodiscard]]`).

---

# Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 18-07-2026 | Initial testing guide. |
| 1.2.0 | 24-07-2026 | Updated baseline to 337 tests at `v1.3.1`; noted M9 analytics coverage. |
| 1.3.0 | 24-07-2026 | Added bulk-log CLI matrix scripts, CI job, and release workflow integration. |
| 1.4.0 | 24-07-2026 | Updated baseline to 365 tests at `v1.4.0`; noted M10 query language coverage. |
| 1.5.0 | 24-07-2026 | Added regression test layer; updated baseline to 395 tests; sanitizer CI env hardening. |
| 1.6.0 | 24-07-2026 | Updated baseline to 396 tests at `v1.4.2`; noted M11 bulk index write performance coverage. |
| 1.7.0 | 24-07-2026 | v1.4.3 scenario test matrix; target ~435 tests at release. |
| 1.9.0 | 25-07-2026 | v1.5.0 release baseline (462 tests); M12 plugin test coverage. |
| 1.10.0 | 25-07-2026 | v1.5.1 release baseline (513 tests); M13 AI Assistant test coverage. |
| 1.11.0 | 30-07-2026 | v1.5.2 release baseline (520 tests); regression expansion, fuzz, license-scan CI. |
| 1.12.0 | 30-07-2026 | v2.0.0 release baseline (524 tests); M14 application layer and tailing source tests; desktop CI smoke. |
| 1.13.0 | 30-07-2026 | Current release baseline `v2.0.1`; all-platform desktop release workflow. |
| 1.14.0 | 30-07-2026 | Current release baseline `v2.0.2`; M14.12 desktop CLI parity polish. |
| 1.15.0 | 30-07-2026 | Current release baseline `v2.0.3`; `logscope_desktop_tests` (529 tests). |
| 1.16.0 | 30-07-2026 | Current release baseline `v2.0.5`; directory e2e isolation, versioned release artifacts. |
| 1.17.0 | 04-08-2026 | Baseline through `v2.2.1`; M15 web integration tests noted. |
| 1.18.0 | 06-08-2026 | Playwright web E2E (Story Gate); CI `web` job runs browser tests. |
| 1.24.0 | 18-08-2026 | Baseline through `v2.12.0`; P2.1 Story 5/6 desktop Story Gate. |
