# Git Conventions

| Field | Value |
|-------|-------|
| Document | Git Conventions |
| Category | Handbook |
| Version | 1.1.0 |
| Status | Approved |
| Created | 18-07-2026 |
| Last Updated | 24-07-2026 |

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

# 4. Release Tags

Public releases use annotated semantic tags: `vX.Y.Z` (for example `v1.2.0`, `v1.3.0`).

These tags are the **sync points** for related private strategy materials maintained outside this repository. Private strategy uses matching tags of the form `sync/vX.Y.Z` after each public release so long-horizon plans stay aligned with shipped code. This repository does not publish or link those private tags.

Tag a public release only from `master` after the release PR is merged.

---

# 5. Related Documents

- [Pull Request Guide](PULL_REQUEST_GUIDE.md)
- [Developer Setup](DEVELOPER_SETUP.md)
- [Release process](../release/RELEASE.md)

---

# 6. Revision History

| Version | Date | Description |
|----------|------------|-----------------------------|
| 1.0.0 | 18-07-2026 | Extracted from Foundation Guidelines. |
| 1.1.0 | 24-07-2026 | Release tags as sync points with private strategy (`sync/vX.Y.Z`). |
