# Document Map

| Field | Value |
|-------|-------|
| Document | Document Map |
| Category | Documentation |
| Version | 3.16.0 |
| Status | Approved |
| Created | 15-07-2026 |
| Last Updated | 30-07-2026 |

---

# 1. Purpose

This document provides a structured overview of the LogScope documentation.

It defines the organization of documents, their purpose, and the recommended reading order for contributors, reviewers, and maintainers.

The Document Map serves as the primary navigation guide for the documentation repository.

---

# 2. Documentation Philosophy

LogScope documentation is organized into logical layers.

Each document has a single responsibility.

Together, the documents provide complete traceability from project vision through implementation.

```text
Engineering Standards
        â”‚
        â–¼
Product Vision
        â”‚
        â–¼
Requirements
        â”‚
        â–¼
Architecture
        â”‚
        â–¼
Implementation
        â”‚
        â–¼
Testing
        â”‚
        â–¼
Release
```

---

# 3. Documentation Structure

Repository-level documents:

```text
README.md
CHANGELOG.md
scripts/                 # Bulk log generation and CLI matrix runners
â”‚   â”œâ”€â”€ README.md
â”‚   â”œâ”€â”€ generate_bulk_log.py
â”‚   â”œâ”€â”€ run_cli_matrix.py
â”‚   â””â”€â”€ check_benchmark_regression.py
```

`docs/` structure:

