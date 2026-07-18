# Runtime

## Purpose

The Runtime module provides platform services shared across LogScope applications.

It depends on Foundation and exposes configuration, diagnostics, plugin registration, and service registration.

## Components

| Component | Description |
|-----------|-------------|
| `Configuration` | Key-value configuration store |
| `Diagnostics` | Diagnostic logging |
| `PluginRegistry` | Plugin metadata registration |
| `ServiceRegistry` | Platform service registration |

## Dependencies

- Foundation (`scope_foundation`)

## Dependents

Applications and future Core modules.
