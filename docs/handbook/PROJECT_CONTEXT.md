# LogScope Project Context

| Field | Value |
|-------|-------|
| Document | LogScope Project Context |
| Category | Handbook |
| Version | 1.0.0 |
| Status | Approved |
| Created | 21-07-2026 |
| Last Updated | 21-07-2026 |

---

> **Usage:** Attach or reference this file at the start of a new Cursor session so the assistant begins with the same engineering mindset and project constraints established for LogScope.

---

# Overview

You are assisting in the development of **LogScope**, a professional-grade, open-source log analysis framework written in modern C++.

This is **not a demo project**. It is intended to become a production-quality software product built with strong software engineering principles similar to LLVM, Qt, Chromium, Envoy, and other mature open-source projects.

The project follows architecture-first development. Every implementation should respect the existing architecture and engineering standards.

---

# Current Project Status

**Current release:** `v1.0.0`

**Completed milestones:**

| Milestone | Description |
|-----------|-------------|
| M0 – Engineering Foundation | Repository setup, build system, coding standards, CI foundation |
| M1 – Product Vision | Project Charter, Product Overview, product goals |
| M2 – Engineering Design | Standards, requirements, architecture, high-level design |
| M3 – Architecture Realization | Core pipeline: source, analysis, investigation, reporting, CLI |
| M4 – Feature Expansion | Analysis depth, sources, reporting, extensions, sessions |
| M5 – Production Readiness | Benchmarks, fuzz, multi-OS CI, packaging, v1.0.0 release |

**Planned:** M6 – Log Format Intelligence (target `v1.1.0`). See [M6 planning](../planning/M6-LOG-FORMAT-INTELLIGENCE.md) and [Roadmap](../ROADMAP.md).

The engineering foundation is complete. Future work should **extend** the product rather than redesign the infrastructure.

---

# Engineering Philosophy

The project emphasizes:

- Clean Architecture
- SOLID Principles
- Value Semantics
- Rule of Zero
- RAII
- Strong Type Safety
- Explicit APIs
- Low Coupling
- High Cohesion
- Modern C++ (C++20 preferred where appropriate)
- Readable, maintainable code
- Minimal dependencies
- Cross-platform compatibility
- Long-term maintainability

Performance is important, but correctness, maintainability, and extensibility take priority.

See also: [Engineering Principles](../standards/ENGINEERING_PRINCIPLES.md), [Architecture Principles](../architecture/ARCHITECTURE_PRINCIPLES.md), [Foundation Guidelines](../standards/FOUNDATION_GUIDELINES.md).

---

# Coding Standards

Always prefer:

- `std::string_view`
- `constexpr`
- `noexcept`
- `[[nodiscard]]`
- explicit constructors
- const correctness
- value objects
- small focused classes
- composition over inheritance
- standard library before third-party libraries

Avoid:

- unnecessary heap allocation
- macros
- global mutable state
- overly clever code
- unnecessary templates
- friend classes unless truly required

See also: [C++ Coding Standard](../standards/CPP_CODING_STANDARD.md).

---

# Repository Workflow

Development is GitHub-first. The repository follows a professional workflow.

**Rules:**

- `master` is protected
- no direct commits to `master`
- no force pushes to `master`
- every change goes through a pull request

**Flow:**

```text
Feature Branch → Pull Request → CI → Review → Squash Merge → master
```

See also: [Pull Request Guide](PULL_REQUEST_GUIDE.md), [Git Conventions](GIT_CONVENTIONS.md), [Developer Setup](DEVELOPER_SETUP.md).

---

# Branch Naming

```text
feature/<feature>
bugfix/<bug>
docs/<documentation>
refactor/<component>
test/<component>
chore/<task>
```

Milestone work may use phased names from planning documents (e.g. `feat/m6.1-format-detection`). See [Git Conventions](GIT_CONVENTIONS.md).

---

# Commit Style

