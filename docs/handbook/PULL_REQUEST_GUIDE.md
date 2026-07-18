# Pull Request Guide

| Field | Value |
|-------|-------|
| Document | Pull Request Guide |
| Category | Handbook |
| Version | 1.0.0 |
| Status | Approved |
| Created | 18-07-2026 |
| Last Updated | 18-07-2026 |

---

# 1. Purpose

This document defines the author's responsibilities when opening a pull request and the criteria for considering a feature complete.

Reviewers should use [Code Review Checklist](CODE_REVIEW_CHECKLIST.md). Commit and branch conventions are defined in [Git Conventions](GIT_CONVENTIONS.md).

---

# 2. Pull Request Checklist

Before requesting review, the author shall verify the following.

## Implementation

- [ ] Feature implemented
- [ ] Architecture unchanged unless intentional
- [ ] Public API reviewed
- [ ] No dead code
- [ ] No TODOs left behind

---

## Testing

- [ ] Unit tests added
- [ ] All tests pass
- [ ] Manual testing completed (if applicable)

---

## Documentation

- [ ] Doxygen updated
- [ ] CHANGELOG updated (if applicable)
- [ ] Engineering documentation updated (if applicable)

---

## Quality

- [ ] Code formatted
- [ ] Static analysis clean
- [ ] Compiler warnings fixed
- [ ] No duplicated code

---

## Git

- [ ] Commit messages follow convention
- [ ] History cleaned up (squash/rebase if required)
- [ ] Branch builds successfully

---

# 3. Definition of Done

A feature is considered complete only when all of the following are satisfied.

## Functional

- [ ] Requirements implemented
- [ ] Acceptance criteria satisfied
- [ ] Expected behavior verified

---

## Engineering

- [ ] Code reviewed
- [ ] Tests written
- [ ] Tests passing
- [ ] Documentation updated
- [ ] Formatting completed
- [ ] No compiler warnings

---

## Architecture

- [ ] Follows Foundation guidelines
- [ ] No unnecessary dependencies
- [ ] API reviewed
- [ ] Performance acceptable

---

## Repository

- [ ] Builds successfully
- [ ] CI passes
- [ ] Ready for merge

---

# 4. Related Documents

- [Git Conventions](GIT_CONVENTIONS.md)
- [Code Review Checklist](CODE_REVIEW_CHECKLIST.md)
- [Foundation Guidelines](../standards/FOUNDATION_GUIDELINES.md)
- [Engineering Principles](../standards/ENGINEERING_PRINCIPLES.md)

---

# 5. Revision History

| Version | Date | Description |
|----------|------------|-----------------------------|
| 1.0.0 | 18-07-2026 | Extracted from Foundation Guidelines. |
