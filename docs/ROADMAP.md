# Roadmap

| Field | Value |
|-------|-------|
| Document | Roadmap |
| Category | Project Planning |
| Version | 2.9.0 |
| Status | Approved |
| Created | 15-07-2026 |
| Last Updated | 18-07-2026 |

---

# Purpose

This roadmap defines the planned evolution of LogScope from project inception through the first production release.

The roadmap is milestone-driven. Each milestone represents a stable engineering baseline before progressing to the next phase.

---

# Milestones

| Milestone | Status | Description |
|-----------|--------|-------------|
| **M0 – Engineering Foundation** | ✅ Complete | Repository setup, development environment, build system, coding standards, CI foundation. |
| **M1 – Product Vision** | ✅ Complete | Project Charter, Product Overview, product goals, and vision. |
| **M2 – Engineering Design** | ✅ Complete | Standards, Requirements, Architecture, and High-Level Design completed. |
| **M3 – Architecture Realization** | ✅ Complete | Implement the architecture defined during M2. |
| **M4 – Feature Expansion** | ✅ Complete | Extend LogScope with additional capabilities while preserving architectural integrity. |
| **M5 – Production Readiness** | ✅ Complete | Performance, reliability, CI, packaging, and release preparation for v1.0.0. |
| **v1.0.0** | ✅ Complete | First stable production release. |
| **M6 – Log Format Intelligence** | 🔄 Planned | Format detection, structured parsing, field extraction, content investigation, format profiles. Target: `v1.1.0`. |

---

# Current Progress

```
M0  ██████████ 100%
M1  ██████████ 100%
M2  ██████████ 100%
M3  ██████████ 100%
M4  ██████████ 100%
M5  ██████████ 100%
M6  ░░░░░░░░░░   0%  (planned)
```

Pre-M3 milestones are tagged at `v0.2.0-design-baseline`. M3 is released as [`v0.3.0`](../CHANGELOG.md). M4 is released as [`v0.4.0`](../CHANGELOG.md). **v1.0.0** is the first stable production release — see [Changelog](../CHANGELOG.md). **M6** targets [`v1.1.0`](../CHANGELOG.md).

---

# M3 – Architecture Realization

The implementation phase is divided into logical increments.

| Phase | Component | Status |
|-------|-----------|--------|
| M3.1 | Repository Structure | ✅ Complete |
| M3.2 | Core Library (Foundation) | ✅ Complete |
| M3.3 | Platform Services (Runtime) | ✅ Complete |
| M3.4 | Configuration Manager | ✅ Complete |
| M3.5 | Source Manager | ✅ Complete |
| M3.6 | Analysis Engine | ✅ Complete |
| M3.7 | Investigation Engine | ✅ Complete |
| M3.8 | Reporting Engine | ✅ Complete |
| M3.9 | CLI | ✅ Complete |
| M3.10 | Integration & End-to-End Testing | ✅ Complete |

---

## M3.1 – Repository Structure

| Item | Status |
|------|--------|
| Workspace model directory layout | ✅ Complete |
| Core module structure (`core/foundation/`) | ✅ Complete |
| CI workflow (build and test) | ✅ Complete |
| CMake format target | ✅ Complete |
| Legacy `include/` + `src/` CLI separation | ✅ Complete |
| CLI under `apps/cli/` linked to Foundation | ✅ Complete |
| Namespace validation across modules | ✅ Complete |
| Target layout cleanup | ✅ Complete |
| Final CMake organization | ✅ Complete |

---

## M3.2 – Foundation Library

The Foundation module provides universal technical capabilities shared by all components. It has no product-specific knowledge and no upstream dependencies.

### Completed

