# Architecture Overview

| Field | Value |
|-------|-------|
| Document | Architecture Overview |
| Category | Architecture |
| Version | 1.0.0 |
| Status | Approved |
| Created | 15-07-2026 |
| Last Updated | 15-07-2026 |

---

# 1. Purpose

This document provides a high-level overview of the LogScope architecture.

It defines the architectural layers, explains how major components are organized, establishes dependency rules, and describes how the architecture supports the project's functional and non-functional requirements.

This document serves as the entry point to the architecture documentation.

---

# 2. Scope

This document describes the logical organization of the system.

It does not describe:

- Internal implementation
- Algorithms
- Programming language
- Class design
- Data structures

Those topics are covered by lower-level architectural and implementation documents.

---

# 3. Architectural Goals

The architecture is designed to achieve the following goals:

- Simplicity
- Maintainability
- Extensibility
- Testability
- Platform Independence
- Predictable Data Flow
- Clear Separation of Concerns

These goals are derived from the Engineering Principles and Non-Functional Requirements.

---

# 4. Architectural Layers

LogScope is organized into four logical layers.

```text
+------------------------------------------------------+
|               Presentation Layer                     |
+------------------------------------------------------+

+------------------------------------------------------+
|                Application Layer                     |
+------------------------------------------------------+

+------------------------------------------------------+
|                  Domain Layer                        |
+------------------------------------------------------+

+------------------------------------------------------+
|               Infrastructure Layer                  |
+------------------------------------------------------+
```

Each layer has a distinct responsibility and communicates only through well-defined interfaces.

For a visual map of components C01–C10 and domain libraries, see the [component structure diagram](COMPONENT_CATALOG.md#component-structure-diagram) in the Component Catalog.

---

# 5. Layer Responsibilities

## Presentation Layer

### Responsibility

Provides the primary interaction between users and LogScope.

### Components

- CLI

This layer is responsible for:

- Processing user commands
- Displaying results
- Reporting errors
- Managing user interaction

Business logic shall not reside in this layer.

---

## Application Layer

### Responsibility

Coordinates product workflows and orchestrates interactions between components.

### Components

- Source Manager
- Investigation Engine
- Reporting Engine
- Extension Manager
- Configuration Manager

Responsibilities include:

- Workflow coordination
- Component orchestration
- Configuration usage
- Extension lifecycle
- User-driven operations

---

## Domain Layer

### Responsibility

Contains the core business logic and domain concepts of LogScope.

### Components

- Analysis Engine
- Core
- Domain Model

Responsibilities include:

- Log analysis
- Domain rules
- Domain objects
- Shared abstractions

This layer represents the heart of the product.

---

## Infrastructure Layer

### Responsibility

Provides platform-specific capabilities required by higher layers.

### Components

- Platform Services

Responsibilities include:

- File system access
- Environment access
- Platform abstraction
- Operating system interaction

Business logic shall not reside in this layer.

---

# 6. Cross-Cutting Concerns

Certain capabilities span multiple architectural layers.

## Diagnostics

Diagnostics support every major subsystem.

Responsibilities include:

- Logging
- Metrics
- Tracing
- Health reporting

Diagnostics should be available across all architectural layers while remaining independent of business logic.

---

# 7. Component Placement

| Component | Layer |
|-----------|-------|
| CLI | Presentation |
| Source Manager | Application |
| Investigation Engine | Application |
| Reporting Engine | Application |
| Extension Manager | Application |
| Configuration Manager | Application |
| Analysis Engine | Domain |
| Core | Domain |
| Domain Model | Domain |
| Platform Services | Infrastructure |
| Diagnostics Manager | Cross-Cutting |

---

# 8. Dependency Rules

The architecture follows strict dependency rules.

```text
Presentation
        │
        ▼
Application
        │
        ▼
Domain
        │
        ▼
Infrastructure
```

Rules:

- Dependencies flow downward.
- Circular dependencies are prohibited.
- Components communicate through stable interfaces.
- Platform-specific behaviour remains isolated.
- Domain concepts remain independent of presentation technology.

---

# 9. Architectural View

The logical architecture is illustrated below.

```text
                     User
                       │
                       ▼
              Presentation Layer
                      CLI
                       │
                       ▼
              Application Layer
   ┌─────────────────────────────────────┐
   │ Source Manager                      │
   │ Investigation Engine                │
   │ Reporting Engine                    │
   │ Extension Manager                   │
   │ Configuration Manager               │
   └─────────────────────────────────────┘
                       │
                       ▼
                 Domain Layer
   ┌─────────────────────────────────────┐
   │ Analysis Engine                     │
   │ Core                               │
   │ Domain Model                       │
   └─────────────────────────────────────┘
                       │
                       ▼
             Infrastructure Layer
   ┌─────────────────────────────────────┐
   │ Platform Services                   │
   └─────────────────────────────────────┘

           Diagnostics Manager
            (Cross-Cutting)
```

---

# 10. Relationship to Other Architecture Artifacts

This document provides the architectural context for the following artifacts.

| Artifact | Purpose |
|----------|---------|
| Component Catalog | Defines component responsibilities. |
| Domain Model | Defines the primary business concepts. |
| Data Flow | Describes movement of domain objects. |
| Architecture Principles | Defines architectural rules. |
| HLD-001 | Defines the complete high-level architecture. |

Together these documents provide a complete logical description of the LogScope architecture.

---

# 11. Revision History

| Version | Date | Description |
|----------|------------|-----------------------------|
| 1.0.0 | 15-07-2026 | Initial architecture overview. |
