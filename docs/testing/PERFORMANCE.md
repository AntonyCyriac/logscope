# Performance Baselines

| Field | Value |
|-------|-------|
| Document | Performance Baselines |
| Category | Testing |
| Version | 1.4.0 |
| Status | Approved |
| Created | 18-07-2026 |
| Last Updated | 24-07-2026 |

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
| `BM_IndexStoreAppend` | SQLite index append throughput (`scope_storage`; 1k and 100k line args) |
| `BM_QueryPushdown` | Filter evaluation against a persisted index |

Synthetic log fixtures are generated at benchmark startup and written to temporary files (not committed to the repository).

---

# Baseline Reference

Machine-readable baselines: [`tests/benchmarks/baseline.json`](../../tests/benchmarks/baseline.json).

| Metric | Baseline (reference) | Tolerance |
|--------|----------------------|-----------|
| `BM_AnalysisEngine/100000` lines/sec | ≥ 500,000 | 20% regression warning in CI |
| `BM_DetectLogLevel` ns/op | ≤ 500 | Informational |
| `BM_SourceReadLoop/100000` lines/sec | ≥ 1,000,000 | 20% regression warning in CI |
| `BM_IndexStoreAppend/100000` lines/sec | ≥ 50,000 | 20% regression warning in CI |

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

# Investigation Index Bounds (M6.4)

Content-aware investigation stores a bounded per-line index during analysis:

| Limit | Value | Location |
|-------|-------|----------|
| Indexed lines per source | 10,000 (default; configurable via `investigation.max_indexed_lines`, max 1,000,000) | `maxIndexedLines` in `line_index.hpp`; `AnalysisConfig` |
| Line content excerpt length | 256 characters | `maxLineContentExcerptLength` in `line_index.hpp` |

Lines beyond the index cap are counted in `truncatedLineCount` but are not searchable without re-reading the source. Session files persist content filter state (serializer v1.2) rather than the full index; `session load` re-analyzes the source when content filters are active.

---

# Search Benchmarks (M7)

| Benchmark | Description |
|-----------|-------------|
| `BM_SearchEngineTextQuery` | Text search over 10,000 indexed synthetic lines |

Target: sub-second search on indexed fixtures on a development workstation.

---

# CI Integration

The `benchmark` job in `.github/workflows/ci.yml` runs benchmarks on Ubuntu and compares `BM_AnalysisEngine/100000`, `BM_SourceReadLoop/100000`, and `BM_IndexStoreAppend/100000` throughput against `baseline.json` with a 20% tolerance.

---

# Storage Benchmarks (M11 / v1.4.2)

`BM_IndexStoreAppend` exercises `SqliteIndexStore` batched writes (WAL, prepared `INSERT`, 5,000-line transaction batches). Added in **v1.4.2** with a CI regression gate at the 100k-line argument size.

`BM_QueryPushdown` measures filter evaluation against a persisted index after append — informational; not gated in `baseline.json`.

---

# Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 18-07-2026 | Initial performance baseline documentation. |
| 1.1.0 | 21-07-2026 | Documented configurable investigation index cap (M6.5). |
| 1.2.0 | 21-07-2026 | Added search engine benchmark (M7). |
| 1.3.0 | 24-07-2026 | Fixed deprecated `DoNotOptimize` usage in analysis benchmark (M8 housekeeping). |
| 1.4.0 | 24-07-2026 | Added `BM_IndexStoreAppend/100000` baseline and storage benchmark section (`v1.4.2`). |
