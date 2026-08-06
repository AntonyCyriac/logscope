# Next Value-Add Backlog

| Field | Value |
|-------|-------|
| Document | Next Value-Add Backlog |
| Category | Project Planning |
| Version | 1.8.0 |
| Status | Approved |
| Created | 30-07-2026 |
| Last Updated | 06-08-2026 |

---

# 1. Purpose

Prioritized **documented** work not yet shipped, derived from the planning corpus (roadmap, M15 completion, post-M15 investigation stories). Use this to sequence releases after [M15](M15-WEB-PLATFORM.md).

**Current public release:** `v2.6.1` (Stories 1–4 shipped). **Story 5 completes Phase A** — Connect the Evidence (see §5).

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
Next    → Story 5 Connect the Evidence — P0.1 Evidence Links (§5)
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

# 5. Active queue — Story 5 completes Phase A (P0–P5)

| Priority | Item | Type | Notes |
|----------|------|------|-------|
| **P0** | **Connect the Evidence** (Story 5) | **Implementation** — next flagship | Timeline-first Evidence Links — "Related Evidence" on timeline; not a Correlation view in v1 |
| **P0.1** | Evidence Links + timeline decorations | **Implementation** — ship first | `PRECEDES`, `FOLLOWS`, `SUPPORTS`, `RELATED`; jump to linked events |
| **P0.2** | Evidence Groups | **Implementation** | Named clusters of related events |
| **P0.3** | Correlation View | **Deferred** | When links are rich |
| **P1** | Crash Timeline | **Implementation** — quick win | `crash.summary` on timeline projection — deferred from v2.6.0; small release |
| **P2** | Desktop Timeline/Crash parity | **Implementation** | Web has bottom dock tabs; desktop gap in [`tests/e2e/web/README.md`](../../tests/e2e/web/README.md) |
| **P3** | ADR-010 domain model + events | **Emergent** | Extract when patterns recur after P0–P1 |
| **P4** | Investigation Query Language | **Research only** | Questions to answer — don't build yet |
| **P5** | AI Investigation Assistant | **Research only** | Evidence-based sequence synthesis — don't build yet |

Tactical planning docs are added when each **implementation** item (P0–P2) is chartered for release (G0).

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
| Investigation Query Language (P4) | Research only — not build queue |
| AI Investigation Assistant (P5) | Research only — not build queue |
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
| 1.6.0 | 06-08-2026 | Execution Contract v2 reassessment — P0–P5 queue; Connect the Evidence is next flagship; P4/P5 research only. |
| 1.7.0 | 06-08-2026 | Story 5 **Connect the Evidence** — timeline-first Evidence Links; Correlation view deferred (P0.3). |
| 1.8.0 | 06-08-2026 | Phase A = five stories; Story 5 completes investigation methodology; relationships are evidence not conclusions. |