[Conventional Commits](https://www.conventionalcommits.org/).

Examples:

```text
feat(core): add UUID parser
fix(storage): resolve cache corruption
refactor(parser): simplify tokenizer
docs: update architecture guide
test(core): add UUID unit tests
chore(ci): update GitHub workflow
```

---

# Pull Requests

Every implementation should include:

- Scope
- Files modified
- Unit tests
- Review checklist
- Suggested commit message
- Suggested PR title

See: [Pull Request Guide](PULL_REQUEST_GUIDE.md), [Code Review Checklist](CODE_REVIEW_CHECKLIST.md).

---

# Continuous Integration

CI is configured and must remain green.

Includes:

- Multi-OS build verification (Ubuntu, Windows, macOS)
- Unit, integration, and end-to-end tests
- Static analysis (clang-tidy)
- Code coverage
- Benchmark regression checks
- Fuzz smoke tests (where applicable)

Do not introduce changes that weaken CI quality.

---

# Documentation Structure

```text
docs/
├── architecture/
├── handbook/          ← you are here
├── implementation/
├── planning/
├── release/
├── requirements/
├── standards/
├── testing/
└── vision/
```

New documentation should fit within this structure. See [Document Map](../DOCUMENT_MAP.md).

---

# Architecture Principles

Follow:

- Layered Architecture
- Dependency Inversion
- Stable APIs
- Explicit dependencies
- No cyclic dependencies
- Small reusable components
- Clear module boundaries

Core pipeline (v1.0.0):

```text
ConfigurationManager → SourceManager → AnalysisEngine → AnalysisModel
                                              ↓
              InvestigationEngine / ReportGenerator / ExtensionManager
                                              ↓
                         SessionStore → CLI
```

See: [Architecture Overview](../architecture/ARCHITECTURE_OVERVIEW.md), [Component Catalog](../architecture/COMPONENT_CATALOG.md), [Data Flow](../architecture/DATA_FLOW.md).

---

# Implementation Expectations

Whenever implementing a feature, provide:

- Header (`.hpp`)
- Source (`.cpp`)
- Unit tests
- Integration updates where the pipeline is affected
- Doxygen comments where appropriate

Avoid producing unnecessary design markdown unless explicitly requested.

---

# Code Review Mindset

Before considering code complete, verify:

- readable
- testable
- exception safe
- thread safe where applicable
- follows coding standards
- follows architecture
- minimal public API
- no dead code
- no duplication

---

# Response Expectations

When helping with implementation:

- Do not provide quick hacks.
- Think like a senior software engineer working on a long-lived production project.
- Prefer maintainability over cleverness.
- If a better architecture exists, explain it before implementing.
- When multiple options exist: explain trade-offs, recommend one, justify the decision.

---

# Development Style

Every feature should be treated like production code. Implementations should feel suitable for inclusion in mature open-source projects.

Quality is more important than speed. Never sacrifice architecture for convenience.

---

# Project Goal

The long-term goal is to make LogScope a professional, extensible, high-performance log analysis platform capable of handling multiple log formats through a plugin architecture, with strong tooling, reporting, and diagnostics.

The project should be maintainable for many years and welcoming to future contributors through consistent engineering practices.

**Product promise:** *Analyze any log format without writing custom scripts.*

See: [Product Overview](../vision/PRODUCT_OVERVIEW.md), [Project Charter](../vision/PROJECT_CHARTER.md).

---

# Key References

| Document | Purpose |
|----------|---------|
| [ROADMAP.md](../ROADMAP.md) | Milestones and current phase |
| [CHANGELOG.md](../../CHANGELOG.md) | Release history |
| [CLI Reference](CLI_REFERENCE.md) | CLI commands and flags |
| [TESTING.md](../testing/TESTING.md) | Test layout and CI jobs |
| [RELEASE.md](../release/RELEASE.md) | Release process |

---

# Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 21-07-2026 | Initial project context for Cursor and contributor onboarding. |
