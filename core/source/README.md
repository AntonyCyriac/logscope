# Source Manager

## Purpose

The Source module acquires log data from supported sources and prepares it for analysis.

It implements **DO-001 Source Dataset** from the domain model.

## Components

| Component | Description |
|-----------|-------------|
| `LogSource` | Abstract interface for sequential log data access |
| `FileLogSource` | File-based `LogSource` implementation |
| `SourceDataset` | Prepared log data ready for the Analysis Engine |
| `SourceManager` | Source discovery, validation, and opening |

## Dependencies

- `scope_foundation` — `Path`, `FileSystem`, `Result<T>`
- `scope_runtime` — Diagnostics logging

## Usage

```cpp
#include "source.hpp"

scope::source::SourceManager manager;

auto dataset = manager.open(path);

if (dataset) {
    std::string line;

    while (true) {
        auto readResult = dataset->source().readLine(line);

        if (!readResult || !*readResult) {
            break;
        }

        // process line
    }
}
```

## CMake Target

- `scope_source` — static library
- `scope_source_tests` — unit tests
