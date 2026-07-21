# Performance Baselines

| Field | Value |
|-------|-------|
| Document | Performance Baselines |
| Category | Testing |
| Version | 1.0.0 |
| Status | Approved |
| Created | 18-07-2026 |
| Last Updated | 18-07-2026 |

---

# Purpose

This document records performance baselines for LogScope per NFR-001 §3.1. Baselines support regression detection in CI and guide optimization work during M5.

---

# Running Benchmarks

```bash
cmake -S . -B build -DLOGSCOPE_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --target logscope_benchmarks
./build/tests/benchmarks/logscope_benchmarks --benchmark_format=json --benchmark_out=benchmark_results.json
```

On Windows (MinGW/MSVC), the executable is under `build\tests\benchmarks\`.

---

# Benchmark Suite

| Benchmark | Description |
|-----------|-------------|
| `BM_DetectLogLevel` | Single-line log level classification |
| `BM_DetectLogLevelMixed` | Rotating INFO/WARN/ERROR lines |
| `BM_AnalysisEngine` | Full analysis of a 100k-line synthetic log file |
| `BM_SourceReadLoop` | Source read loop without analysis |

Synthetic log fixtures are generated at benchmark startup and written to temporary files (not committed to the repository).

---

# Baseline Reference

Machine-readable baselines: [`tests/benchmarks/baseline.json`](../../tests/benchmarks/baseline.json).

| Metric | Baseline (reference) | Tolerance |
|--------|----------------------|-----------|
| `BM_AnalysisEngine` lines/sec | ≥ 500,000 | 20% regression warning in CI |
| `BM_DetectLogLevel` ns/op | ≤ 500 | Informational |
| `BM_SourceReadLoop` lines/sec | ≥ 1,000,000 | 20% regression warning in CI |

> Reference values were captured on a development workstation. CI uses relative regression checks rather than absolute thresholds across heterogeneous runners.

---

# Field Extraction Bounds (M6.3)

Analysis field extraction uses streaming summaries rather than indexing every line:

| Limit | Value | Location |
|-------|-------|----------|
| Message excerpt length | 120 characters | `maxMessageExcerptLength` in `field_summary.hpp` |
| Top message patterns reported | 10 | `FieldSummary::topMessages()` default |
| Unique message keys tracked | Unbounded hash map | Acceptable for typical log volumes; revisit if memory becomes an issue |

Timestamp min/max tracking is O(1) per line. `BM_AnalysisEngine` remains the primary regression guard for analysis throughput after field extraction.

---

# CI Integration

The `benchmark` job in `.github/workflows/ci.yml` runs benchmarks on Ubuntu (non-gating initially) and compares `BM_AnalysisEngine` throughput against `baseline.json` with a 20% tolerance.

---

# Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 18-07-2026 | Initial performance baseline documentation. |
