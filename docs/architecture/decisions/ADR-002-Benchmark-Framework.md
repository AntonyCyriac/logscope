# ADR-002: Performance Benchmark Framework

- **Status:** Accepted
- **Date:** 18-07-2026

---

## Context

LogScope must satisfy NFR-001 §3.1 (Performance): characteristics must be measurable, regressions detectable, and improvements verifiable.

M5.1 requires a standard benchmark harness integrated with CMake and CI, consistent with ADR-001 (GoogleTest for unit tests).

---

## Decision

LogScope adopts **Google Benchmark** as the standard performance benchmark framework.

Benchmarks live under `tests/benchmarks/` and are built when `LOGSCOPE_BENCHMARKS=ON`.

Baseline results are stored in `tests/benchmarks/baseline.json` and documented in `docs/testing/PERFORMANCE.md`.

---

## Rationale

Google Benchmark:

- Integrates with CMake via `FetchContent` (same pattern as GoogleTest)
- Provides iteration timing, CPU time, and custom counters (e.g. lines processed)
- Is widely adopted in C++ projects
- Supports CI regression comparison via JSON export

---

## Consequences

### Positive

- Consistent performance measurement across modules
- Enables data-driven optimization in M5 and beyond
- Baseline JSON supports automated regression checks in CI

### Negative

- Additional FetchContent dependency at configure time
- Benchmarks are not run on every developer build by default (`LOGSCOPE_BENCHMARKS=OFF`)

---

## Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 18-07-2026 | Initial decision. |