| Component | Description |
|-----------|-------------|
| `Status` | Lightweight operation outcome enumeration |
| `Error` / `ErrorCode` | Structured error reporting |
| `Result<T>` | Type-safe success/failure return type |
| `Version` | Semantic version value type |
| `Uuid` | RFC 4122 UUID (parse, generate, compare) |
| `Time` | Time-of-day value type |
| `Date` | Calendar date value type |
| `DateTime` | Combined date and time |
| `Duration` | Non-negative time interval with nanosecond precision |
| `Timestamp` | Absolute UTC instant as Unix nanoseconds since epoch |
| `Clock` | System wall-clock source for current UTC timestamps |
| `Stopwatch` | Monotonic elapsed-time measurement |
| `Random` | Seedable pseudo-random number generator |
| `String` utilities | Trim, case conversion, split, prefix/suffix checks |
| `Hash` | FNV-1a string hashing and hash-combine helpers |
| `Path` | Filesystem path value type |
| `FileSystem` | File existence and metadata operations |

All completed components include unit tests and are built as part of `scope_foundation`.

Foundation components are implemented incrementally using small pull requests with tests mirroring the public API.

---

## M3.3 – Platform Services (Runtime)

| Component | Description | Status |
|-----------|-------------|--------|
| `Configuration` | Key-value configuration store | Complete |
| `Diagnostics` | Level filtering, categories, macros, UTC timestamps, config integration | Complete |
| `PluginRegistry` | Plugin metadata registration | Complete |
| `ServiceRegistry` | Platform service registration | Complete |

Built as `scope_runtime` with unit tests. Depends on Foundation. CLI links `scope_runtime` for flow tracing.

### Next

- Thread pool and task scheduler
- Extended plugin loading
- Environment access
- Structured JSON log output

---

## M3.4 – Configuration Manager

| Component | Description | Status |
|-----------|-------------|--------|
| `ConfigurationManager` | Load properties files, apply `SCOPE_` environment overrides, validate required keys | Complete |

Built as `scope_configuration` with unit tests. Depends on Foundation and Runtime.

---

## M3.5 – Source Manager

| Component | Description | Status |
|-----------|-------------|--------|
| `LogSource` | Abstract interface for sequential log data access | Complete |
| `FileLogSource` | File-based log source implementation | Complete |
| `SourceDataset` | Prepared log data (DO-001) for analysis | Complete |
| `SourceManager` | Source validation and opening | Complete |

Built as `scope_source` with unit tests. CLI `LogAnalyzer` uses `SourceManager` for file input.

---

## M3.6 – Analysis Engine

| Component | Description | Status |
|-----------|-------------|--------|
| `AnalysisModel` | Canonical analysis result (DO-002) | Complete |
| `AnalysisEngine` | Transforms `SourceDataset` into `AnalysisModel` | Complete |

Built as `scope_analysis` with unit tests. CLI `LogAnalyzer` delegates analysis to `AnalysisEngine`.

---

## M3.8 – Reporting Engine

| Component | Description | Status |
|-----------|-------------|--------|
| `Report` | User-facing report presentation (DO-003) | Complete |
| `ReportGenerator` | Generates text reports from `AnalysisModel` | Complete |

Built as `scope_reporting` with unit tests. CLI `LogAnalyzer` renders output through `ReportGenerator`.

---

## M3.7 – Investigation Engine

| Component | Description | Status |
|-----------|-------------|--------|
| `InvestigationView` | Read-only inspection of analysis results | Complete |
| `LineCountFilter` | Progressive line-count filtering | Complete |
| `InvestigationEngine` | Search, filter, and inspect `AnalysisModel` | Complete |

Built as `scope_investigation` with unit tests. CLI `LogAnalyzer` inspects analysis results before reporting.

---

## M3.9 – CLI

| Component | Description | Status |
|-----------|-------------|--------|
| `CliApplication` | Command dispatcher and top-level usage | Complete |
| `analyze` | Analyze a log file with text or JSON output | Complete |
| `config validate` | Validate configuration files and required keys | Complete |

CLI supports subcommands while preserving legacy `logscope <log-file>` invocation. Includes `logscope_cli_tests`.

---

## M3.10 – Integration and End-to-End Testing

| Component | Description | Status |
|-----------|-------------|--------|
| `logscope_integration_tests` | Core pipeline tests across Source, Analysis, Investigation, Reporting, and Configuration | Complete |
| `logscope_e2e_tests` | CLI executable smoke tests using `samples/` fixtures | Complete |

