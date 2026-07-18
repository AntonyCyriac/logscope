# Roadmap

| Field | Value |
|-------|-------|
| Document | Roadmap |
| Category | Project Planning |
| Version | 2.1.0 |
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
M3  ███░░░░░░░ ~25%
```

Pre-M3 milestones are tagged at `v0.2.0-design-baseline`. M3 changes are tracked in [CHANGELOG.md](../CHANGELOG.md).

---

# M3 – Architecture Realization

The implementation phase is divided into logical increments.

| Phase | Component | Status |
|-------|-----------|--------|
| M3.1 | Repository Structure | 🟡 In Progress |
| M3.2 | Core Library (Foundation) | 🟡 In Progress |
| M3.3 | Platform Services | ⏳ Planned |
| M3.4 | Configuration Manager | ⏳ Planned |
| M3.5 | Source Manager | ⏳ Planned |
| M3.6 | Analysis Engine | ⏳ Planned |
| M3.7 | Investigation Engine | ⏳ Planned |
| M3.8 | Reporting Engine | ⏳ Planned |
| M3.9 | CLI | ⏳ Planned |
| M3.10 | Integration & End-to-End Testing | ⏳ Planned |

---

## M3.1 – Repository Structure

| Item | Status |
|------|--------|
| Workspace model directory layout | ✅ Complete |
| Core module structure (`core/foundation/`) | ✅ Complete |
| CI workflow (build and test) | ✅ Complete |
| CMake format target | ✅ Complete |
| Legacy `include/` + `src/` CLI separation | ⏳ Pending |
| Namespace validation across modules | ⏳ Pending |
| Target layout cleanup | ⏳ Pending |
| Final CMake organization | ⏳ Pending |

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

All completed components include unit tests and are built as part of `scope_foundation`.

### Next

| Component | Priority |
|-----------|----------|
| `Time` | High |
| `Date` | High |
| `DateTime` | High |
| `Duration` | Medium |
| `Timestamp` | Medium |
| `Clock` | Medium |
| `Stopwatch` | Medium |
| `Path` | High |
| `FileSystem` | High |
| `Random` | Medium |
| `String` utilities | Medium |
| `Hash` | Medium |

Foundation components are implemented incrementally using small pull requests with tests mirroring the public API.

---

## M3.3 – M3.10 (Planned)

Subsequent M3 phases depend on a stable Foundation library:

- **M3.3** – Platform Services (configuration, plugin manager, registry, diagnostics, threading)
- **M3.4** – Configuration Manager
- **M3.5** – Source Manager
- **M3.6** – Analysis Engine
- **M3.7** – Investigation Engine
- **M3.8** – Reporting Engine
- **M3.9** – CLI (command framework, subcommands, configuration, output formats)
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