```text
docs/

â”œâ”€â”€ DOCUMENT_MAP.md
â”œâ”€â”€ PRODUCT.md
â”œâ”€â”€ ROADMAP.md
â”‚
â”œâ”€â”€ planning/
â”‚   â”œâ”€â”€ POST_V1_STRATEGIC_ROADMAP.md
â”‚   â”œâ”€â”€ M4-FEATURE-EXPANSION.md
â”‚   â”œâ”€â”€ M5-PRODUCTION-READINESS.md
â”‚   â”œâ”€â”€ M6-LOG-FORMAT-INTELLIGENCE.md
â”‚   â”œâ”€â”€ M7-SEARCH-ENGINE.md
â”‚   â”œâ”€â”€ M8-ADVANCED-REPORTING.md
â”‚   â”œâ”€â”€ M9-ANALYTICS-ENGINE.md
â”‚   â”œâ”€â”€ M10-QUERY-LANGUAGE.md
â”‚   â”œâ”€â”€ M11-STORAGE-LAYER.md
â”‚   â”œâ”€â”€ PHASE-1-STABILIZATION.md
â”‚   â”œâ”€â”€ PHASE-1-V152-SCENARIOS.md
â”‚   â”œâ”€â”€ M15-WEB-PLATFORM.md
â”‚   â”œâ”€â”€ M15-V210-WEB-SCENARIOS.md
â”‚   â””â”€â”€ NEXT-VALUE-ADD.md
â”‚
â”‚   (Long-horizon strategy beyond published Mn plans is maintained privately
â”‚    outside this repository; public Mn docs are added when implementation starts.)
â”‚
â”œâ”€â”€ tutorials/
â”‚   â”œâ”€â”€ 01-analyze-logs.md
â”‚   â”œâ”€â”€ 02-investigate-logs.md
â”‚   â””â”€â”€ 03-plugins-and-ai.md
â”‚
â”œâ”€â”€ testing/
â”‚   â”œâ”€â”€ TESTING.md
â”‚   â””â”€â”€ PERFORMANCE.md
â”‚
â”œâ”€â”€ api/
â”‚   â”œâ”€â”€ README.md
â”‚   â”œâ”€â”€ mainpage.md
â”‚   â””â”€â”€ Doxyfile.in          # CMake-generated Doxygen config
â”‚
â”œâ”€â”€ release/
â”‚   â”œâ”€â”€ RELEASE.md
â”‚   â”œâ”€â”€ V1_VALIDATION.md
â”‚   â”œâ”€â”€ v1.5.2-RELEASE-NOTES.md
â”‚   â”œâ”€â”€ v2.0.0-RELEASE-NOTES.md
â”‚   â”œâ”€â”€ v2.0.1-RELEASE-NOTES.md
â”‚   â”œâ”€â”€ v2.0.2-RELEASE-NOTES.md
â”‚   â”œâ”€â”€ v2.0.3-RELEASE-NOTES.md
â”‚   â””â”€â”€ v2.0.4-RELEASE-NOTES.md
â”‚
â”œâ”€â”€ handbook/
â”‚   â”œâ”€â”€ PROJECT_CONTEXT.md
â”‚   â”œâ”€â”€ CODE_REVIEW_CHECKLIST.md
â”‚   â”œâ”€â”€ CONFIGURATION_GUIDE.md
â”‚   â”œâ”€â”€ DEVELOPER_GUIDE.md
â”‚   â”œâ”€â”€ DEVELOPER_SETUP.md
â”‚   â”œâ”€â”€ GIT_CONVENTIONS.md
â”‚   â”œâ”€â”€ PULL_REQUEST_GUIDE.md
â”‚   â”œâ”€â”€ PLUGIN_DEVELOPMENT_GUIDE.md
â”‚   â”œâ”€â”€ SECURITY_REVIEW.md
â”‚   â”œâ”€â”€ THIRD_PARTY_LICENSES.md
â”‚   â”œâ”€â”€ USER_MANUAL.md
â”‚   â”œâ”€â”€ WINDOWS_RELEASE_SIGNING.md
â”‚   â””â”€â”€ MACOS_RELEASE_NOTARIZATION.md
â”‚
â”œâ”€â”€ standards/
â”‚   â”œâ”€â”€ API_DESIGN_GUIDELINES.md
â”‚   â”œâ”€â”€ CPP_CODING_STANDARD.md
â”‚   â”œâ”€â”€ DOCUMENT_STANDARD.md
â”‚   â”œâ”€â”€ ENGINEERING_PRINCIPLES.md
â”‚   â””â”€â”€ FOUNDATION_GUIDELINES.md
â”‚
â”œâ”€â”€ vision/
â”‚   â”œâ”€â”€ PROJECT_CHARTER.md
â”‚   â””â”€â”€ PRODUCT_OVERVIEW.md
â”‚
â”œâ”€â”€ requirements/
â”‚   â”œâ”€â”€ functional/
â”‚   â”‚   â”œâ”€â”€ FR-001-Analyze-Logs.md
â”‚   â”‚   â”œâ”€â”€ FR-002-Investigate-Logs.md
â”‚   â”‚   â”œâ”€â”€ FR-003-Generate-Reports.md
â”‚   â”‚   â””â”€â”€ FR-004-Extend-LogScope.md
â”‚   â”‚
â”‚   â”œâ”€â”€ non_functional/
â”‚   â”‚   â””â”€â”€ NFR-001-Quality-Attributes.md
â”‚   â”‚
â”‚   â””â”€â”€ future/
â”‚       â”œâ”€â”€ README.md
â”‚       â””â”€â”€ PRD-001-AI-Engineering-Agents.md
â”‚
â”œâ”€â”€ architecture/
â”‚   â”œâ”€â”€ ARCHITECTURE_OVERVIEW.md
â”‚   â”œâ”€â”€ ARCHITECTURE_PRINCIPLES.md
â”‚   â”œâ”€â”€ COMPONENT_CATALOG.md
â”‚   â”œâ”€â”€ DOMAIN_MODEL.md
â”‚   â”œâ”€â”€ DATA_FLOW.md
â”‚   â”œâ”€â”€ HLD-001-Logical-Architecture.md
â”‚   â”‚
â”‚   â”œâ”€â”€ decisions/
â”‚   â”‚   â”œâ”€â”€ ADR-001-Testing-Framework.md
â”‚   â”‚   â”œâ”€â”€ ADR-002-Benchmark-Framework.md
â”‚   â”‚   â”œâ”€â”€ ADR-003-PDF-Report-Generation.md
â”‚   â”‚   â”œâ”€â”€ ADR-004-Query-DSL-Grammar.md
â”‚   â”‚   â”œâ”€â”€ ADR-005-Storage-Architecture.md
â”‚   â”‚   â”œâ”€â”€ ADR-006-Plugin-Loading.md
â”‚   â”‚   â”œâ”€â”€ ADR-007-AI-Integration.md
â”‚   â”‚   â”œâ”€â”€ ADR-008-Desktop-Qt-Presentation.md
â”‚   â”‚   â””â”€â”€ ADR-009-Web-Platform-REST.md
â”‚   â”‚
â”‚   â””â”€â”€ foundation/
â”‚       â””â”€â”€ RESULT.md
â”‚
â””â”€â”€ implementation/
    â””â”€â”€ WORKSPACE_MODEL.md
```

