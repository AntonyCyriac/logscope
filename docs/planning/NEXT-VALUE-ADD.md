# Next Value-Add Backlog

| Field | Value |
|-------|-------|
| Document | Next Value-Add Backlog |
| Category | Project Planning |
| Version | 1.5.0 |
| Status | Approved |
| Created | 30-07-2026 |
| Last Updated | 06-08-2026 |

---

# 1. Purpose

Prioritized **documented** work not yet shipped, derived from the planning corpus (roadmap, M15 completion, post-M15 investigation stories). Use this to sequence releases after [M15](M15-WEB-PLATFORM.md).

**Current public release:** `v2.6.1` (Phase A complete). **Next ship:** post–Story 4 backlog — versions TBD at G0 charter (see §5).

---

# 2. Recommended sequence

```text
v2.2.1  → shipped (M15.4 thin auth)
v2.2.2  → shipped (API key hashing)
v2.3.0  → shipped (Create an Investigation)
v2.4.0  → shipped (Understand Everything)
v2.5.0  → shipped (See What Happened)
v2.6.0  → shipped (Understand Why It Crashed)
v2.6.1  → shipped (hotfix #129)
Next    → post–Story 4 backlog (§5)
```

---

# 3. Immediate — next (`v2.2.2`)

| Item | Status |
|------|--------|
| API key hashing at rest | ✅ Shipped — `v2.2.2` |
| Migration from plaintext `web.api_key` config | ✅ Shipped — `v2.2.2` |

---

# 4. Near-term — investigation stories (`v2.3.0`–`v2.6.1`)

| Item | Target | Notes |
|------|--------|-------|
| Create an Investigation | `v2.3.0` | ✅ Shipped |
| Multi-source inputs | `v2.4.0` | ✅ Shipped |
| Unified timeline view | `v2.5.0` | ✅ Shipped |
| Crash analysis (basic) | `v2.6.0` | ✅ Shipped |
| Pstack fault-thread hotfix | `v2.6.1` | ✅ Shipped — [#129](https://github.com/AntonyCyriac/logscope/issues/129) |

---

# 5. Post–Story 4 backlog (ordered; versions TBD)

| # | Item | Notes |
|---|------|-------|
| 1 | Timeline crash events | `crash.summary` on timeline projection — deferred from v2.6.0 |
| 2 | Cross-log / multi-artifact correlation | Beyond single-source filters today |
| 3 | Desktop Timeline/Crash tab parity | Web has bottom dock tabs; desktop gap in [`tests/e2e/web/README.md`](../../tests/e2e/web/README.md) |
| 4 | ADR-010 domain model + events | Emergent after v2.6 — extract when patterns recur |

Tactical planning docs are added when each item is chartered for implementation (G0).

---

# 6. Shipped — M15 Web Platform (completed)

| Item | Status |
|------|--------|
| M15 REST API + browser MVP | ✅ `v2.1.0` |
| M15.3 shared workspaces, tail, async analyze | ✅ `v2.2.0` |
| M15.4 thin auth | ✅ `v2.2.1` |

---

# 7. Defer (documented non-goals)

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

# 8. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 30-07-2026 | Initial backlog from planning corpus audit. |
| 1.1.0 | 30-07-2026 | ADR-009 accepted; M15.0 planning complete. |
| 1.2.0 | 30-07-2026 | M15 ship track through v2.1.0. |
| 1.3.0 | 04-08-2026 | M15 complete at v2.2.1; v2.2.2 patch and investigation stories are active queue. |
| 1.4.0 | 04-08-2026 | Removed enterprise/cloud backlog rows; public horizon ends at investigation stories. |
| 1.5.0 | 06-08-2026 | Phase A shipped through v2.6.1; post–Story 4 backlog is active queue. |
