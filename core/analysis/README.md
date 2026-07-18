# Analysis Engine

## Purpose

The Analysis module transforms source datasets into analysis models.

It implements **DO-002 Analysis Model** from the domain model.

## Components

| Component | Description |
|-----------|-------------|
| `AnalysisModel` | Canonical representation of analyzed log information |
| `LogLevelCounts` | Per-level line statistics (INFO, WARN, ERROR) |
| `AnalysisEngine` | Orchestrates analysis of a `SourceDataset` |
| `detectLogLevel` | Generic log line level classification |

## Dependencies

- `scope_foundation` — `Path`, `Result<T>`
- `scope_source` — `SourceDataset`, `LogSource`
- `scope_runtime` — Diagnostics logging

## Usage

```cpp
#include "analysis.hpp"
#include "source.hpp"

scope::source::SourceManager sourceManager;
scope::analysis::AnalysisEngine engine;

auto dataset = sourceManager.open(path);

if (dataset) {
    auto model = engine.analyze(*dataset);

    if (model) {
        // model->totalLines(), model->sourcePath()
    }
}
```

## CMake Target

- `scope_analysis` — static library
- `scope_analysis_tests` — unit tests