---

# 4. Recommended Reading Order

Developers new to LogScope should read the documentation in the following order.

| Step | Document | Purpose |
|------|----------|---------|
| 0 | PROJECT_CONTEXT.md | Agent session bootstrap; engineering mindset and constraints. |
| 1 | ROADMAP.md | Understand the project milestones and current development phase. |
| 2 | POST_V1_STRATEGIC_ROADMAP.md | Understand the long-term post-v1 vision and version targets. |
| 3 | M6-LOG-FORMAT-INTELLIGENCE.md | Understand the completed M6 log format intelligence plan. |
| 4 | M7-SEARCH-ENGINE.md | Understand the completed M7 search engine (`v1.2.0`). |
| 5 | M8-ADVANCED-REPORTING.md | Understand the completed M8 advanced reporting (`v1.3.0`). |
| 6 | M9-ANALYTICS-ENGINE.md | Understand the completed M9 analytics engine (`v1.3.1`). |
| 7 | M10-QUERY-LANGUAGE.md | Understand the completed M10 query language (`v1.4.0`). |
| 8 | M11-STORAGE-LAYER.md | Understand M11 storage (`v1.4.1` core, `v1.4.2` bulk perf, `v1.4.3` remainder). |
| 8a | M11-V143-STORAGE-SCENARIOS.md | v1.4.3 acceptance scenarios and test matrix. |
| 8b | M12-DYNAMIC-PLUGINS.md | Understand M12 dynamic plugins (`v1.5.0`). |
| 8c | M12-V150-PLUGIN-SCENARIOS.md | v1.5.0 plugin acceptance scenarios and test matrix. |
| 8d | M13-AI-ASSISTANT.md | Understand M13 AI Assistant (`v1.5.1`). |
| 8e | M13-V151-AI-SCENARIOS.md | v1.5.1 AI acceptance scenarios and test matrix. |
| 8f | M14-DESKTOP-APPLICATION.md | M14 desktop application plan (`v2.0.0`). |
| 8g | M14-V200-DESKTOP-SCENARIOS.md | v2.0.0 desktop acceptance scenarios. |
| 8h | M14-DESKTOP-CLI-PARITY-GAPS.md | Desktop vs CLI gap list and polish phases. |
| 8h | PHASE-1-STABILIZATION.md | Phase 1 stabilize v1.x plan (`v1.5.2`). |
| 8i | PHASE-1-V152-SCENARIOS.md | v1.5.2 stabilization acceptance scenarios. |
| 8 | M5-PRODUCTION-READINESS.md | Understand the completed M5 production readiness plan. |
| 9 | M4-FEATURE-EXPANSION.md | Understand the completed M4 feature expansion plan. |
| 10 | PROJECT_CHARTER.md | Understand why LogScope exists. |
| 11 | PRODUCT_OVERVIEW.md | Understand what LogScope aims to build. |
| 12 | ENGINEERING_PRINCIPLES.md | Understand the engineering philosophy. |
| 13 | CPP_CODING_STANDARD.md | Understand repository-wide C++ conventions. |
| 14 | FOUNDATION_GUIDELINES.md | Understand Foundation implementation patterns. |
| 15 | Functional Requirements | Understand the required capabilities. |
| 16 | NFR-001 â€“ Quality Attributes | Understand the quality expectations. |
| 17 | ARCHITECTURE_OVERVIEW.md | Understand the overall system structure. |
| 18 | ARCHITECTURE_PRINCIPLES.md | Understand architectural design rules. |
| 19 | COMPONENT_CATALOG.md | Understand component responsibilities. |
| 20 | DOMAIN_MODEL.md | Understand the primary business concepts. |
| 21 | DATA_FLOW.md | Understand how information moves through the system. |
| 22 | HLD-001 â€“ Logical Architecture | Understand the complete system architecture. |
| 23 | WORKSPACE_MODEL.md | Understand the workspace and repository layout. |
| 24 | DEVELOPER_SETUP.md | Prepare the development environment and begin implementation. |
| 25 | DEVELOPER_GUIDE.md | Contribute features: workflow, testing, and PR expectations. |
| 26 | CONFIGURATION_GUIDE.md | Configure LogScope via properties files and environment variables. |
| 27 | USER_MANUAL.md | End-user workflows: analyze, investigate, sessions, large logs. |
| 28 | CLI_REFERENCE.md | Command-line usage reference. |
| 29 | PLUGIN_DEVELOPMENT_GUIDE.md | Built-in extensions and report contributor hooks. |

