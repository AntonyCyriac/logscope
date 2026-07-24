# Plugin Development Guide

| Field | Value |
|-------|-------|
| Document | Plugin Development Guide |
| Category | Handbook |
| Version | 1.0.0 |
| Status | Approved |
| Created | 24-07-2026 |
| Last Updated | 24-07-2026 |

---

# 1. Purpose

This guide explains how LogScope extensions work today and how contributors add new **built-in** extensions in the core repository.

**Current scope (v1.4.1):** static, compile-time extensions registered through `ExtensionManager`. There is no `.so`/`.dll` loading yet — that is planned for **M12 – Dynamic Plugins** (`v1.5.0`).

Phase 1 stabilization deliverable — see [Post-v1 Strategic Roadmap](../planning/POST_V1_STRATEGIC_ROADMAP.md#phase-1--stabilize-v1x).

Related requirements: [FR-004 – Extend LogScope](../requirements/functional/FR-004-Extend-LogScope.md). Implementation baseline: [M4.4 – Extension Ecosystem](../planning/M4-FEATURE-EXPANSION.md).

---

# 2. Architecture overview

```text
Configuration (extensions.<id>.enabled)
        │
        ▼
ExtensionManager::createWithBuiltIns()
        │  registerBuiltInExtensions()
        ▼
applyConfiguration() → initializeEnabled()
        │
        ├── metadata only (list/describe)
        └── optional init hook → register hooks (e.g. report contributors)
```

| Component | Location | Role |
|-----------|----------|------|
| `ExtensionDescriptor` | `core/extension/extension_descriptor.hpp` | Registration metadata + optional init function |
| `ExtensionManager` | `core/extension/extension_manager.hpp` | Config, lifecycle, discovery (C06) |
| `registerBuiltInExtensions` | `core/extension/builtin_extensions.cpp` | Built-in extension table |
| `ReportSectionContributor` | `core/reporting/report_section_contributor.hpp` | M8 report hook (only plugin interface today) |
| `scope_extension` | `core/extension/CMakeLists.txt` | Static library target |

**Not the extension system:** `core/runtime/plugin_registry.hpp` (`PluginRegistry`) is legacy M3 name/version metadata only. New work uses `ExtensionManager`.

---

# 3. Built-in extensions today

| ID | Version | Init hook | Purpose |
|----|---------|-----------|---------|
| `analysis.log-levels` | 1.0.0 | No | Documents generic level statistics during analysis |
| `source.files` | 1.0.0 | No | Documents file, directory, and stdin source acquisition |
| `reporting.multi-format` | 1.3.0 | Yes | Multi-format reporting; registers `ReportSectionContributor` |

List or describe from the CLI:

```bash
logscope extensions list --config samples/logscope.properties
logscope extensions describe reporting.multi-format --config samples/logscope.properties
```

---

# 4. Extension ID conventions

Use dot-separated names: `<category>.<feature>`.

| Prefix | Intended capability | M12 direction |
|--------|---------------------|---------------|
| `analysis.*` | Analysis pipeline additions | Parser plugins |
| `source.*` | Log source providers | Custom source readers |
| `reporting.*` | Report sections and formats | Report generator plugins |
| `search.*` | (reserved) | Search provider plugins |
| `storage.*` | (reserved) | Storage provider plugins |

IDs must be unique. Duplicate registration is rejected with a diagnostic log entry.

---

# 5. Configuration

Enable or disable extensions in `logscope.properties`:

```properties
extensions.analysis.log-levels.enabled=true
extensions.source.files.enabled=true
extensions.reporting.multi-format.enabled=false
```

| Value | Effect |
|-------|--------|
| `true`, `1`, `yes`, `on` | Enabled (case-insensitive) |
| Any other non-empty value | Disabled |
| Key absent | Uses `enabledByDefault` from descriptor |

Validate a file:

```bash
logscope config validate --config samples/logscope.properties
```

Full config reference: [Configuration Guide](CONFIGURATION_GUIDE.md#68-extensions).

### Enablement semantics

- `applyConfiguration` sets the enabled flag per extension.
- `initializeEnabled` runs the init hook only for enabled extensions.
- Init failures set status to `InitializationFailed` without crashing unrelated capabilities (FR-004.4).
- **Important:** disabling an extension stops its init hook; core singletons may still register related behavior independently. Design init hooks to be the single registration path when enablement must gate behavior.

---

# 6. CLI integration pattern

Every pipeline command follows this sequence (see `apps/cli/analyze_command.cpp` and siblings):

```cpp
#include "extension.hpp"

scope::extension::ExtensionManager manager =
    scope::extension::ExtensionManager::createWithBuiltIns();
manager.applyConfiguration(configurationManager.configuration());
manager.initializeEnabled();
```

`ExtensionManager` is not passed into `AnalysisEngine` or `InvestigationEngine`. Extensions integrate through **init-hook side effects** (e.g. registering report contributors) or serve as **discoverable capability metadata**.

---

# 7. Adding a built-in extension

## 7.1 Metadata-only extension

Use when the capability already lives in core and you want discoverability (FR-004.5) without a separate init hook.

1. Add a descriptor in `core/extension/builtin_extensions.cpp`:

```cpp
manager.registerBuiltIn(ExtensionDescriptor{
    "analysis.my-feature",
    "1.0.0",
    "Short description for extensions describe.",
    nullptr,   // no init hook
    true       // enabledByDefault
});
```

2. Implement the capability in the appropriate `core/` module.
3. Add unit tests in `core/extension/tests/extension_manager_test.cpp` if config behavior matters.
4. Update [Configuration Guide](CONFIGURATION_GUIDE.md) and this guide's built-in table.
5. Rebuild — `scope_extension` picks up changes automatically.

## 7.2 Extension with an init hook

Use when registration must run at startup (report contributors, future hook types).

1. Define an init function returning `foundation::Result<bool>`:

```cpp
foundation::Result<bool> initializeMyExtension()
{
    // Register hooks, wire singletons, etc.
    return foundation::Result<bool>(true);
}
```

2. Register in `builtin_extensions.cpp` with the function pointer.
3. Guard idempotent registration (see `registerReportingContributors` static flag pattern).

---

# 8. Report section contributor hook

The only formal extension hook today is `ReportSectionContributor` (M8).

## 8.1 Interface

```cpp
class ReportSectionContributor
{
  public:
    virtual ~ReportSectionContributor() = default;
    virtual std::string id() const = 0;
    virtual ReportSection section() const noexcept = 0;
    virtual ReportFragment render(const analysis::AnalysisModel& model) const = 0;
};
```

Reference implementation: `FormatsFooterContributor` in `core/reporting/report_contributors.cpp`.

## 8.2 Steps to add a section

1. **Add enum value** (if new section) in `core/reporting/report_section.hpp` (`ReportSection`).
2. **Parse CLI/config name** in `ReportSections::parse()` if the section is user-selectable.
3. **Implement contributor** subclass returning a `ReportFragment` with `textBody`, `jsonBody`, `markdownBody`, `htmlBody`, and `csvRows` as needed.
4. **Register** in `registerReportingContributors()` or from your extension init hook via `ReportSectionRegistry::instance()`.
5. **Wire section order** in `core/reporting/report_section_renderer.cpp` (`kSectionOrder`).
6. **Test** in `core/reporting/tests/report_section_renderer_test.cpp` and e2e if CLI-visible.

## 8.3 Wire extension init

Pattern from `reporting.multi-format`:

```cpp
foundation::Result<bool> initializeReportingMultiFormat()
{
    reporting::registerReportingContributors(
        reporting::ReportSectionRegistry::instance());
    return foundation::Result<bool>(true);
}
```

Register the descriptor with this function as `initialize`.

---

# 9. Testing checklist

| Layer | Location | What to verify |
|-------|----------|----------------|
| Unit | `core/extension/tests/extension_manager_test.cpp` | Registration, config enable/disable, init failure isolation |
| Module | e.g. `core/reporting/tests/` | Contributor output per format |
| Integration | `tests/integration/` | Pipeline with extension disabled in config |
| E2E | `tests/end_to_end/cli_e2e_test.cpp` | `extensions list` / `describe` output |

Example config disable test pattern:

```properties
extensions.reporting.multi-format.enabled=false
```

---

# 10. Limitations (v1.4.x)

| Limitation | Workaround / future |
|------------|---------------------|
| No dynamic `.so`/`.dll` loading | Contribute built-ins in-tree; M12 adds shared libraries |
| No extension SDK or C ABI | C++ static registration only |
| No formal extension type enum | Use ID prefix convention |
| Single hook type (`ReportSectionContributor`) | Parser/search/storage hooks planned M12 |
| `ExtensionManager` not a runtime service bus | Init hooks register into core singletons |
| Third-party out-of-tree plugins | Not supported until M12 |

---

# 11. Roadmap: M12 dynamic plugins

From [Roadmap](../ROADMAP.md) and [Post-v1 Strategic Roadmap](../planning/POST_V1_STRATEGIC_ROADMAP.md):

| M12 target | Description |
|------------|-------------|
| Shared-library loading | `.so` (Linux/macOS), `.dll` (Windows) |
| Provider types | Parsers, report generators, search providers, storage backends |
| Extension SDK | Stable C ABI or documented C++ surface (ADR required) |
| Marketplace prep | Versioning, signing, sandboxing (later) |

**Guardrails:**

- Configuration and format profiles (M6.5) before dynamic parsers.
- ADR required before M12 implementation starts.
- CLI-first: extensions consume the same core APIs as built-ins.

When M12 lands, this guide will gain an out-of-tree plugin section. Built-in registration in `builtin_extensions.cpp` remains the pattern for first-party extensions.

---

# 12. Related documents

| Document | Purpose |
|----------|---------|
| [FR-004 – Extend LogScope](../requirements/functional/FR-004-Extend-LogScope.md) | Functional requirements |
| [M4 – Feature Expansion](../planning/M4-FEATURE-EXPANSION.md) | M4.4 extension ecosystem plan |
| [M8 – Advanced Reporting](../planning/M8-ADVANCED-REPORTING.md) | Report section contributor hook |
| [Component Catalog §C06](../architecture/COMPONENT_CATALOG.md) | Extension Manager responsibilities |
| [Configuration Guide](CONFIGURATION_GUIDE.md) | `extensions.*` keys |
| [CLI Reference](CLI_REFERENCE.md) | `extensions list` / `describe` |
| [core/extension/README.md](../../core/extension/README.md) | Module quick reference |

---

# 13. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 24-07-2026 | Initial Phase 1 plugin development guide (built-in extensions, M12 preview). |
