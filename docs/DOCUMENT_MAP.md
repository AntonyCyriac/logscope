# Document Map

| Field | Value |
|-------|-------|
| Document | Document Map |
| Category | Documentation |
| Version | 3.2.0 |
| Status | Approved |
| Created | 15-07-2026 |
| Last Updated | 24-07-2026 |

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
        │
        ▼
Product Vision
        │
        ▼
Requirements
        │
        ▼
Architecture
        │
        ▼
Implementation
        │
        ▼
Testing
        │
        ▼
Release
```

---

# 3. Documentation Structure

Repository-level documents:

```text
README.md
CHANGELOG.md
scripts/                 # Bulk log generation and CLI matrix runners
│   ├── README.md
│   ├── generate_bulk_log.py
│   ├── run_cli_matrix.py
│   └── check_benchmark_regression.py
```

`docs/` structure:

```text
docs/

├── DOCUMENT_MAP.md
├── PRODUCT.md
├── ROADMAP.md
│
├── planning/
│   ├── POST_V1_STRATEGIC_ROADMAP.md
│   ├── M4-FEATURE-EXPANSION.md
│   ├── M5-PRODUCTION-READINESS.md
│   ├── M6-LOG-FORMAT-INTELLIGENCE.md
│   ├── M7-SEARCH-ENGINE.md
│   ├── M8-ADVANCED-REPORTING.md
│   ├── M9-ANALYTICS-ENGINE.md
│   ├── M10-QUERY-LANGUAGE.md
│   └── M11-STORAGE-LAYER.md
│
│   (Long-horizon strategy beyond published Mn plans is maintained privately
│    outside this repository; public Mn docs are added when implementation starts.)
│
├── testing/
│   ├── TESTING.md
│   └── PERFORMANCE.md
│
├── api/
│   ├── README.md
│   ├── mainpage.md
│   └── Doxyfile.in          # CMake-generated Doxygen config
│
├── release/
│   ├── RELEASE.md
│   └── V1_VALIDATION.md
│
├── handbook/
│   ├── PROJECT_CONTEXT.md
│   ├── CODE_REVIEW_CHECKLIST.md
│   ├── CONFIGURATION_GUIDE.md
│   ├── DEVELOPER_GUIDE.md
│   ├── DEVELOPER_SETUP.md
│   ├── GIT_CONVENTIONS.md
│   ├── PULL_REQUEST_GUIDE.md
│   ├── PLUGIN_DEVELOPMENT_GUIDE.md
│   ├── SECURITY_REVIEW.md
│   └── USER_MANUAL.md
│
├── standards/
│   ├── API_DESIGN_GUIDELINES.md
│   ├── CPP_CODING_STANDARD.md
│   ├── DOCUMENT_STANDARD.md
│   ├── ENGINEERING_PRINCIPLES.md
│   └── FOUNDATION_GUIDELINES.md
│
├── vision/
│   ├── PROJECT_CHARTER.md
│   └── PRODUCT_OVERVIEW.md
│
├── requirements/
│   ├── functional/
│   │   ├── FR-001-Analyze-Logs.md
│   │   ├── FR-002-Investigate-Logs.md
│   │   ├── FR-003-Generate-Reports.md
│   │   └── FR-004-Extend-LogScope.md
│   │
│   └── non_functional/
│       └── NFR-001-Quality-Attributes.md
│
├── architecture/
│   ├── ARCHITECTURE_OVERVIEW.md
│   ├── ARCHITECTURE_PRINCIPLES.md
│   ├── COMPONENT_CATALOG.md
│   ├── DOMAIN_MODEL.md
│   ├── DATA_FLOW.md
│   ├── HLD-001-Logical-Architecture.md
│   │
│   ├── decisions/
│   │   ├── ADR-001-Testing-Framework.md
│   │   ├── ADR-002-Benchmark-Framework.md
│   │   ├── ADR-003-PDF-Report-Generation.md
│   │   ├── ADR-004-Query-DSL-Grammar.md
│   │   └── ADR-005-Storage-Architecture.md
│   │
│   └── foundation/
│       └── RESULT.md
│
└── implementation/
    └── WORKSPACE_MODEL.md
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
| 8 | M11-STORAGE-LAYER.md | Understand the completed M11 storage layer (`v1.4.1` core, `v1.4.2` bulk write perf). |
| 8 | M5-PRODUCTION-READINESS.md | Understand the completed M5 production readiness plan. |
| 9 | M4-FEATURE-EXPANSION.md | Understand the completed M4 feature expansion plan. |
| 10 | PROJECT_CHARTER.md | Understand why LogScope exists. |
| 11 | PRODUCT_OVERVIEW.md | Understand what LogScope aims to build. |
| 12 | ENGINEERING_PRINCIPLES.md | Understand the engineering philosophy. |
| 13 | CPP_CODING_STANDARD.md | Understand repository-wide C++ conventions. |
| 14 | FOUNDATION_GUIDELINES.md | Understand Foundation implementation patterns. |
| 15 | Functional Requirements | Understand the required capabilities. |
| 16 | NFR-001 – Quality Attributes | Understand the quality expectations. |
| 17 | ARCHITECTURE_OVERVIEW.md | Understand the overall system structure. |
| 18 | ARCHITECTURE_PRINCIPLES.md | Understand architectural design rules. |
| 19 | COMPONENT_CATALOG.md | Understand component responsibilities. |
| 20 | DOMAIN_MODEL.md | Understand the primary business concepts. |
| 21 | DATA_FLOW.md | Understand how information moves through the system. |
| 22 | HLD-001 – Logical Architecture | Understand the complete system architecture. |
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
    │
    ▼
PROJECT CHARTER
    │
    ▼
PRODUCT OVERVIEW
    │
    ▼
FUNCTIONAL REQUIREMENTS
    │
    ▼
NON-FUNCTIONAL REQUIREMENTS
    │
    ▼
ARCHITECTURE OVERVIEW
    │
    ▼
ARCHITECTURE PRINCIPLES
    │
    ▼
COMPONENT CATALOG
    │
    ▼
DOMAIN MODEL
    │
    ▼
DATA FLOW
    │
    ▼
HLD-001 LOGICAL ARCHITECTURE
    │
    ▼
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
| 3.3.0 | 24-07-2026 | v1.4.2 doc sync: USER_MANUAL, PERFORMANCE, handbook revision history. |
| 3.4.0 | 24-07-2026 | USER_MANUAL expanded for full CLI workflow coverage. |