---

# 5. Document Relationships

The documentation is designed to provide complete traceability.

```text
ROADMAP
    â”‚
    â–¼
PROJECT CHARTER
    â”‚
    â–¼
PRODUCT OVERVIEW
    â”‚
    â–¼
FUNCTIONAL REQUIREMENTS
    â”‚
    â–¼
NON-FUNCTIONAL REQUIREMENTS
    â”‚
    â–¼
ARCHITECTURE OVERVIEW
    â”‚
    â–¼
ARCHITECTURE PRINCIPLES
    â”‚
    â–¼
COMPONENT CATALOG
    â”‚
    â–¼
DOMAIN MODEL
    â”‚
    â–¼
DATA FLOW
    â”‚
    â–¼
HLD-001 LOGICAL ARCHITECTURE
    â”‚
    â–¼
IMPLEMENTATION
```

---

# 6. Document Roles

| Document | Role |
|----------|------|
| ENGINEERING_PRINCIPLES.md | Defines the engineering philosophy and decision framework. |
| CPP_CODING_STANDARD.md | Defines repository-wide C++ conventions. |
| FOUNDATION_GUIDELINES.md | Defines Foundation implementation patterns, examples, and workflow checklists. |
| API_DESIGN_GUIDELINES.md | Defines public API design conventions. |
| PRODUCT.md | Provides a concise product summary at the documentation root. |
| CHANGELOG.md | Tracks notable project changes from M3 onward (repository root). |
| ROADMAP.md | Defines milestones, current progress, and planned work. |
| planning/ | Public tactical milestone plans (`Mn-*.md`) and post-v1 strategic summary. Long-horizon strategy beyond published plans is private and not linked from this repository. |
| architecture/decisions/ | Records Architecture Decision Records (ADRs). |
| architecture/foundation/ | Documents foundation-layer component designs. |
| implementation/ | Describes how architectural concepts map to the codebase. |
| handbook/ | Developer onboarding, configuration, workflow, and contribution checklists. |

---

# 7. Documentation Ownership

| Category | Responsibility |
|----------|----------------|
| Standards | Engineering practices and documentation conventions. |
| Vision | Product purpose and long-term direction. |
| Requirements | Functional and non-functional expectations. |
| Architecture | Logical system design and engineering decisions. |
| Implementation | Mapping architecture to code and workspace layout. |
| Handbook | Developer onboarding and environment setup. |
| Roadmap | Project planning and milestone tracking. |

---

# 8. Documentation Maintenance

Documentation should evolve together with the project.

The following principles apply:

- Every document shall have a single responsibility.
- Duplicate information should be avoided.
- Architecture changes shall be reflected in the relevant architecture documents.
- Requirements changes shall be reflected before implementation.
- Documentation should remain consistent across the repository.

---

# 9. Future Documentation

The following document categories may be introduced as LogScope evolves.

```text
testing/
release/
api/          # Doxygen-generated HTML (build/docs/api/html); see docs/api/README.md
```

These categories should be added only when they provide clear value and support the project's engineering goals.

---

# 10. Revision History

