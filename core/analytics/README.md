# Analytics Engine

## Purpose

The Analytics module delivers higher-level analysis over indexed log lines: frequency tables, error clustering, timeline buckets, trend detection, and correlation findings.

## Components

| Component | Description |
|-----------|-------------|
| `AnalyticsEngine` | Orchestrates all analyzers |
| `FrequencyAnalyzer` | Top message and correlation-ID frequencies |
| `ErrorClusterer` | Groups errors by normalized signature |
| `TimelineAnalyzer` | Time-bucket histograms |
| `TrendAnalyzer` | Half-window rate comparison and spike detection |
| `CorrelationAnalyzer` | Repeated errors and shared correlation IDs |

## Dependencies

- `scope_analysis` — `AnalysisModel`, `LineIndex`, `FieldSummary`
- `scope_foundation` — `Timestamp`, string utilities
- `scope_runtime` — Diagnostics logging

## Usage

```cpp
#include "analytics.hpp"

scope::analytics::AnalyticsEngine engine;
scope::analytics::AnalyticsConfig config;
config.topN = 10U;

const scope::analytics::AnalyticsResult result = engine.analyze(model, config);
```

## Tests

`scope_analytics_tests` covers analyzers and the orchestration engine.
