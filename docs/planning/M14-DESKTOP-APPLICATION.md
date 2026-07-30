# M14 – Desktop Application

| Field | Value |
|-------|-------|
| Document | M14 – Desktop Application |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | Complete |
| Created | 30-07-2026 |
| Last Updated | 30-07-2026 |

---

# 1. Purpose

Deliver **M14 – Desktop Application** at **`v2.0.0`**: Qt Widgets GUI over existing investigation, reporting, analytics, session, extension, and AI surfaces — plus live tail.

See [ADR-008](../architecture/decisions/ADR-008-Desktop-Qt-Presentation.md) and [M14 v2.0.0 Scenarios](M14-V200-DESKTOP-SCENARIOS.md).

---

# 2. Dependencies

| Prior milestone | M14 dependency |
|-----------------|----------------|
| M3–M5 | Core pipeline, sessions, reporting |
| M7–M10 | Search, query DSL, investigation filters |
| M9 | Analytics panels |
| M11 | Persist-index / reuse-index in UI |
| M12 | Extensions panel |
| M13 | AI assistant panel |
| Phase 1 (`v1.5.2`) | `--stats`, regression baseline, docs |

---

# 3. Phased Delivery

| Phase | Focus | Status |
|-------|-------|--------|
| M14.0 | ADR-008, planning doc, scenarios | ✅ Complete |
| M14.1 | `ApplicationService` + CLI refactor | ✅ Complete |
| M14.2 | Qt shell: open, log table, analyze | ✅ Complete |
| M14.3 | Investigation filter bar | ✅ Complete |
| M14.4 | Analytics tabs | ✅ Complete |
| M14.5 | Report export + sections | ✅ Complete |
| M14.6 | Session sidebar | ✅ Complete |
| M14.7 | Extensions + config editor | ✅ Complete |
| M14.8 | AI assistant panel | ✅ Complete |
| M14.9 | Live tail | ✅ Complete |
| M14.10 | Themes, shortcuts, status stats | ✅ Complete |
| M14.11 | CI smoke, doc sync, `v2.0.0` / `v2.0.1` release | ✅ Complete |
| M14.12 | Desktop CLI parity polish (Phase A–C complete; see [M14-DESKTOP-CLI-PARITY-GAPS.md](M14-DESKTOP-CLI-PARITY-GAPS.md)) | ✅ Complete (`v2.0.4`; hotfix `v2.0.5`) |

---

# 4. Deliverables

## Application layer (`apps/common/`)

- `ApplicationService` — shared orchestration (see ADR-008)
- Unit tests without Qt

## Source tail (`core/source/`)

- `TailingFileLogSource`, `OpenOptions::follow`
- `SourceManager::open(path, options)`

## Desktop (`apps/desktop/`)

- `logscope-desktop` — Qt6 Widgets main window
- Navigator: open, recent, sessions, plugins
- Filter bar: search, query, DSL, level, time range
- Virtualized log table
- Analytics tabs: timeline, frequency, clusters, correlations
- Report export dialog (formats + sections)
- Session save/load/list
- Extensions list/describe + settings dialog
- AI panel: ask, summarize, hints
- Tail toggle + worker thread
- Dark/light themes, status bar with `--stats` parity

**Follow-on:** [M14-DESKTOP-CLI-PARITY-GAPS.md](M14-DESKTOP-CLI-PARITY-GAPS.md) — prioritized CLI surface not yet in the GUI (config editor, sections, persist-index, time-range wiring).

## CLI

- Refactor to use `ApplicationService` where practical (behaviour-preserving)

## CI

- Desktop build job on Ubuntu with `LOGSCOPE_DESKTOP=ON`
- Release workflow: `logscope-desktop` artifacts for Linux, Windows, and macOS

---

# 5. Non-goals

- M15 Web UI, REST API
- CrashScope, multi-source workspaces, playbooks
- Plugin marketplace / install UX
- Autonomous AI actions
- QML UI
- UI-specific logic in `core/`

---

# 6. FR mapping

| Capability | M14 implementation |
|------------|-------------------|
| FR-001 Analyze | Analyze toolbar + export |
| FR-002 Investigate | Filter bar + log table |
| FR-003 Reports | Export menu + section picker |
| FR-004 Extend | Extensions panel + config |
