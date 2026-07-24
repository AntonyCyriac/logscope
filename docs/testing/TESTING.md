# Testing Guide

| Field | Value |
|-------|-------|
| Document | Testing Guide |
| Category | Testing |
| Version | 1.3.0 |
| Status | Approved |
| Created | 18-07-2026 |
| Last Updated | 24-07-2026 |

---

# Purpose

This document describes LogScope test layers, how to run them, and how they map to release quality gates (M5 production readiness through ongoing milestone delivery).

**Current baseline:** **337** automated tests at **`v1.3.1`** (unit, integration, and end-to-end). Reporting and analytics coverage includes chart renderers, analytics analyzers, and CLI/e2e cases for `logscope analytics` and analytics report sections.

---

# Test Layers

| Layer | Target | Location |
|-------|--------|----------|
| Unit | Individual modules | `core/*/tests/`, `apps/cli/tests/` |
| Integration | Core pipeline | `tests/integration/` |
| End-to-end | CLI executable | `tests/end_to_end/` |
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

---

# Integration and End-to-End Tests

```bash
cmake --build build --target logscope_integration_tests logscope_e2e_tests
./build/tests/integration/logscope_integration_tests
./build/tests/end_to_end/logscope_e2e_tests
```

Integration tests run with the repository root as the working directory.

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

CI runs this matrix on Ubuntu with 10,000-line fixtures. Release builds use 100,000-line fixtures on every OS before publishing binaries.

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
cmake --build build --target session_serializer_fuzz configuration_fuzz
./build/tests/fuzz/session_serializer_fuzz -runs=10000
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

Requires `clang-tidy` on PATH.

---

# Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 18-07-2026 | Initial testing guide. |
| 1.2.0 | 24-07-2026 | Updated baseline to 337 tests at `v1.3.1`; noted M9 analytics coverage. |
| 1.3.0 | 24-07-2026 | Added bulk-log CLI matrix scripts, CI job, and release workflow integration. |
