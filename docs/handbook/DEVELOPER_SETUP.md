# Developer Setup

| Field | Value |
|-------|-------|
| Document | Developer Setup |
| Category | Handbook |
| Version | 2.1.0 |
| Status | Approved |
| Created | 15-07-2026 |
| Last Updated | 18-07-2026 |

---

# 1. Purpose

This document describes how to set up a development environment for LogScope.

A developer should be able to:

- Clone the repository.
- Configure the development environment.
- Build and run LogScope.
- Debug the application.
- Understand the recommended development workflow.

This document focuses on environment setup. Architecture and implementation guidance are provided by the architecture documentation.

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

---

# 10. Run

Run LogScope:

```bash
./build/logscope.exe samples/sample.log
```

Adjust the executable path if the build configuration changes.

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

---

# 19. Revision History

| Version | Date | Description |
|----------|------------|-----------------------------|
| 2.0.0 | 15-07-2026 | Updated to align with the completed engineering design baseline and architecture-driven development workflow. |
| 2.1.0 | 18-07-2026 | Added contribution workflow section linking to handbook guides. |
