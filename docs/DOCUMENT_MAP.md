# Document Map

| Field | Value |
|-------|-------|
| Document | Document Map |
| Category | Documentation |
| Version | 1.3.0 |
| Status | Approved |
| Created | 15-07-2026 |
| Last Updated | 18-07-2026 |

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

```text
docs/

├── DOCUMENT_MAP.md
├── PRODUCT.md
├── ROADMAP.md
│
├── handbook/
│   ├── CODE_REVIEW_CHECKLIST.md
│   ├── DEVELOPER_SETUP.md
│   ├── GIT_CONVENTIONS.md
│   └── PULL_REQUEST_GUIDE.md
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
│   │   └── ADR-001-Testing-Framework.md
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
| 1 | ROADMAP.md | Understand the project milestones and current development phase. |
| 2 | PROJECT_CHARTER.md | Understand why LogScope exists. |
| 3 | PRODUCT_OVERVIEW.md | Understand what LogScope aims to build. |
| 4 | ENGINEERING_PRINCIPLES.md | Understand the engineering philosophy. |
| 5 | CPP_CODING_STANDARD.md | Understand repository-wide C++ conventions. |
| 6 | FOUNDATION_GUIDELINES.md | Understand Foundation implementation patterns. |
| 7 | Functional Requirements | Understand the required capabilities. |
| 8 | NFR-001 – Quality Attributes | Understand the quality expectations. |
| 9 | ARCHITECTURE_OVERVIEW.md | Understand the overall system structure. |
| 10 | ARCHITECTURE_PRINCIPLES.md | Understand architectural design rules. |
| 11 | COMPONENT_CATALOG.md | Understand component responsibilities. |
| 12 | DOMAIN_MODEL.md | Understand the primary business concepts. |
| 13 | DATA_FLOW.md | Understand how information moves through the system. |
| 14 | HLD-001 – Logical Architecture | Understand the complete system architecture. |
| 15 | WORKSPACE_MODEL.md | Understand the workspace and repository layout. |
| 16 | DEVELOPER_SETUP.md | Prepare the development environment and begin implementation. |

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
| architecture/decisions/ | Records Architecture Decision Records (ADRs). |
| architecture/foundation/ | Documents foundation-layer component designs. |
| implementation/ | Describes how architectural concepts map to the codebase. |
| handbook/ | Developer onboarding, workflow, and contribution checklists. |

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
api/
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
