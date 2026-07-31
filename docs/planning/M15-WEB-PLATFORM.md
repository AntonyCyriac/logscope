# M15 – Web Platform

| Field | Value |
|-------|-------|
| Document | M15 – Web Platform |
| Category | Project Planning |
| Version | 0.3.0 |
| Status | M15.2 complete — `v2.1.0` |
| Created | 30-07-2026 |
| Last Updated | 31-07-2026 |

---

# 1. Purpose

Deliver **M15 – Web Platform**: browser UI and **REST API** over existing investigation surfaces, reusing `ApplicationService` (ADR-008) without duplicating domain logic in the web tier.

Strategic phase: [Post-v1 Strategic Roadmap § Phase 7](POST_V1_STRATEGIC_ROADMAP.md).

**Target track:** `v2.1.0` (follow-on to M14 desktop at `v2.0.x`; see [v2.0.0 release notes](../release/v2.0.0-RELEASE-NOTES.md)).

---

# 2. Dependencies

| Prior | M15 dependency |
|-------|----------------|
| M14 + `ApplicationService` | Shared orchestration for CLI and desktop |
| ADR-008 | Desktop proved GUI consumes application layer; web is next presentation |
| `v2.0.4` (M14.12 Phase C) | Desktop CLI parity closed before new surface |
| **[ADR-009](../architecture/decisions/ADR-009-Web-Platform-REST.md)** | HTTP server, API shape, auth boundary, test strategy — **M15.0 gate satisfied** |

---

# 3. Scope (in)

| Area | Description |
|------|-------------|
| REST API | Open/analyze/investigate, sessions, export, extensions list, agent investigate |
| Browser MVP | Open log, table, filters, analytics, export — parity with desktop workbench |
| Remote log input | Upload or server-side path (design in ADR-009) |
| Shared investigations | Server-persisted workspaces (slice after MVP) |

---

# 4. Non-goals (M15)

| Item | Notes |
|------|-------|
| Full RBAC / multi-tenant enterprise | M16 |
| K8s / Helm / gRPC / OpenTelemetry | M17 |
| Plugin marketplace install UX | M12 non-goal |
| QML or native desktop changes | M14 complete |
| Replacing CLI for scripting | CLI remains primary automation surface |

---

# 5. Phased delivery (proposed)

| Phase | Focus | Target |
|-------|-------|--------|
| M15.0 | ADR-009 + scenario matrix + API sketch | ✅ Planning complete |
| M15.1 | REST service over `ApplicationService` | ✅ `v2.1.0` |
| M15.2 | Browser MVP (investigate workflow) | ✅ `v2.1.0` |
| M15.3 | Shared investigations / saved workspaces API | `v2.1.x` |
| M15.4 | Thin auth (optional; or defer to M16) | TBD |

---

# 6. Acceptance (M15.2 MVP)

| Scenario | Expected |
|----------|----------|
| Open + analyze | REST or UI opens `sample.log`, returns line counts |
| Investigate | Filter bar equivalent returns matching rows |
| Export | Report sections match CLI/desktop export options |
| Session | Save/load session via API |
| Plugins + AI | Existing `.properties` providers work through service layer |

---

# 7. Related documents

| Document | Role |
|----------|------|
| [ADR-008 Desktop Qt](../architecture/decisions/ADR-008-Desktop-Qt-Presentation.md) | ApplicationService graduation path |
| [ADR-009 Web REST](../architecture/decisions/ADR-009-Web-Platform-REST.md) | HTTP server, API contract, auth, test strategy |
| [M15-V210-WEB-SCENARIOS.md](M15-V210-WEB-SCENARIOS.md) | v2.1.0 acceptance scenario matrix |
| [NEXT-VALUE-ADD.md](NEXT-VALUE-ADD.md) | Prioritized backlog before/during M15 |
| [ROADMAP.md](../ROADMAP.md) | Milestone status |

---

# 8. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 0.1.0 | 30-07-2026 | Initial draft; graduation stub before ADR-009. |
| 0.2.0 | 30-07-2026 | ADR-009 accepted; M15.0 gate satisfied; scenario matrix linked. |
| 0.3.0 | 31-07-2026 | M15.1 + M15.2 shipped in `v2.1.0` (`logscope-web` + SPA). |
