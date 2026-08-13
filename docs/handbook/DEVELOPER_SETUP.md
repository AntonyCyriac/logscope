# Developer Setup

| Field | Value |
|-------|-------|
| Document | Developer Setup |
| Category | Handbook |
| Version | 2.17.0 |
| Last Updated | 13-08-2026 |

---

# 1. Purpose

This document describes how to set up a development environment for LogScope.

A developer should be able to:

- Clone the repository.
- Configure the development environment.
- Build and run LogScope.
- Debug the application.
- Understand the recommended development workflow.

This document focuses on environment setup. For contributing workflow and testing expectations, see [Developer Guide](DEVELOPER_GUIDE.md). Architecture guidance is in the architecture documentation.

**Current release:** [`v2.10.0`](../../CHANGELOG.md) — Stories 1–6 + P1 + P1.1 TID pstack dialects (CLI, desktop, web). **Next:** P2 desktop Timeline/Crash parity. See [CHANGELOG](../../CHANGELOG.md) for release history.

**Planning:** [Next Value-Add Backlog](../planning/NEXT-VALUE-ADD.md) · Plugin development: [Plugin Development Guide](PLUGIN_DEVELOPMENT_GUIDE.md)

---

# 2. Supported Development Environment

The current reference development environment is:

| Item | Value |
|------|-------|
| Operating System | Windows 11 |
| Shell | MSYS2 UCRT64 |
| Compiler | GCC 16.x |
| Debugger | GDB 17.x |
| Build System | CMake 4.x |
| Version Control | Git |
| IDE | Cursor |

Other platforms may be supported in the future, but all development should remain cross-platform whenever practical.

---

# 3. Required Software

| Tool | Version |
|------|---------|
| GCC | 16.1.0 |
| GDB | 17.2 |
| CMake | 4.4.0 |
| Git | 2.55.x |
| Cursor | Latest |

---

# 4. Install MSYS2

Install MSYS2 and use the **UCRT64** environment.

Install the required packages:

```bash
pacman -Syu

pacman -S \
mingw-w64-ucrt-x86_64-gcc \
mingw-w64-ucrt-x86_64-gdb \
mingw-w64-ucrt-x86_64-cmake \
mingw-w64-ucrt-x86_64-clang-tools-extra
```

---

# 5. Environment Variables

Add the following directory to the Windows User PATH:

```text
C:\msys64\ucrt64\bin
```

Restart Cursor after updating the PATH.

---

# 6. Verify Installation

Verify the installation using:

```bash
g++ --version
gcc --version
gdb --version
cmake --version
git --version
clang-format --version
```

All commands should execute successfully before proceeding.

---

# 7. Cursor Configuration

Install the following extensions:

- C/C++
- CMake
- CMake Tools
- GitLens
- Error Lens
- EditorConfig
- Clang Format (xaver)
- Markdown All in One

---

# 8. Repository Configuration

The repository includes project configuration files.

```text
.clang-format
.clang-tidy
.editorconfig
.gitignore
CMakePresets.json
.vscode/
```

These files define the standard development environment and should normally not require modification.

---

# 9. Build

Configure the project:

```bash
cmake -S . -B build
```

Build the project:

```bash
cmake --build build
```

### Windows: SQLite FetchContent TLS

M11 downloads the SQLite amalgamation during **CMake configure**. On some Windows machines, Schannel may fail certificate revocation checks (`CRYPT_E_REVOCATION_OFFLINE`) when contacting `sqlite.org`. This affects **building from source only** — release binaries do not need this workaround.

PowerShell (configure step only):

```powershell
$env:CMAKE_TLS_VERIFY = "0"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

MSVC Release binary path: `build\apps\cli\Release\logscope.exe`. For `--persist-index` on Windows, see [User Manual §8](USER_MANUAL.md#8-large-logs-and-persistent-indexes).

### Desktop application (optional, M14)

Requires Qt6 Widgets (`Qt6::Widgets`). Enable with `-DLOGSCOPE_DESKTOP=ON` (default **OFF** so CI and minimal builds skip Qt).

| Platform | Qt install | Configure | Binary path |
|----------|------------|-----------|-------------|
| **Linux** | `qt6-base-dev` | `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLOGSCOPE_DESKTOP=ON` | `build/apps/desktop/logscope-desktop` |
| **Windows (MSVC)** | Qt 6 MSVC kit or [install-qt-action](https://github.com/jurplel/install-qt-action) locally | Same + `CMAKE_TLS_VERIFY=0` if SQLite FetchContent fails TLS | `build\apps\desktop\Release\logscope-desktop.exe` |
| **Windows (MinGW)** | `pacman -S mingw-w64-ucrt-x86_64-qt6-base` | Same as Linux | `build/apps/desktop/logscope-desktop.exe` |
| **macOS** | `brew install qt@6` | `cmake … -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6)"` | `build/apps/desktop/logscope-desktop.app` (run binary inside `Contents/MacOS/`) |

