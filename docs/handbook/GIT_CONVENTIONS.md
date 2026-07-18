# Git Conventions

| Field | Value |
|-------|-------|
| Document | Git Conventions |
| Category | Handbook |
| Version | 1.0.0 |
| Status | Approved |
| Created | 18-07-2026 |
| Last Updated | 18-07-2026 |

---

# 1. Purpose

This document defines commit message and branch naming conventions for the LogScope repository.

Incremental delivery aligns with [EP-011 – Incremental Evolution](../standards/ENGINEERING_PRINCIPLES.md#ep-011--incremental-evolution).

---

# 2. Commit Message Convention

Follow the [Conventional Commits](https://www.conventionalcommits.org/) specification.

Examples:

```text
feat(uuid): add UUID value object

fix(version): handle invalid semantic version

refactor(result): simplify error propagation

docs(foundation): update coding guidelines

test(uuid): add comparison tests

build(cmake): add foundation library

perf(parser): reduce allocations

chore(ci): update GitHub workflow
```

---

# 3. Branch Naming Convention

Use descriptive branch names.

```text
feature/uuid-core

feature/uuid-parser

feature/version

bugfix/parser-overflow

refactor/error-system

docs/foundation-guidelines

test/version

release/v1.0.0
```

---

# 4. Related Documents

- [Pull Request Guide](PULL_REQUEST_GUIDE.md)
- [Developer Setup](DEVELOPER_SETUP.md)

---

# 5. Revision History

| Version | Date | Description |
|----------|------------|-----------------------------|
| 1.0.0 | 18-07-2026 | Extracted from Foundation Guidelines. |
