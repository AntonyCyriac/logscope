# ADR-008: Desktop Qt Presentation Layer

- **Status:** Accepted
- **Date:** 30-07-2026

---

## Context

M0–M13 delivered a CLI-only presentation layer ([COMPONENT_CATALOG.md](../COMPONENT_CATALOG.md) C09). **M14 (`v2.0.0`)** adds a Qt Widgets desktop application that exposes the same investigation, reporting, analytics, session, extension, and AI workflows without duplicating domain logic in the GUI.

Requirements:

- GUI consumes **core** and **application orchestration** APIs; no Qt in `core/`.
- CLI behaviour remains unchanged (parity tests).
- Live tail is a **new source capability** (not available in CLI v1; desktop primary UX).
- Threading: long-running analyze/index/tail work off the UI thread.
- Offline-first: default config unchanged; AI optional.

---

## Decision

### 1. Presentation components

| ID | Component | Location |
|----|-----------|----------|
| C09 | CLI | `apps/cli/` (existing) |
| C10 | Application orchestration | `apps/common/` — `scope_application` |
| C11 | Desktop (Qt Widgets) | `apps/desktop/` — `logscope-desktop` |

### 2. Qt6 Widgets only

- **Qt6** `Widgets` module; **no QML** in M14.
- CMake option `LOGSCOPE_DESKTOP=ON` (default OFF for minimal CI builds).
- `find_package(Qt6 REQUIRED COMPONENTS Widgets)`.
- Long work on `QThread` workers; UI updates via signals/slots.
- Log table: `QAbstractTableModel` + `QTableView` (virtualized).

### 3. ApplicationService (`apps/common/`)

Thin orchestration layer shared by CLI and desktop:

- Configuration load (`ConfigurationManager`)
- `openSource(path, OpenOptions{follow})`
- `analyze`, `investigate`, `analytics`, `exportReport`
- `saveSession`, `loadSession`, `listSessions`
- `listExtensions`, `describeExtension`
- `agentInvestigate` (ask / summarize / hints)
- Returns domain types (`AnalysisModel`, `InvestigationResult`, `AnalyticsResult`, `Report`) — not formatted stdout strings.

CLI commands refactor to call `ApplicationService` internally where practical; output formatting stays in CLI.

### 4. Live tail

- `TailingFileLogSource` in `core/source/` — on EOF with `follow=true`, poll file growth and read appended bytes.
- `SourceManager::open(path, OpenOptions)` passes follow flag.
- Desktop `TailWorker` reads tail lines on a worker thread; appends to in-memory line list and notifies table model.
- Full re-analyze available on demand; tail shows raw appended lines between analyze passes.
- Document: tail stops on explicit user action; file rotation behaviour documented in M14 scenarios.

### 5. Themes

- Light and dark palettes via `QPalette` + `QStyleFactory`.
- Default: follow OS / Fusion style.
- View menu toggle; no custom branding in M14 v1.

### 6. Charts

- Analytics panels use `QPainter` bar/timeline widgets fed from `AnalyticsResult` — no new chart framework.
- Report HTML/PDF export reuses existing reporting engine.

---

## Consequences

### Positive

- Single pipeline for CLI and desktop (parity, less drift).
- Clear graduation path for M15 Web (reuse `ApplicationService` behind HTTP later); formalized in [ADR-009](ADR-009-Web-Platform-REST.md) (accepted; web presentation **C12**).
- Qt Widgets matches C++ codebase and ships faster than QML.

### Negative

- Qt dependency increases build/CI complexity on Linux (Qt6 dev packages).
- Tail incremental analysis is UI-oriented first; CLI `--follow` is optional follow-up.

---

## References

- [M14-DESKTOP-APPLICATION.md](../../planning/M14-DESKTOP-APPLICATION.md)
- [M14-V200-DESKTOP-SCENARIOS.md](../../planning/M14-V200-DESKTOP-SCENARIOS.md)
- [ADR-009 Web Platform REST](ADR-009-Web-Platform-REST.md)
- [POST_V1_STRATEGIC_ROADMAP.md](../../planning/POST_V1_STRATEGIC_ROADMAP.md) § Phase 6
