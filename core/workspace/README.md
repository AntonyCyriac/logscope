# Workspace

## Purpose

The Workspace module persists investigation sessions for progressive analysis workflows (FR-002.5).

## Components

| Component | Description |
|-----------|-------------|
| `InvestigationSession` | Saved analysis metadata, filters, and report preferences |
| `SessionSerializer` | Properties-style session file format |
| `SessionStore` | Save, load, and list `.logscope-session` files |

## Session file format

Sessions use a properties-style text file with keys such as `source.path`, `analysis.totalLines`, `filter.minErrors`, and `report.format`.

## Usage

```cpp
#include "workspace.hpp"

scope::workspace::InvestigationSession session =
    scope::workspace::InvestigationSession::fromAnalysis(model, lineFilter, levelFilter, "query", reportOptions, configFile);

scope::workspace::SessionStore store;
store.save(session, path);
```

## CMake Target

- `scope_workspace` — static library
- `scope_workspace_tests` — unit tests
