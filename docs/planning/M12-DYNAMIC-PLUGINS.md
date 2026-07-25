# M12 – Dynamic Plugins

| Field | Value |
|-------|-------|
| Document | M12 – Dynamic Plugins |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | Complete |
| Created | 25-07-2026 |
| Last Updated | 25-07-2026 |

---

# 1. Purpose

Deliver **M12 – Dynamic Plugins** at **`v1.5.0`**: runtime `.so`/`.dll` loading with parser, report, search, and storage provider registration, Plugin SDK, and sample plugins.

See [ADR-006](../architecture/decisions/ADR-006-Plugin-Loading.md) and [M12 v1.5.0 Scenarios](M12-V150-PLUGIN-SCENARIOS.md).

---

# 2. Dependencies

| Prior milestone | M12 dependency |
|-----------------|----------------|
| M4.4 — ExtensionManager | Static registration, config keys, lifecycle |
| M6.5 — Format profiles | Config-before-plugins guardrail |
| M8 — Reporting | `ReportSectionContributor` hook |
| M11 — Storage | `IndexStore` abstraction, `createIndexStore` factory |

---

# 3. Phased Delivery

| Phase | Focus | Status |
|-------|-------|--------|
| M12.0 | ADR-006, planning doc, scenarios | 🟡 In progress |
| M12.1 | `SharedLibrary`, `PluginLoader`, config | ✅ Complete |
| M12.2 | Dynamic `ExtensionManager` integration | ✅ Complete |
| M12.3 | Report provider plugins + sample | ✅ Complete |
| M12.4 | `ParserRegistry` + `AnalysisEngine` wire-up | ✅ Complete |
| M12.5 | `SearchProviderRegistry` + sample | ✅ Complete |
| M12.6 | `StorageBackendRegistry` + sample | ✅ Complete |
| M12.7 | Plugin SDK headers, CMake, CI sample builds | ✅ Complete |
| M12.8 | Doc sync, `v1.5.0` release | ✅ Complete |

---

# 4. Deliverables

## Core

- `core/plugin/` — `SharedLibrary`, `PluginLoader`, `PluginHostApi`
- Dynamic extension registration in `ExtensionManager`
- Config: `plugins.enabled`, `plugins.paths`, `LOGSCOPE_PLUGIN_PATH`
- `storage.backend` = `sqlite` (default) | `plugin:<id>`

## Provider registries

- `ParserRegistry` (`core/analysis`)
- `SearchProviderRegistry` (`core/search`)
- `StorageBackendRegistry` (`core/storage`)
- Report contributors (existing `ReportSectionRegistry`)

## SDK and samples

- `include/logscope/plugin/plugin.h`
- `examples/plugins/sample_report_plugin`
- `examples/plugins/sample_parser_plugin`
- `examples/plugins/sample_search_plugin`
- `examples/plugins/sample_storage_plugin`

## Tests

- Unit: loader, version mismatch, bad symbols, registry
- Integration: load sample plugin, verify provider behaviour
- E2E: `extensions list` shows dynamic plugins
- CI: build sample plugins on Ubuntu, Windows, macOS

---

# 5. Non-goals

- Plugin marketplace UI
- `logscope install` package manager
- Plugin signing or sandboxing (metadata hooks only)
- M13 AI Assistant runtime
- Breaking changes to default CLI behaviour (`plugins.enabled=false`)

---

# 6. FR-004 mapping

| FR-004 criterion | M12 implementation |
|------------------|-------------------|
| Configurable behaviour | `plugins.enabled`, `plugins.paths`, `storage.backend` |
| Extension support | Dynamic load + registries |
| Existing functionality preserved | Default config unchanged |
| Extension failures isolated | Per-plugin error handling (P7 scenarios) |
| Identify extensions | `extensions list` / `describe` with source and API version |

---

# 7. Engineering conventions

| Convention | Value |
|------------|-------|
| Module | `core/plugin` (`scope_plugin`) |
| SDK | `include/logscope/plugin/` |
| Tests | `scope_plugin_tests` + integration + e2e |
| Branch | `feat/v1.5.0-m12-dynamic-plugins` |

---

# 8. Related documents

| Document | Purpose |
|----------|---------|
| [ADR-006](../architecture/decisions/ADR-006-Plugin-Loading.md) | Architecture decision |
| [M12-V150-PLUGIN-SCENARIOS.md](M12-V150-PLUGIN-SCENARIOS.md) | Acceptance gate |
| [PLUGIN_DEVELOPMENT_GUIDE.md](../handbook/PLUGIN_DEVELOPMENT_GUIDE.md) | Author guide |
| [FR-004](../requirements/functional/FR-004-Extend-LogScope.md) | Requirements |