```bash
# Linux / MinGW (from repo root)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLOGSCOPE_DESKTOP=ON
cmake --build build --target logscope-desktop
./build/apps/desktop/logscope-desktop --config samples/logscope.properties
```

```bash
# macOS
brew install qt@6
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLOGSCOPE_DESKTOP=ON \
  -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6)"
cmake --build build --target logscope-desktop
open build/apps/desktop/logscope-desktop.app
```

```powershell
# Windows MSVC (configure TLS workaround if needed)
$env:CMAKE_TLS_VERIFY = "0"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLOGSCOPE_DESKTOP=ON
cmake --build build --config Release --target logscope-desktop
.\build\apps\desktop\Release\logscope-desktop.exe --config samples\logscope.properties
```

Release archives bundle Qt on Windows (`windeployqt`) and macOS (`macdeployqt` on `.app`). Linux tarball ships the binary; install system Qt6 Widgets or build locally.

See [apps/desktop/README.md](../../apps/desktop/README.md) (Ollama/`--config`) and [M14 planning](../planning/M14-DESKTOP-APPLICATION.md).

### CMake options (build flavors)

| Option | Default | Purpose |
|--------|---------|---------|
| `LOGSCOPE_DESKTOP` | OFF | Build `logscope-desktop` (Qt6 Widgets) |
| `LOGSCOPE_BENCHMARKS` | OFF | Performance benchmark target |
| `LOGSCOPE_FUZZING` | OFF | libFuzzer targets (Clang only) |
| `LOGSCOPE_BUILD_SAMPLE_PLUGINS` | ON | Example dynamic plugins |
| `LOGSCOPE_DOCS` | OFF | Doxygen API docs (`cmake --build build --target docs`) |
| `LOGSCOPE_SANITIZE` | OFF | ASan/UBSan (`-DLOGSCOPE_SANITIZE=ON`, Debug) |

Common configure flags: `-DCMAKE_BUILD_TYPE=Release|Debug`, `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` (clang-tidy), `-DCMAKE_PREFIX_PATH=…` (Qt on macOS/custom installs). Windows MinGW uses system `ZLIB` for `scope_storage`; other platforms use vendored zlib via FetchContent.

---

# 9.1 API documentation (optional)