Repository tests run through CTest with the project root as the working directory.

---

# M4 – Feature Expansion

M4 extends the M3 pipeline with meaningful analysis, richer sources, reporting, extensions, and session support. Phases follow the product evolution order defined in [Product Overview](vision/PRODUCT_OVERVIEW.md): built-in capability, configuration, plugin, then SDK (late M4).

Detailed planning: [M4 – Feature Expansion](planning/M4-FEATURE-EXPANSION.md).

| Phase | Focus | Primary FR | Status |
|-------|-------|------------|--------|
| M4.1 | Analysis depth | FR-001, FR-002 | ✅ Complete |
| M4.2 | Additional source types | FR-001 | ✅ Complete |
| M4.3 | Advanced reporting | FR-003 | ✅ Complete |
| M4.4 | Extension ecosystem | FR-004 | ✅ Complete |
| M4.5 | Session / workspace | FR-002 | ✅ Complete |
| Deferred | REST API, Web UI, AI-assisted investigation | — | Post-M4 |

---

## M4.1 – Analysis Depth

| Component | Description | Status |
|-----------|-------------|--------|
| `AnalysisModel` extensions | Per-level line counts and aggregate log statistics | Complete |
| `AnalysisEngine` | Generic pattern-based stats during line scan | Complete |
| `InvestigationEngine` | Level-based filters and content-aware search | Complete |
| Reporting / CLI | Surface new stats in text and JSON output | Complete |

**Goal:** Produce meaningful analysis results from plain-text logs (FR-001.3) without custom scripts.

**Acceptance:** `logscope analyze samples/sample.log` reports error and warning counts; investigation can filter by log level.

---

## M4.2 – Additional Source Types

| Component | Description | Status |
|-----------|-------------|--------|
| `StdinLogSource` | Pipe-friendly stdin input | Complete |
| Multi-file / directory dataset | Batch analysis across multiple files | Complete |
| Unsupported format feedback | Clear, actionable errors (FR-001.4) | Complete |

**Goal:** Broaden supported inputs without breaking the existing file workflow (FR-001.5).

**Acceptance:** `logscope analyze -` and directory input are documented and tested.

---

## M4.3 – Advanced Reporting

| Component | Description | Status |
|-----------|-------------|--------|
| Report sections | Summary, level breakdown, source metadata | Complete |
| Content selection | CLI or config-driven report sections (FR-003.2) | Complete |
| Additional formats | CSV and Markdown alongside text and JSON (FR-003.4) | Complete |

**Goal:** Selectable, reproducible reports in multiple formats (FR-003.2, FR-003.4, FR-003.6).

---

## M4.4 – Extension Ecosystem

| Component | Description | Status |
|-----------|-------------|--------|
| `ExtensionManager` | Extension discovery, registration, lifecycle (C06) | Complete |
| Built-in extensions | Static registration at startup; config enablement (FR-004.1) | Complete |
| CLI discoverability | `extensions list` and `extensions describe` (FR-004.5) | Complete |

**Goal:** Introduce capabilities without modifying core code (FR-004.2). Dynamic library loading is a later sub-phase.

---

## M4.5 – Session / Workspace

| Component | Description | Status |
|-----------|-------------|--------|
| `core/workspace/` | Persist investigation filters and report preferences | Complete |
| CLI session commands | Save and load investigation context (FR-002.5) | Complete |

**Goal:** Progressive investigation without restarting analysis.

---

## M4 – Deferred

The following remain out of scope for early M4:

- REST API
- Web interface
- AI-assisted investigation
- Performance and fuzz testing (see M5)

---

## M4 – Success Criteria

M4 is considered complete when:

- FR-001 acceptance criteria are met for supported plain-text log analysis.
- FR-002 supports content-aware search and progressive investigation.
- FR-003 supports selectable report content and multiple output formats.
- FR-004 provides discoverable, isolated extension mechanisms.
- All M4 phases are implemented with unit, integration, and end-to-end test coverage.

---

# M5 – Production Readiness

