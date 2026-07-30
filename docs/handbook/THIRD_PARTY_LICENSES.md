# Third-Party Licenses

| Field | Value |
|-------|-------|
| Document | Third-Party Licenses |
| Category | Handbook |
| Version | 1.0.0 |
| Status | Approved |
| Created | 30-07-2026 |
| Last Updated | 30-07-2026 |

---

# 1. Purpose

This document lists **third-party software** vendored into LogScope builds via CMake `FetchContent`. It supports compliance review and release packaging for **v1.5.2** Phase 1 engineering CI.

The canonical machine-readable manifest is [`third_party/manifest.json`](../../third_party/manifest.json). CI runs [`scripts/check_third_party_licenses.py`](../../scripts/check_third_party_licenses.py) after CMake configure to verify license files are present in `build/_deps/`.

LogScope itself is released under the [MIT License](../../LICENSE).

---

# 2. Runtime dependencies

These libraries are compiled or linked into release binaries when the corresponding features are enabled.

| Component | Version (pinned) | SPDX / license | FetchContent source | Usage |
|-----------|------------------|----------------|---------------------|-------|
| SQLite amalgamation | 3.45.1 (`sqlite-amalgamation-3450100`) | blessing (public domain) | `core/storage/CMakeLists.txt` | Persistent index storage (`scope_storage`) |
| zlib | 1.3.1 (`v1.3.1`) | Zlib | `core/storage/CMakeLists.txt` | Index compression (`zlibstatic`) |
| cpp-httplib | 0.14.3 (`v0.14.3`) | MIT | `core/ai/CMakeLists.txt` | HTTP AI provider (`scope_ai`) |

---

# 3. Build and test dependencies

Fetched during CMake configure for development, CI, and optional benchmark jobs. They are **not** required to run the `logscope` CLI alone, but appear in developer and CI builds.

| Component | Version (pinned) | SPDX / license | FetchContent source | Usage |
|-----------|------------------|----------------|---------------------|-------|
| GoogleTest | 1.17.0 (`v1.17.0`) | BSD-3-Clause | Root `CMakeLists.txt` | Unit, integration, regression, e2e tests |
| Google Benchmark | 1.9.1 (`v1.9.1`) | Apache-2.0 | Root `CMakeLists.txt` when `LOGSCOPE_BENCHMARKS=ON` | Performance benchmarks |

---

# 4. Verification

After configuring the project:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLOGSCOPE_BENCHMARKS=ON
python3 scripts/check_third_party_licenses.py --deps-dir build/_deps --require-optional
```

CI job **Third-Party License Scan (Ubuntu)** runs the same check on every push to `master` and on pull requests.

---

# 5. Adding a new dependency

1. Add `FetchContent_Declare` in the appropriate `CMakeLists.txt`.
2. Add an entry to `third_party/manifest.json` with `fetch_prefix`, SPDX identifier, and `license_files`.
3. Update this document.
4. Ensure `scripts/check_third_party_licenses.py` passes locally after configure.

---

# 6. Traceability

| Source | Relationship |
|--------|--------------|
| [PHASE-1-STABILIZATION.md](../planning/PHASE-1-STABILIZATION.md) §P1.6 | Engineering deliverable |
| [PHASE-1-V152-SCENARIOS.md](../planning/PHASE-1-V152-SCENARIOS.md) E1.1–E1.2 | Acceptance scenarios |
| [POST_V1_STRATEGIC_ROADMAP.md](../planning/POST_V1_STRATEGIC_ROADMAP.md) | Dependency/license scanning gap |