| Version | Date | Description |
|----------|------------|-----------------------------|
| 1.0.0 | 15-07-2026 | Initial document map. |
| 1.1.0 | 18-07-2026 | Updated structure for ENGINEERING_GUIDELINES, implementation/, ADRs, and foundation docs. |
| 1.2.0 | 18-07-2026 | Renamed ENGINEERING_GUIDELINES to FOUNDATION_GUIDELINES under standards/; updated reading order and cross-links. |
| 1.3.0 | 18-07-2026 | Extracted handbook workflow docs; deduplicated CPP_CODING_STANDARD with Foundation Guidelines. |
| 1.4.0 | 18-07-2026 | Added CHANGELOG and ROADMAP to document roles; repository-level doc structure. |
| 1.5.0 | 18-07-2026 | Added planning/ directory and M4-FEATURE-EXPANSION.md to structure and reading order. |
| 1.6.0 | 18-07-2026 | Added M5 planning, testing/, release/, SECURITY_REVIEW, and ADR-002. |
| 1.7.0 | 18-07-2026 | Added M6-LOG-FORMAT-INTELLIGENCE.md; updated reading order for current phase. |
| 1.8.0 | 21-07-2026 | Added PROJECT_CONTEXT.md for agent session bootstrap. |
| 1.9.0 | 21-07-2026 | Added POST_V1_STRATEGIC_ROADMAP.md and M7-SEARCH-ENGINE.md; updated reading order. |
| 2.0.0 | 24-07-2026 | Added M8-ADVANCED-REPORTING.md and ADR-003; updated reading order for `v1.3.0`. |
| 2.1.0 | 24-07-2026 | Note that long-horizon strategy beyond published Mn plans is private (no private URL). |
| 2.2.0 | 24-07-2026 | Merged M8 document map with private-strategy documentation note. |
| 2.3.0 | 24-07-2026 | Added M9-ANALYTICS-ENGINE.md and reading order for `v1.3.1`. |
| 2.4.0 | 24-07-2026 | Documented `scripts/` bulk-log CLI matrix tooling and CI integration. |
| 2.5.0 | 24-07-2026 | Added M10-QUERY-LANGUAGE.md, ADR-004, and reading order for `v1.4.0`. |
| 2.6.0 | 24-07-2026 | Added M11-STORAGE-LAYER.md, ADR-005, and reading order for `v1.4.1`. |
| 2.7.0 | 24-07-2026 | Added CONFIGURATION_GUIDE.md (Phase 1 stabilization). |
| 2.8.0 | 24-07-2026 | Added docs/api/ Doxygen scaffold and CI docs job. |
| 2.9.0 | 24-07-2026 | Added USER_MANUAL.md (Phase 1 stabilization). |
| 3.0.0 | 24-07-2026 | Added PLUGIN_DEVELOPMENT_GUIDE.md (Phase 1 stabilization). |
| 3.1.0 | 24-07-2026 | Added DEVELOPER_GUIDE.md; regression tests in document map. |
| 3.2.0 | 24-07-2026 | Updated M11, ADR-005, and reading order for `v1.4.2` bulk index write performance. |
| 3.3.0 | 25-07-2026 | Added `requirements/future/PRD-001-AI-Engineering-Agents.md` (post-v1.x vision). |
| 3.3.0 | 24-07-2026 | v1.4.2 doc sync: USER_MANUAL, PERFORMANCE, handbook revision history. |
| 3.4.0 | 24-07-2026 | USER_MANUAL expanded for full CLI workflow coverage. |
| 3.5.0 | 24-07-2026 | Added M11-V143-STORAGE-SCENARIOS.md for v1.4.3 design. |
| 3.8.0 | 25-07-2026 | M13 planning docs, ADR-007, and v1.5.1 AI scenarios in reading order. |
| 3.9.0 | 25-07-2026 | v1.5.1 release notes and M13 doc sync. |
| 3.10.0 | 30-07-2026 | v1.5.2 release notes; Phase 1 planning, tutorials, THIRD_PARTY_LICENSES in map. |
| 3.11.0 | 30-07-2026 | M14 desktop planning, ADR-008, v2.0.0 release notes. |
| 3.12.0 | 30-07-2026 | v2.0.1 release notes; handbook build-flavor sync (`DEVELOPER_SETUP`). |
| 3.13.0 | 30-07-2026 | v2.0.2 release notes; M14.12 desktop CLI parity polish. |
| 3.14.0 | 30-07-2026 | v2.0.3 release notes; desktop regression hotfix + GUI tests. |
| 3.13.0 | 30-07-2026 | `WINDOWS_RELEASE_SIGNING.md` handbook guide. |
| 3.14.0 | 30-07-2026 | `MACOS_RELEASE_NOTARIZATION.md` handbook guide. |
| 3.15.0 | 30-07-2026 | `M14-DESKTOP-CLI-PARITY-GAPS.md` desktop polish planning. |
| 3.16.0 | 30-07-2026 | M15 planning, ADR-009, v2.0.4 release notes in structure tree. |