M5 prepares LogScope for the first stable production release (`v1.0.0`). Detailed planning: [M5 – Production Readiness](planning/M5-PRODUCTION-READINESS.md).

| Phase | Focus | Primary NFR | Status |
|-------|-------|-------------|--------|
| M5.1 | Performance baselines | §3.1, §3.7 | ✅ Complete |
| M5.2 | Reliability and fuzz testing | §3.2 | ✅ Complete |
| M5.3 | CI quality gates | §3.3, §3.6 | ✅ Complete |
| M5.4 | Packaging and release workflow | §3.6 | ✅ Complete |
| M5.5 | Documentation, security, v1.0.0 | §3.5 | ✅ Complete |

### Focus areas

- Performance optimization with measurable baselines
- Reliability improvements and fuzz testing
- Comprehensive testing (integration, end-to-end, performance, fuzz)
- Documentation refinement
- Packaging and distribution (source install, then binary GitHub Releases)
- Continuous Integration enhancements (static analysis, coverage, multi-platform builds, release workflow)
- Security review
- Release validation

Completion of this milestone prepares LogScope for the first stable production release.

---

# M6 – Log Format Intelligence

M6 deepens analysis beyond plain-text heuristics toward format-aware, field-rich workflows. Detailed planning: [M6 – Log Format Intelligence](planning/M6-LOG-FORMAT-INTELLIGENCE.md).

| Phase | Focus | Primary FR | Status |
|-------|-------|------------|--------|
| M6.1 | Format detection | FR-001.4 | ⏳ Planned |
| M6.2 | JSON Lines parsing | FR-001.1, FR-001.3 | ⏳ Planned |
| M6.3 | Timestamp and field extraction | FR-001.3 | ⏳ Planned |
| M6.4 | Content-aware investigation | FR-002.1, FR-002.2, FR-002.4 | ⏳ Planned |
| M6.5 | Format profiles and configuration | FR-004.1 | ⏳ Planned |

### Focus areas

- Generic format identification with actionable unsupported-input feedback
- Structured log parsing (JSON Lines / NDJSON first)
- Timestamp and field extraction for richer `AnalysisModel` output
- Content search, time-range filters, and basic correlation in investigation
- Configuration-driven format profiles (configuration before plugins)
- Target release: **`v1.1.0`**

### Deferred to post-M6

- REST API, Web UI, AI-assisted investigation
- Dynamic shared-library plugin loading and extension SDK (M7)
- Vendor-specific parsers

---

# Success Criteria

The roadmap is considered successful when:

- The architecture is fully implemented.
- All core capabilities defined in the functional requirements are operational.
- Quality attributes defined in NFR-001 are satisfied.
- The project is ready for community adoption and long-term maintenance.

---

# Revision History

| Version | Date | Description |
|----------|------------|-----------------------------|
| 2.0.0 | 15-07-2026 | Updated roadmap to align with the completed engineering design baseline and architecture realization plan. |
| 2.1.0 | 18-07-2026 | Updated M3 progress: UUID complete, Foundation status, M3.1/M3.2 tracking, and next priorities. |
| 2.2.0 | 18-07-2026 | Added Time, Date, DateTime, Path, FileSystem; Runtime module; CLI migration to apps/. |
| 2.3.0 | 18-07-2026 | M3 complete; all M3 phases marked complete; linked `v0.3.0` release. |
| 2.4.0 | 18-07-2026 | Added M4 phased roadmap (M4.1–M4.5), success criteria, and planning document link. |
| 2.5.0 | 18-07-2026 | M4 complete; all M4 phases marked complete; linked `v0.4.0` release. |
| 2.6.0 | 18-07-2026 | Added M5 phased roadmap (M5.1–M5.5) and planning document link. |
| 2.7.0 | 18-07-2026 | M5 complete; all M5 phases marked complete; production readiness infrastructure delivered. |
| 2.8.0 | 18-07-2026 | v1.0.0 first stable production release. |
| 2.9.0 | 18-07-2026 | Added M6 phased roadmap (M6.1–M6.5) and planning document link; target v1.1.0. |
