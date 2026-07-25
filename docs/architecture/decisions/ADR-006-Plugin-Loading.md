# ADR-006: Plugin Loading Architecture

- **Status:** Accepted
- **Date:** 25-07-2026

---

## Context

LogScope through **v1.4.3** shipped a **static** extension system (`ExtensionManager`, built-in registration, `ReportSectionContributor` hook). **M12 (`v1.5.0`)** added dynamic `.so`/`.dll` loading for parser, report, search, and storage providers. FR-004 requires extension failures to not compromise unrelated capabilities.

M12 must add runtime plugin loading for parser, report, search, and storage providers without redesigning the core pipeline.

---

## Decision

### 1. C ABI entry point

Out-of-tree plugins export:

```c
int logscope_plugin_register(const LogScopeHostApi* host);
```

- Returns `0` on success, non-zero on failure.
- Symbol name is stable across platforms.
- Plugins link against `logscope_plugin_api` (header-only C API + host version constants).

### 2. API versioning

| Constant | Value (M12) | Purpose |
|----------|-------------|---------|
| `LOGSCOPE_PLUGIN_API_VERSION` | `1` | Plugin descriptor version |
| `LOGSCOPE_HOST_API_VERSION` | `1` | Host services struct version |

Host rejects plugins when `plugin_api_version > LOGSCOPE_HOST_API_VERSION` (fail closed).

### 3. Discovery

| Source | Key / variable |
|--------|----------------|
| Configuration | `plugins.enabled` (default `false`) |
| Configuration | `plugins.paths` (semicolon-separated on Windows, colon on Unix) |
| Environment | `LOGSCOPE_PLUGIN_PATH` (same separator rules) |

When `plugins.enabled=false`, no filesystem scan occurs (zero behaviour change for existing users).

### 4. Platform loading

`SharedLibrary` abstraction:

| Platform | API |
|----------|-----|
| Linux / macOS | `dlopen` / `dlsym` / `dlclose` |
| Windows | `LoadLibraryW` / `GetProcAddress` / `FreeLibrary` |

File extensions: `.so` (Linux), `.dylib` (macOS), `.dll` (Windows).

### 5. Lifecycle

```text
scan paths → load library → resolve logscope_plugin_register
    → validate API version → call register(host)
    → register ExtensionDescriptor (dynamic)
    → initializeEnabled() (same as built-ins)
```

Unload on `ExtensionManager` destruction (RAII `LoadedPlugin` vector).

### 6. Provider registries

| Registry | Module | Integration |
|----------|--------|-------------|
| `ParserRegistry` | `core/analysis` | `AnalysisEngine` consults before built-in parsers |
| `ReportSectionRegistry` | `core/reporting` | existing hook; plugins register contributors |
| `SearchProviderRegistry` | `core/search` | optional named provider; CLI/search path |
| `StorageBackendRegistry` | `core/storage` | `createIndexStore` when `storage.backend=plugin:<id>` |

Built-in SQLite remains default (`storage.backend=sqlite`).

### 7. Failure isolation (FR-004.5)

- Load failure: log error, skip plugin, continue.
- Register failure: mark extension `InitializationFailed`, continue.
- Broken plugin must not abort unrelated extensions or core commands.

### 8. Security (M12)

- Plugins are **trusted local code**; no signing, sandbox, or marketplace in M12.
- Document threat model: loading a malicious `.so` equals arbitrary code execution.
- Future milestones may add signing and sandboxing.

### 9. SDK layout

```text
include/logscope/plugin/plugin.h       # C ABI
include/logscope/plugin/plugin.hpp     # C++ helpers (optional)
```

CMake: `LogScopePluginSdk` INTERFACE target; sample plugins under `examples/plugins/`.

---

## Consequences

**Positive**

- Open/Closed: new providers without recompiling core.
- Unified `extensions list/describe` for built-in and dynamic extensions.
- Clear ADR gate for M13 AI analyzer providers.

**Negative**

- Cross-platform plugin builds require CI matrix for sample plugins.
- C ABI limits direct C++ virtual interfaces; SDK wraps with adapters.

**Neutral**

- Legacy `PluginRegistry` (name/version metadata) deprecated; bridge logs once at load.

---

## Related

- [FR-004](../../requirements/functional/FR-004-Extend-LogScope.md)
- [ADR-005](ADR-005-Storage-Architecture.md) — `IndexStore` backend extension
- [M12-DYNAMIC-PLUGINS.md](../../planning/M12-DYNAMIC-PLUGINS.md)
- [PLUGIN_DEVELOPMENT_GUIDE.md](../../handbook/PLUGIN_DEVELOPMENT_GUIDE.md)
