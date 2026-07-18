# Reporting Engine

## Purpose

The Reporting module generates user-facing reports from analysis results.

It implements **DO-003 Report** from the domain model.

## Components

| Component | Description |
|-----------|-------------|
| `Report` | Formatted presentation of analysis results |
| `ReportGenerator` | Creates reports from `AnalysisModel` |

## Dependencies

- `scope_foundation` — `Path`, `Result<T>`
- `scope_analysis` — `AnalysisModel`
- `scope_runtime` — Diagnostics logging

## Usage

```cpp
#include "reporting.hpp"

scope::reporting::ReportGenerator generator;
scope::reporting::Report report = generator.generate(model);

std::cout << report.text() << std::endl;
```

## CMake Target

- `scope_reporting` — static library
- `scope_reporting_tests` — unit tests
