# Runtime

## Purpose

The Runtime module provides platform services shared across LogScope applications.

It depends on Foundation and exposes configuration, diagnostics, plugin registration, and service registration.

## Components

| Component | Description |
|-----------|-------------|
| `Configuration` | Key-value configuration store |
| `Diagnostics` | Diagnostic logging with level filtering and category tags |
| `PluginRegistry` | Plugin metadata registration |
| `ServiceRegistry` | Platform service registration |

## Diagnostics

`Diagnostics` writes leveled messages to stderr using the format:

```text
[LEVEL] [category] message
```

### Log levels

| Level | Label | Default visibility |
|-------|-------|--------------------|
| `Debug` | DEBUG | Suppressed (min level is Info) |
| `Info` | INFO | Visible |
| `Warning` | WARN | Visible |
| `Error` | ERROR | Visible |

### Configuration

Set the minimum log level via `Configuration`:

```cpp
configuration.set("log.level", "debug");  // debug, info, warn, error
Diagnostics::instance().applyConfiguration(configuration);
```

If `log.level` is not set, `SCOPE_LOG_LEVEL` environment variable is checked.

### Macros

Include `log_macros.hpp` (or `runtime.hpp`) and use:

```cpp
SCOPE_LOG_DEBUG("cli", "detail message");
SCOPE_LOG_INFO("cli", "starting");
SCOPE_LOG_WARN("runtime.config", "missing key");
SCOPE_LOG_ERROR("cli", "failed to open file");
```

### Categories

| Category | Used by |
|----------|---------|
| `cli` | CLI application |
| `runtime.config` | Configuration store |
| `runtime.plugin` | Plugin registry |
| `runtime.service` | Service registry |

## Dependencies

- Foundation (`scope_foundation`)

## Dependents

Applications and future Core modules.
