# Developer Setup

## Purpose

This document describes how to set up a development environment for LogScope.

The goal is that any developer should be able to clone the repository, follow this guide, and build, run, and debug LogScope without additional assistance.

---

# Supported Platform

Current supported platform:

- Windows 11
- MSYS2 UCRT64
- GCC 16.x
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
| Git | 2.55.0 |
| Cursor | Latest |

---

# Install MSYS2

Install MSYS2 from the official website.

After installation, use the **UCRT64** environment.

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

Restart Cursor after updating the PATH.

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

# Cursor IDE

Install the following extensions:

- C/C++ (Microsoft)
- CMake Tools (Microsoft)
- CMake
- GitLens
- Error Lens
- EditorConfig
- Clang Format (xaver)
- Markdown All in One

---

# Repository Configuration

The repository already contains:

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

Configure:

```bash
cmake -S . -B build
```

Build:

```bash
cmake --build build
```

---

# Debugging

Debugging is configured through Cursor.

Press **F5** to:

- Build LogScope
- Launch the executable
- Pass the sample log file
- Attach GDB

---

# Coding Standards

Formatting is automatic.

- Format on Save is enabled.
- `.clang-format` defines the project style.
- `.editorconfig` defines editor behavior.

Do not manually format source code.

---

# Git

Recommended configuration:

```bash
git config --global core.autocrlf true
```

Commit messages follow the Conventional Commits specification.

Examples:

```text
feat:
fix:
docs:
refactor:
test:
build:
ci:
chore:
```

---

# Verification Checklist

Before starting development:

- [ ] Project configures successfully.
- [ ] Project builds successfully.
- [ ] F5 debugging works.
- [ ] Format on Save works.
- [ ] `git status` is clean.
