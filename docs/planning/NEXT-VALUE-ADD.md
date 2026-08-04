# Next Value-Add Backlog

| Field | Value |
|-------|-------|
| Document | Next Value-Add Backlog |
| Category | Project Planning |
| Version | 1.4.0 |
| Status | Approved |
| Created | 30-07-2026 |
| Last Updated | 04-08-2026 |

---

# 1. Purpose

Prioritized **documented** work not yet shipped, derived from the planning corpus (roadmap, M15 completion, post-M15 investigation stories). Use this to sequence releases after [M15](M15-WEB-PLATFORM.md).

**Current public release:** `v2.2.1` (M15 complete). **Next ship:** **`v2.2.2`** — API key hashing at rest → **`v2.3.0`** — Create an Investigation.

---

# 2. Recommended sequence

```text
v2.2.1  → shipped (M15.4 thin auth)
v2.2.2  → API key hashing at rest (security patch)
v2.3.0  → Create an Investigation
v2.4.0+ → Multi-source, unified timeline, crash analysis stories
```

---

# 3. Immediate — next (`v2.2.2`)

| Item | Status |
|------|--------|
| API key hashing at rest | ⏳ Planned — [Securing logscope-web](../handbook/SECURING_LOGSCOPE_WEB.md) |
| Migration from plaintext `web.api_key` config | ⏳ Planned |

---

# 4. Near-term — investigation stories (`v2.3.0`–`v2.6.0`)

| Item | Target | Notes |
|------|--------|-------|
| Create an Investigation | `v2.3.0` | Portable incident container; artifacts; save/reopen |
| Multi-source inputs | `v2.4.0` | App + system logs, core, pstack in one investigation |
| Unified timeline view | `v2.5.0` | Chronological story inside investigation |
| Crash analysis (basic) | `v2.6.0` | Understand why it crashed |

Tactical planning docs are added when each investigation story is chartered for implementation.

---

# 5. Shipped — M15 Web Platform (completed)

| Item | Status |
|------|--------|
| M15 REST API + browser MVP | ✅ `v2.1.0` |
| M15.3 shared workspaces, tail, async analyze | ✅ `v2.2.0` |
| M15.4 thin auth | ✅ `v2.2.1` |

---

# 6. Defer (documented non-goals)

| Item | Why defer |
|------|-----------|
| Full RBAC / multi-tenant auth | Out of scope for current investigation releases |
| Hosted multi-tenant SaaS | Out of product scope |
| Plugin marketplace / `logscope install` | M12/M14 non-goals |
| CrashScope flagship program, playbooks | Folded into investigation stories where applicable |
| SIMD / zero-copy perf | Profile first |
| Distro packages (DEB/RPM/Homebrew) | GitHub Releases sufficient |
| PRD-001 lifecycle agents | Future vision |

---

# 7. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 30-07-2026 | Initial backlog from planning corpus audit. |
| 1.1.0 | 30-07-2026 | ADR-009 accepted; M15.0 planning complete. |
| 1.2.0 | 30-07-2026 | M15 ship track through v2.1.0. |
| 1.3.0 | 04-08-2026 | M15 complete at v2.2.1; v2.2.2 patch and investigation stories are active queue. |
| 1.4.0 | 04-08-2026 | Removed enterprise/cloud backlog rows; public horizon ends at investigation stories. |
