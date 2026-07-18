# Changelog

All notable changes to this project will be documented in this file.

This project follows the principles of [Keep a Changelog](https://keepachangelog.com/).

Pre-M3 history (M0–M2) is preserved in Git history, project documentation, and the `v0.2.0-design-baseline` tag. **Changelog entries begin at M3 – Architecture Realization.**

---

## [Unreleased]

### Added

#### M3.1 – Repository Architecture

- Introduced workspace model directory layout (`core/`, `docs/`, `apps/`, and related areas).
- Established continuous integration workflow (build and test on push/PR).
- Added CMake `format` target for `clang-format`.

#### M3.2 – Foundation Library

- `Status` type for lightweight operation outcomes.
- `Error` and `ErrorCode` types for structured error reporting.
- `Result<T>` template for type-safe, exception-free error handling.
- `Version` value type with semantic versioning support.
- `Uuid` value type with RFC 4122 parsing, version 4 generation, and comparison operators.
- `Time`, `Date`, and `DateTime` value types with parsing and comparison.
- `Duration` value type for non-negative time intervals with nanosecond precision.
- `Timestamp` value type for absolute UTC instants as Unix nanoseconds since epoch.
- `Clock` wall-clock source returning current UTC `Timestamp`.
- `Stopwatch` monotonic elapsed-time measurement returning `Duration`.
- `Random` seedable pseudo-random number generator.
- `String` utilities for trim, case conversion, prefix/suffix checks, and split.
- `Hash` FNV-1a hashing and hash-combine helpers.
- `Path` and `FileSystem` for filesystem path handling and file operations.
- Unit tests for all Foundation components (69 tests via GoogleTest).
- `scope_foundation` static library and `scope_foundation_tests` test target.

#### M3.8 – Reporting Engine

- `Report` and `ReportGenerator` for text reports from `AnalysisModel`.
- `scope_reporting` library and `scope_reporting_tests` test target.
- CLI `LogAnalyzer` refactored to render output via `ReportGenerator`.

#### M3.6 – Analysis Engine

- `AnalysisModel` and `AnalysisEngine` for transforming `SourceDataset` into analysis results.
- `scope_analysis` library and `scope_analysis_tests` test target.
- CLI `LogAnalyzer` refactored to use `AnalysisEngine` for log analysis.

#### M3.5 – Source Manager

- `LogSource`, `FileLogSource`, `SourceDataset`, and `SourceManager` for file-based log source acquisition.
- `scope_source` library and `scope_source_tests` test target.
- CLI `LogAnalyzer` refactored to read logs through `SourceManager`.

#### M3.4 – Configuration Manager

- `ConfigurationManager` for loading properties files, applying `SCOPE_` environment overrides, and validating required keys.
- `scope_configuration` library and `scope_configuration_tests` test target.
- CLI `--config` flag to load configuration from a properties file before analysis.

#### M3.3 – Platform Services (Runtime)

- `Configuration` key-value store.
- `Diagnostics` logging facility with level filtering, category tags, UTC timestamps, and `SCOPE_LOG_*` macros.
- `PluginRegistry` and `ServiceRegistry`.
- `scope_runtime` library and `scope_runtime_tests` test target.
- Runtime and CLI flow tracing via `Diagnostics` (`log.level` / `SCOPE_LOG_LEVEL`).

#### Applications

- CLI migrated from legacy `src/` + `include/` to `apps/cli/`.
- CLI linked to `scope_foundation` and `scope_runtime` using `scope::cli`, `scope::foundation`, and `scope::runtime` namespaces.

#### Documentation

- ADR-001: unit testing framework selection (GoogleTest).
- C++ Coding Standard and API Design Guidelines.
- Foundation Guidelines (renamed from engineering guidelines; moved under `docs/standards/`).
- Handbook workflow docs: Git Conventions, Pull Request Guide, Code Review Checklist.
- Updated Document Map, README, and cross-links across standards documents.

### Changed

- Migrated Foundation unit tests to GoogleTest.
- Reorganized engineering documentation into standards and handbook layers.
- Deduplicated C++ Coding Standard with Foundation Guidelines.
- Renamed `.github/README.md` to `.github/GITHUB.md` so the root README displays on the repository homepage.

---

## Prior Releases

| Tag | Scope |
|-----|-------|
| `v0.2.0-design-baseline` | M0–M2 complete: engineering foundation, product vision, and design baseline. |
