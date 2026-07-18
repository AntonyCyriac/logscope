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
| [Foundation](foundation/README.md) | Complete | Universal types: `Status`, `Error`, `Result<T>`, time, path, string, and hash utilities |
| [Runtime](runtime/README.md) | Complete | Configuration store, diagnostics, plugin and service registries |
| [Configuration](configuration/README.md) | Complete | Configuration loading, validation, environment overrides |
| [Source](source/README.md) | Complete | Log source validation, acquisition, and dataset preparation |
| [Analysis](analysis/README.md) | Complete | Source dataset analysis and analysis model production |
| [Investigation](investigation/README.md) | Complete | Search, filter, and inspect analysis models |
| [Reporting](reporting/README.md) | Complete | Report generation from analysis models |
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