Generate the Doxygen HTML reference when [Doxygen](https://www.doxygen.org/) is installed:

```bash
cmake -S . -B build -DLOGSCOPE_DOCS=ON
cmake --build build --target docs
```

Open `build/docs/api/html/index.html`. See [API Documentation](../api/README.md) for CI integration and input scope.

### Third-party licenses

Vendored FetchContent dependencies and SPDX identifiers are documented in [Third-Party Licenses](THIRD_PARTY_LICENSES.md). Run the license scan after configure:

```bash
python3 scripts/check_third_party_licenses.py --deps-dir build/_deps --require-optional
```

---

# 10. Run

Run LogScope:

```bash
./build/apps/cli/logscope.exe samples/sample.log
./build/apps/cli/logscope.exe analyze --format json samples/sample.log
./build/apps/cli/logscope.exe --config samples/logscope.properties samples/sample.log
./build/apps/cli/logscope.exe config validate --config samples/logscope.properties --require log.level
```

Adjust the executable path if the build configuration changes.

### Try AI locally

Sample configs live under `samples/`. See [samples/README.md](../../samples/README.md).

```bash
# Offline — no network (CI-safe)
./build/apps/cli/logscope.exe agent investigate --config samples/ai-noop.properties --summarize samples/sample.log

# Ollama — requires Ollama running; any non-empty LOGSCOPE_AI_API_KEY
export LOGSCOPE_AI_API_KEY=ollama
./build/apps/cli/logscope.exe agent investigate --config samples/ai-ollama.properties --summarize samples/sample.log
```

```powershell
$env:LOGSCOPE_AI_API_KEY = "ollama"
.\build\apps\cli\logscope.exe agent investigate --config samples\ai-ollama.properties --summarize samples\sample.log
```

---

# 11. Debug

Press **F5** in Cursor to:

- Build the project
- Launch LogScope
- Attach GDB
- Stop at configured breakpoints

---

# 12. Coding Style

The project follows a consistent coding style.

Configuration is provided by:

- `.clang-format`
- `.editorconfig`

Guidelines:

- Format on Save should remain enabled.
- Do not manually reformat code.
- Keep commits focused and self-contained.
- Follow the project's engineering principles.

---

# 13. Git Configuration

Recommended Git configuration:

```bash
git config --global core.autocrlf true
```

Commit messages follow the Conventional Commits specification.

Examples:

- feat:
- fix:
- docs:
- refactor:
- test:
- build:
- ci:
- chore:

---

# 14. Development Workflow

LogScope follows an architecture-first engineering workflow.

```text
Engineering Principles
        ↓
Product Vision
        ↓
Requirements
        ↓
Architecture
        ↓
Implementation
        ↓
Testing
```

Implementation should always follow approved architecture.

---

# 15. Architecture References

Before implementing a new component, review the relevant architecture documentation.

Recommended reading order:

1. Architecture Overview
2. Architecture Principles
3. Component Catalog
4. Domain Model
5. Data Flow
6. HLD-001 – Logical Architecture

Developers should understand the target architecture before modifying implementation.

---

# 16. Component-Based Development

Implementation should align with the approved architecture.

Each implementation task should identify:

- Component ID (Cxx)
- Functional Requirement (FR)
- Non-Functional Requirement (NFR)

Example:

```text
Component:
C03 – Analysis Engine

Implements:
FR-001 – Analyze Logs

Constrained By:
NFR-001 – Quality Attributes
```

This maintains traceability between requirements, architecture, and implementation.

---

# 17. Contribution Workflow

After environment setup, follow the handbook workflow guides:

| Document | Purpose |
|----------|---------|
| [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) | Contributing workflow, testing, and PR expectations |
| [GIT_CONVENTIONS.md](GIT_CONVENTIONS.md) | Commit and branch naming |
| [PULL_REQUEST_GUIDE.md](PULL_REQUEST_GUIDE.md) | Author checklist and definition of done |
| [CODE_REVIEW_CHECKLIST.md](CODE_REVIEW_CHECKLIST.md) | Reviewer checklist |

---

# 18. Verification Checklist

Before starting development:

- [ ] Development tools installed.
- [ ] Project configures successfully.
- [ ] Project builds successfully.
- [ ] Debugging works.
- [ ] Format on Save works.
- [ ] Git working tree is clean.
- [ ] Architecture documents reviewed.

For benchmarks, fuzz tests, sanitizers, coverage, and the bulk-log CLI matrix, see [Testing Guide](../testing/TESTING.md).

---

# 19. Revision History

| Version | Date | Description |
|----------|------------|-----------------------------|
| 2.0.0 | 15-07-2026 | Updated to align with the completed engineering design baseline and architecture-driven development workflow. |
| 2.1.0 | 18-07-2026 | Added contribution workflow section linking to handbook guides. |
| 2.2.0 | 18-07-2026 | Linked M5 testing guide from verification checklist. |
| 2.3.0 | 24-07-2026 | Added optional Doxygen API documentation build (`LOGSCOPE_DOCS`). |
| 2.4.0 | 24-07-2026 | Current release baseline (`v1.4.2`, 396 tests). |
| 2.6.0 | 25-07-2026 | Current release baseline (`v1.5.0`, 462 tests). |
| 2.7.0 | 25-07-2026 | Current release baseline (`v1.5.1`, 513 tests); M13 AI Assistant. |
| 2.8.0 | 30-07-2026 | Current release baseline (`v1.5.2`, 520 tests); Phase 1 stabilization. |
| 2.9.0 | 30-07-2026 | Current release baseline (`v2.0.0`, 524 tests); M14 desktop build (`LOGSCOPE_DESKTOP`). |
| 2.10.0 | 30-07-2026 | Current release baseline (`v2.0.1`); per-platform desktop build table and CMake options. |
| 2.11.0 | 30-07-2026 | Current release baseline (`v2.0.2`); M14.12 desktop CLI parity polish. |
| 2.12.0 | 30-07-2026 | Current release baseline (`v2.0.3`); desktop regression hotfix + `logscope_desktop_tests`. |
| 2.13.0 | 30-07-2026 | Current release baseline (`v2.0.5`); CI/build hotfix, versioned release artifacts. |
| 2.15.0 | 05-08-2026 | Current release baseline (`v2.3.0`); Story 1 investigations; next `v2.4.0`. |
| 2.16.0 | 06-08-2026 | Current release baseline (`v2.6.1`); Story 5 Connect the Evidence active. |
| 2.19.0 | 13-08-2026 | Current release baseline (`v2.10.0`); P1.1 TID pstack dialects (#144). |
