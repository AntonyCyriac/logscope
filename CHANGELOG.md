# Changelog

All notable changes to this project will be documented in this file.

This project follows the principles of [Keep a Changelog](https://keepachangelog.com/).

Pre-M3 history (M0–M2) is preserved in Git history, project documentation, and the `v0.2.0-design-baseline` tag. **Changelog entries begin at M3 – Architecture Realization.**

---

## [Unreleased]

---

## [1.4.1] - 2026-07-24

M11 – Storage Layer complete (core scope). Introduces `scope_storage`, SQLite-backed hybrid indexing, session index reuse, and basic M10 query pushdown. 392 automated tests.

### Added

- M11.0 `ADR-005` storage architecture and `M11-STORAGE-LAYER.md`.
- M11.1–M11.3 `scope_storage` module: `IndexStore`, `SqliteIndexStore`, `IndexFingerprint`, `HybridIndexWriter`, `IndexReader`, `QueryPlanner`.
- M11.4 Session version `1.3` with `index.fingerprint`, `index.path`, `index.line_count`; fingerprint reuse on load.
- M11.5 `--persist-index`, `--reuse-index`, `--index-path` CLI flags; storage config keys; `BM_IndexStoreAppend`, `BM_QueryPushdown`.
- `scope_storage_tests` with SQLite CRUD, fingerprint, planner, and config coverage (25 cases).

### Changed

- `AnalysisEngine` spills indexed lines to SQLite when capacity is exceeded or persistence is requested.
- `InvestigationEngine` and analytics read through `IndexReader` with optional SQL pushdown.
- Session load reuses persisted indexes when the source fingerprint still matches.

### Deferred to v1.4.2

- zlib compression, query cache, incremental append indexing, `line_json_fields` table, FTS5.

---

## [1.4.0] - 2026-07-24

M10 – Query Language complete. Introduces `scope_query`, field-aware filter DSL, `--filter` on investigate/search, and `logscope query`. 365 automated tests.

### Added

- M10.0 `ADR-004` query DSL grammar and `M10-QUERY-LANGUAGE.md`.
- M10.1–M10.3 `scope_query` module with lexer, parser, `QueryEvaluator`, and unit tests.
- M10.4 Investigation integration, session `filter.expression`, and `query.saved.*` config validation.
- M10.5 `--filter` CLI flag, `logscope query` subcommand, e2e/matrix coverage, `BM_QueryEvaluator`.
- Bulk-log CLI matrix scripts and CI/release smoke coverage (from prior merge).

### Changed

- `InvestigationEngine::investigate()` applies DSL filters after M7 text search and legacy field filters.
- Release workflow runs bulk-log CLI matrix before publishing binaries.

---

## [1.3.1] - 2026-07-24

M9 – Analytics Engine complete. Introduces `scope_analytics`, `logscope analytics`, and analytics/timeline/clusters report sections with time-series charts. 337 automated tests.

### Added

- M9.1 `scope_analytics` module and `FrequencyAnalyzer`; `M9-ANALYTICS-ENGINE.md`.
- M9.2 `ErrorClusterer` with normalized message signatures.
- M9.3 `TimelineAnalyzer`, `TrendAnalyzer`, and timeline bucket sizing.
- M9.4 `CorrelationAnalyzer` refactor; `logscope analytics` CLI; analytics config keys.
- M9.5 Time-series charts, `analytics`/`timeline`/`clusters` report sections; `BM_AnalyticsEngine`.

### Changed

- `InvestigationEngine::findCorrelations` delegates to `CorrelationAnalyzer`.
- `Charts` report section includes timeline chart when timeline data is available.

---

## [1.3.0] - 2026-07-24

M8 – Advanced Reporting complete. Introduces section registry architecture, executive/error/chart sections, HTML and PDF formats, and `--output` file writing. 326 automated tests.

### Added

- M8.1 `ReportSectionRegistry`, `ReportFragment`, `FormatRenderer`; refactored `ReportFormatter`; `M8-ADVANCED-REPORTING.md`.
- M8.2 `executive` and `errors` report sections across text, JSON, CSV, and Markdown.
- M8.3 `chart_model`, ASCII and SVG level bar chart renderers; `charts` section.
- M8.4 `ReportFormat::Html`, self-contained HTML reports, CLI `--output <file>`.
- M8.5 ADR-003 minimal PDF writer, `ReportFormat::Pdf`, `ReportSectionContributor` hook, `formats` footer section.
- Config keys: `report.format`, `report.include_charts`, `report.template`.

### Changed

- `Report` supports binary PDF payloads via `bytes()` and `mimeType()`.
- `reporting.multi-format` extension registers report section contributor on init.

---

## [1.2.0] - 2026-07-21

M7 – Search Engine complete. Introduces a dedicated search subsystem with boolean queries, regex mode, CLI search workflow, and session search history. 318 automated tests.

### Added

- M7.1 `scope_search` module: `SearchQuery`, `SearchEngine`, `text_matcher`, investigation integration.
- M7.2 regex search mode with pattern validation and length limits.
- M7.3 boolean query parser (`AND`, `OR`, `NOT`, quoted terms, parentheses).
- M7.4 CLI `--query`, `--regex`, `--case-sensitive`; `logscope search` subcommand; investigation output metadata.
- M7.5 session serializer v1.2 (`search.query`, `search.history`); `search.saved.*` config keys; `BM_SearchEngine` benchmark.

### Changed

- `InvestigationEngine` delegates content matching to `SearchEngine`.
- `Configuration::keys()` added for search configuration validation.

---

## [1.1.0] - 2026-07-21

M6 – Log Format Intelligence complete. Extends analysis with format detection, JSON Lines parsing, field extraction, content-aware investigation, and configuration-driven format profiles. 296 automated tests.

### Added

- Post-v1 strategic roadmap (`POST_V1_STRATEGIC_ROADMAP.md`) with 10-phase vision, M6–M17 milestone mapping, and version targets.
- M7 Search Engine planning stub (`M7-SEARCH-ENGINE.md`).
- M6.1 format detection: `LogFormat`, `FormatDetector`, report metadata `format`, and CLI `--log-format auto|plain|jsonl`.
- M6.2 JSON Lines parsing: per-line JSON validation, `JsonLinesSummary` stats (valid lines, parse failures, top-level keys), field-aware level mapping, and `samples/sample.jsonl`.
- M6.3 field extraction: `FieldSummary` with time range and top message patterns; plain-text timestamp prefixes; JSON `timestamp`/`message` fields (hand-rolled parser extended; nlohmann/json deferred).
- M6.4 content-aware investigation: bounded `LineIndex`, `searchContent`, `TimeRangeFilter`, `FieldFilter`, correlation summary, `investigate` CLI subcommand, and session serializer v1.1 content filter persistence.
- M6.5 format profiles: `AnalysisConfig`, built-in profiles (`generic-plain`, `generic-json`), configuration keys (`source.format`, `source.json.timestamp_field`, `source.json.level_field`, `investigation.max_indexed_lines`), CLI `--profile`, `config validate` extensions, and `FormatParser` interface.

### Fixed

- Analyze failures now print actionable error messages to stderr (including unsupported binary input).
- Explicit `Error` copy/move operations to silence clang-analyzer false positives in `Result<T>` error propagation.

---

## [1.0.0] - 2026-07-18

First stable production release. Delivers the complete M0–M5 roadmap: core pipeline, feature expansion, and production readiness with 246 automated tests.

### Added

#### M5 – Production Readiness

- M5 planning document and phased roadmap (M5.1–M5.5).
- Google Benchmark harness under `tests/benchmarks/` with baseline JSON and `PERFORMANCE.md`.
- ADR-002: benchmark framework selection.
- Malformed-input unit tests for session serializer, configuration, and CLI parser.
- libFuzzer targets for session serializer and configuration (`LOGSCOPE_FUZZING=ON`).
- Sanitizer build preset (`LOGSCOPE_SANITIZE=ON`) and `clang-tidy` target.
- Multi-OS CI matrix (Ubuntu, Windows, macOS), coverage, benchmark, fuzz, and tidy jobs.
- CMake `install()` and CPack packaging for source distributions.
- Tag-triggered release workflow with per-OS binary artifacts.
- Documentation: `TESTING.md`, `RELEASE.md`, `V1_VALIDATION.md`, `SECURITY_REVIEW.md`, `CLI_REFERENCE.md`.

### Changed

- `SessionSerializer::deserialize` uses safe numeric parsing (`std::from_chars`) instead of throwing `std::stoull`.
- CI expanded from single Ubuntu job to full M5 quality gate pipeline.
- `InvestigationSession` default constructor explicitly deleted.

### Fixed

- Session deserialization no longer throws on invalid numeric fields.
- CMake CMP0135 `FetchContent` timestamp warnings (`DOWNLOAD_EXTRACT_TIMESTAMP`).
- Duplicate static library linkage in integration test target.

---

## [0.4.0] - 2026-07-18

M4 – Feature Expansion complete. Extends the M3 pipeline with analysis depth, richer sources, advanced reporting, extensions, and session persistence, with 230 automated tests.

### Added

#### M4.1 – Analysis Depth

- `LogLevelCounts` and `detectLogLevel` for generic INFO/WARN/ERROR statistics.
- `LogLevelFilter` for investigation filtering by error and warning thresholds.
- Text and JSON reports include per-level line counts.

#### M4.2 – Additional Source Types

- `StdinLogSource` for pipe-friendly workflows (`logscope analyze -`).
- `CompositeLogSource` and directory support for multi-file datasets.
- `FileSystem::listRegularFiles` for directory enumeration.
- Clear unsupported-input errors for empty directories and invalid sources (FR-001.4).

#### M4.3 – Advanced Reporting

- Report sections: summary, level breakdown, and source metadata.
- Section selection via `--sections` CLI flag or `report.sections` configuration key (FR-003.2).
- CSV and Markdown output formats alongside text and JSON (FR-003.4).
- Unified formatting in `core/reporting` for reproducible output (FR-003.6).

#### M4.4 – Extension Ecosystem

- `ExtensionManager` for built-in extension registration and lifecycle (C06).
- Configuration-based enablement via `extensions.<id>.enabled` keys (FR-004.1).
- CLI commands `extensions list` and `extensions describe` (FR-004.5).
- Failure-isolated extension initialization during analyze (FR-004.4).

#### M4.5 – Session / Workspace

- `core/workspace` module with `InvestigationSession` and `SessionStore`.
- Persist analysis metadata, investigation filters, search query, and report preferences.
- CLI commands `session save`, `session load`, and `session list` (FR-002.5).

---

## [0.3.0] - 2026-07-18

M3 – Architecture Realization complete. Delivers the full core pipeline from configuration through CLI, with 186 automated tests.

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

#### M3.10 – Integration and End-to-End Testing

- `logscope_integration_tests` for the Source → Analysis → Investigation → Reporting pipeline.
- `logscope_e2e_tests` for CLI executable smoke tests against `samples/` fixtures.
- Repository `tests/` directory wired into the root CMake build.

#### M3.9 – CLI

- `CliApplication`, `analyze`, and `config validate` subcommands with text and JSON output formats.
- Legacy `logscope <log-file>` invocation preserved for backward compatibility.
- `logscope_cli_tests` unit test target for CLI parsing and output formatting.

#### M3.7 – Investigation Engine

- `InvestigationView`, `LineCountFilter`, and `InvestigationEngine` for inspecting, filtering, and searching analysis models.
- `scope_investigation` library and `scope_investigation_tests` test target.
- CLI `LogAnalyzer` logs an investigation summary before report generation.

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
| `v1.0.0` | First stable release: M5 production readiness, multi-OS CI, packaging, and distribution. |
| `v0.4.0` | M4 complete: analysis depth, additional sources, advanced reporting, extensions, and session persistence. |
| `v0.3.0` | M3 complete: core pipeline, CLI framework, integration and end-to-end tests. |
| `v0.2.0-design-baseline` | M0–M2 complete: engineering foundation, product vision, and design baseline. |
