# Investigation Engine

## Purpose

The Investigation module explores and queries analysis results without re-running analysis.

It implements **FR-002 Investigate Logs** by operating directly on **DO-002 Analysis Model**.

## Components

| Component | Description |
|-----------|-------------|
| `InvestigationView` | Read-only inspection of analyzed log information |
| `LineCountFilter` | Progressive line-count filtering |
| `LogLevelFilter` | Progressive log-level count filtering |
| `InvestigationEngine` | Search, filter, and inspect analysis models (delegates content search to `SearchEngine`) |

## Dependencies

- `scope_foundation` — `Path`, `String` utilities
- `scope_analysis` — `AnalysisModel`
- `scope_runtime` — Diagnostics logging

## Usage

```cpp
#include "investigation.hpp"

scope::investigation::InvestigationEngine engine;
const auto view = engine.inspect(model);

if (engine.matches(model, scope::investigation::LineCountFilter::nonEmpty()) &&
    engine.searchSource(model, "error")) {
    // model->summary()
}
```

## CMake Target

- `scope_investigation` — static library
- `scope_investigation_tests` — unit tests
