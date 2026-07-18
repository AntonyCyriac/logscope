# Roadmap

| Field | Value |
|-------|-------|
| Document | Roadmap |
| Category | Project Planning |
| Version | 2.2.0 |
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
| **M3 – Architecture Realization** | 🚧 In Progress | Implement the architecture defined during M2. |
| **M4 – Feature Expansion** | ⏳ Planned | Extend LogScope with additional capabilities while preserving architectural integrity. |
| **M5 – Production Readiness** | ⏳ Planned | Performance optimization, testing, documentation, packaging, and release preparation. |
| **v1.0.0** | 🎯 Target | First stable production release. |

---

# Current Progress

```
M0  ██████████ 100%
M1  ██████████ 100%
M2  ██████████ 100%
M3  ███████░░░ ~65%
```

Pre-M3 milestones are tagged at `v0.2.0-design-baseline`. M3 changes are tracked in [CHANGELOG.md](../CHANGELOG.md).

---

# M3 – Architecture Realization

The implementation phase is divided into logical increments.

| Phase | Component | Status |
|-------|-----------|--------|
| M3.1 | Repository Structure | 🟡 In Progress |
| M3.2 | Core Library (Foundation) | 🟡 In Progress |
| M3.3 | Platform Services (Runtime) | 🟡 In Progress |
| M3.4 | Configuration Manager | ✅ Complete |
| M3.5 | Source Manager | ✅ Complete |
| M3.6 | Analysis Engine | ✅ Complete |
| M3.7 | Investigation Engine | ✅ Complete |
| M3.8 | Reporting Engine | ✅ Complete |
| M3.9 | CLI | ✅ Complete |
| M3.10 | Integration & End-to-End Testing | ⏳ Planned |

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

## M3.10 (Planned)

Subsequent M3 phases depend on a stable Foundation library:

- **M3.10** – Integration and end-to-end testing

---

# M4 – Feature Expansion

The following capabilities are expected to be introduced incrementally after the architectural foundation is implemented.

Potential initiatives include:

- Extension ecosystem
- Additional source types
- Advanced reporting
- Session management
- AI-assisted investigation
- REST API
- Web interface

Priorities will be determined based on project goals and community feedback.

---

# M5 – Production Readiness

Focus areas include:

- Performance optimization
- Reliability improvements
- Comprehensive testing (integration, end-to-end, performance, fuzz)
- Documentation refinement
- Packaging and distribution
- Continuous Integration enhancements (static analysis, coverage, multi-platform builds, release workflow)
- Security review
- Release validation

Completion of this milestone prepares LogScope for the first stable production release.

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
