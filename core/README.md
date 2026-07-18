# Core

## Purpose

The Core area contains reusable engineering capabilities shared across the Scope ecosystem.

Core modules provide common building blocks used by LogScope and future Scope products.

The Core layer contains no product-specific business logic.

---

## Design Principles

- Product independent
- Reusable across multiple products
- Stable public interfaces
- Minimal dependencies
- Built through proven reuse

---

## Modules

| Module | Status | Responsibility |
|--------|--------|----------------|
| [Foundation](foundation/README.md) | In progress | `Status`, `Error`, `Result<T>`, `Version`, `Uuid` |
| Runtime | Planned | Platform abstraction |
| Configuration | Planned | Configuration management |
| Analysis | Planned | Analysis pipeline |
| Investigation | Planned | Investigation workflow |
| Reporting | Planned | Report generation |
| Search | Planned | Search and filtering |
| Workspace | Planned | Sessions and persistence |

---

## Dependency Rules

Core modules may depend only on other Core modules.

Core modules must never depend on:

- Products
- Applications
- Extensions

---

## Engineering Philosophy

The Core evolves by extracting reusable capabilities from products.

Capabilities are promoted only after reuse has been demonstrated.
