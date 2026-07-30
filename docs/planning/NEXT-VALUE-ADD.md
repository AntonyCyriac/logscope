# Next Value-Add Backlog

| Field | Value |
|-------|-------|
| Document | Next Value-Add Backlog |
| Category | Project Planning |
| Version | 1.1.0 |
| Status | Approved |
| Created | 30-07-2026 |
| Last Updated | 30-07-2026 |

---

# 1. Purpose

Prioritized **documented** work not yet shipped, derived from the full planning corpus audit (roadmap, M14 gaps, Phase 1, M11 scenarios, strategy themes). Use this to sequence releases before and during [M15](M15-WEB-PLATFORM.md).

**Current public release:** `v2.0.3`. **In tree (pre-release):** M14.12 Phase C → **`v2.0.4` track**.

---

# 2. Recommended sequence

```text
v2.0.4  → M14.12 Phase C + desktop tests + large sample + doc sync
v2.0.x  → Hotfixes, signing, handbook sweep
v2.1.0  → M15 REST API + browser MVP
v2.2.0+ → Shared investigations, thin auth (or M16)
```

---

# 3. Immediate — `v2.0.4` (high value, low effort)

| Item | Source | Notes |
|------|--------|-------|
| Ship M14.12 Phase C | [M14-DESKTOP-CLI-PARITY-GAPS.md](M14-DESKTOP-CLI-PARITY-GAPS.md) | Config editor, format/profile, session save, clipboard/stdin |
| `logscope_desktop_tests` (14 cases) | Desktop CI | Regression guard after v2.0.3 |
| `samples/large-app.log` | [samples/README.md](../../samples/README.md) | Persist-index / performance fixture |
| Desktop user docs | [USER_MANUAL.md](../handbook/USER_MANUAL.md) gap | CLI-only today; tutorial `04-desktop` proposed |
| Doc version sync | README, PRODUCT.md, handbook | Credibility |

---

# 4. Short-term — M15 kickoff (high value)

| Item | Source | Notes |
|------|--------|-------|
| ~~**ADR-009** Web/REST~~ | Graduation gate | ✅ Accepted — [ADR-009](../architecture/decisions/ADR-009-Web-Platform-REST.md) |
| [M15-WEB-PLATFORM.md](M15-WEB-PLATFORM.md) | ROADMAP | M15.0 gate satisfied; M15.1 implementation next |
| REST over `ApplicationService` | ADR-008 | M15.1 — core deliverable |
| Browser MVP | Strategic roadmap Phase 7 | M15.2 |
| Strategy `sync/v2.0.4` | RELEASE.md §8 | After public tag |

---

# 5. Medium value — optional before M15 code

| Item | Source | Effort |
|------|--------|--------|
| CLI `--follow` / tail | ADR-008 follow-up | M |
| M11 scenarios S4.8, S2.5, S2.7 | [M11-V143-STORAGE-SCENARIOS.md](M11-V143-STORAGE-SCENARIOS.md) | S–M |
| Win/mac release signing | RELEASE.md | Ops |
| GitHub Pages API docs | [api/README.md](../api/README.md) | S |
| Thread-safe diagnostics | strategy `ideas/` | S — before concurrent web |

---

# 6. Defer (documented non-goals / future-private)

| Item | Why defer |
|------|-----------|
| M16 Enterprise (RBAC, agents, streaming) | Product boundary |
| M17 Cloud (K8s, Helm, gRPC) | After M15 REST |
| Plugin marketplace / `logscope install` | M12/M14 non-goals |
| CrashScope, playbooks, multi-source workspace | Strategy future-private |
| SIMD / zero-copy perf | Phase 1 deferred; profile first |
| Distro packages (DEB/RPM/Homebrew) | GitHub Releases sufficient |
| PRD-001 lifecycle agents | Future vision |

---

# 7. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 30-07-2026 | Initial backlog from planning corpus audit. |
| 1.1.0 | 30-07-2026 | ADR-009 accepted; M15.0 planning gate complete. |
