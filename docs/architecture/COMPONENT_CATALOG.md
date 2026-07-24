# Component Catalog

| Field | Value |
|-------|-------|
| Document | Component Catalog |
| Category | Architecture |
| Version | 1.1.0 |
| Status | Approved |
| Created | 15-07-2026 |
| Last Updated | 24-07-2026 |

---

# 1. Purpose

This document defines the major architectural components of LogScope.

It establishes the responsibility, ownership, and boundaries of each component within the system.

The Component Catalog serves as the authoritative reference for architectural decomposition and provides the foundation for the High-Level Design (HLD).

---

# 2. Scope

This document applies to the logical architecture of LogScope.

It defines **what** components exist and **what responsibilities they own**.

It intentionally does not define:

- Internal implementation
- Class design
- Data structures
- Algorithms
- Technology choices

Those concerns are addressed in later architecture and implementation documents.

---

# 3. Architectural Principles

The component architecture follows the engineering principles defined in:

- ENGINEERING_PRINCIPLES.md
- NFR-001 – Quality Attributes

Every component SHALL:

- Have a single responsibility.
- Expose clear boundaries.
- Minimize coupling.
- Maximize cohesion.
- Support future extensibility.
- Remain independently maintainable.

---

# 4. Component Overview

| ID | Component | Primary Responsibility |
|----|-----------|------------------------|
| C01 | Core | Shared domain models, interfaces, common utilities, and shared services. |
| C02 | Source Manager | Discover, validate, and acquire log data from supported sources. |
| C03 | Analysis Engine | Analyze supported log data and produce analysis results. |
| C04 | Investigation Engine | Search, filter, inspect, correlate, and explore analysis results. |
| C05 | Reporting Engine | Generate reports and export analysis results. |
| C06 | Extension Manager | Discover, load, and manage supported extensions. |
| C07 | Configuration Manager | Load, validate, and provide product configuration. |
| C08 | Diagnostics Manager | Logging, diagnostics, metrics, tracing, and health information. |
| C09 | CLI | Provide the command-line user interface. |
| C10 | Platform Services | Provide platform abstraction for operating system services. |

---

# 5. Component Responsibilities

## C01 – Core

### Responsibility

Provide reusable domain abstractions shared across the entire system.

### Owns

- Domain models
- Common interfaces
- Shared utilities
- Error model
- Common services

### Depends On

None.

---

## C02 – Source Manager

### Responsibility

Acquire log data from supported sources.

### Owns

- Source discovery
- Source validation
- Source access
- Input abstraction

### Depends On

- Core
- Configuration Manager
- Platform Services

---

## C03 – Analysis Engine

### Responsibility

Analyze supported log data and produce analysis results.

### Owns

- Analysis workflow
- Analysis orchestration
- Processing coordination

### Depends On

- Core
- Source Manager
- Extension Manager

---

## C04 – Investigation Engine

### Responsibility

Provide capabilities for exploring and understanding analysis results.

### Owns

- Search
- Filtering
- Correlation
- Navigation
- Result inspection

### Depends On

- Core
- Analysis Engine

---

## C05 – Reporting Engine

### Responsibility

Generate reusable reports from analysis results.

### Owns

- Report generation
- Section registry and contributed sections
- Output formatting (text, JSON, CSV, Markdown, HTML, PDF)
- Export workflow

### Depends On

- Core
- Investigation Engine

---

## C06 – Extension Manager

### Responsibility

Manage supported extension mechanisms.

### Owns

- Extension discovery
- Extension lifecycle
- Extension registration

### Depends On

- Core
- Configuration Manager

---

## C07 – Configuration Manager

### Responsibility

Provide validated configuration to the system.

### Owns

- Configuration loading
- Validation
- Configuration access

### Depends On

- Core
- Platform Services

---

## C08 – Diagnostics Manager

### Responsibility

Provide diagnostic capabilities for engineers.

### Owns

- Logging
- Metrics
- Tracing
- Health reporting

### Depends On

- Core

---

## C09 – CLI

### Responsibility

Provide the primary command-line user experience.

### Owns

- Command processing
- User interaction
- Console output

### Depends On

- Core
- Source Manager
- Analysis Engine
- Investigation Engine
- Reporting Engine
- Configuration Manager

---

## C10 – Platform Services

### Responsibility

Abstract operating system services required by the product.

### Owns

- File system access
- Environment access
- Process interaction
- Platform abstraction

### Depends On

- Core

---

# 6. Dependency Principles

The component architecture follows these dependency rules.

- Dependencies SHALL point toward lower-level components.
- Components SHALL NOT introduce circular dependencies.
- Components SHOULD communicate through stable interfaces.
- Core SHALL remain independent of higher-level components.
- Platform-specific functionality SHALL remain isolated.

---

# 7. Traceability

| Functional Requirement | Primary Component(s) |
|------------------------|----------------------|
| FR-001 – Analyze Logs | Source Manager, Analysis Engine |
| FR-002 – Investigate Logs | Investigation Engine |
| FR-003 – Generate Reports | Reporting Engine |
| FR-004 – Extend LogScope | Extension Manager |
| NFR-001 – Quality Attributes | All Components |

---

# 8. Future Evolution

The component architecture is expected to evolve through the addition of new components rather than expansion of existing responsibilities.

New components should:

- Address a distinct responsibility.
- Preserve existing architectural boundaries.
- Minimize coupling with existing components.

Major architectural changes should be documented through Architecture Decision Records (ADRs).

---

# 9. Revision History

| Version | Date | Description |
|----------|------------|-----------------------------|
| 1.0.0 | 15-07-2026 | Initial component catalog. |
| 1.1.0 | 24-07-2026 | M8 reporting: section registry, contributed sections, HTML/PDF formats (`v1.3.0`). |
