# Reporting Engine

## Purpose

The Reporting module generates user-facing reports from analysis results.

It implements **DO-003 Report** from the domain model.

## Components

| Component | Description |
|-----------|-------------|
| `Report` | Formatted presentation (text or binary PDF payload) |
| `ReportSections` | Selectable sections (`executive`, `summary`, `levels`, `errors`, `charts`, `metadata`, `formats`, `all`) |
| `ReportOptions` | Format, section selection, chart inclusion |
| `ReportSectionRegistry` | Built-in and contributed section renderers |
| `ReportFragment` | Per-format section bodies assembled by `FormatRenderer` |
| `ReportFormatter` | Orchestrates registry + format rendering |
| `ReportGenerator` | Creates reports from `AnalysisModel` |

## Output Formats

| Format | Description |
|--------|-------------|
| `text` | Human-readable sections (default) |
| `json` | Structured JSON section objects |
| `csv` | Section/key/value rows |
| `markdown` | Markdown tables and fenced ASCII charts |
| `html` | Self-contained HTML with embedded SVG charts |
| `pdf` | Native PDF via `MinimalPdfWriter` (ADR-003) |

## Dependencies

- `scope_foundation` — `Path`, `Result<T>`
- `scope_analysis` — `AnalysisModel`
- `scope_runtime` — Diagnostics logging

## Usage

```cpp
#include "reporting.hpp"

scope::reporting::ReportGenerator generator;
scope::reporting::ReportOptions options;
options.format = scope::reporting::ReportFormat::Html;
options.sections = scope::reporting::ReportSections::parse("executive,summary,charts").value();

scope::reporting::Report report = generator.generate(model, options);

if (report.isBinary()) {
    // write report.bytes() to a file
} else {
    std::cout << report.text() << std::endl;
}
```

## CMake Target

- `scope_reporting` — static library
- `scope_reporting_tests` — unit tests

Planning: [M8 – Advanced Reporting](../../docs/planning/M8-ADVANCED-REPORTING.md)
