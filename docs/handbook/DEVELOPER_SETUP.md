# Developer Setup

## Purpose

This document describes how to set up a development environment for LogScope.

A developer should be able to clone the repository, follow this guide, and build, run, and debug LogScope successfully.

---

# Supported Platform

Current development environment:

- Windows 11
- MSYS2 UCRT64
- GCC 16.x
- GDB 17.x
- CMake 4.x
- Git
- Cursor IDE

---

# Required Software

| Tool | Version |
|------|---------|
| GCC | 16.1.0 |
| GDB | 17.2 |
| CMake | 4.4.0 |
| Git | 2.55.x |
| Cursor | Latest |

---

# Install MSYS2

Install MSYS2 and use the **UCRT64** environment.

Install required packages:

```bash
pacman -Syu

pacman -S \
mingw-w64-ucrt-x86_64-gcc \
mingw-w64-ucrt-x86_64-gdb \
mingw-w64-ucrt-x86_64-cmake \
mingw-w64-ucrt-x86_64-clang-tools-extra
```

---

# Environment Variables

Add the following directory to the Windows User PATH:

```text
C:\msys64\ucrt64\bin
```

Restart Cursor after updating PATH.

---

# Verify Installation

```bash
g++ --version
gcc --version
gdb --version
cmake --version
git --version
clang-format --version
```

---

# Cursor Configuration

Install the following extensions:

- C/C++
- CMake Tools
- CMake
- GitLens
- Error Lens
- EditorConfig
- Clang Format (xaver)
- Markdown All in One

---

# Repository Configuration

The repository contains project configuration files:

- `.clang-format`
- `.editorconfig`
- `.gitignore`
- `CMakePresets.json`
- `.vscode/launch.json`
- `.vscode/tasks.json`
- `.vscode/settings.json`

These files should normally not require modification.

---

# Build

```bash
cmake -S . -B build
cmake --build build
```

---

# Debug

Press **F5** in Cursor to:

- Build the project
- Launch LogScope
- Attach GDB
- Stop at breakpoints

---

# Coding Style

- Format on Save is enabled.
- `.clang-format` defines the project style.
- `.editorconfig` defines editor behavior.

Do not manually format source code.

---

# Git Configuration

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

# Verification Checklist

Before starting development:

- [ ] Project configures successfully.
- [ ] Project builds successfully.
- [ ] F5 debugging works.
- [ ] Format on Save works.
- [ ] Git working tree is clean.
