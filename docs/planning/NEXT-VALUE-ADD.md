# Next Value-Add Backlog

| Field | Value |
|-------|-------|
| Document | Next Value-Add Backlog |
| Category | Project Planning |
| Version | 2.0.0 |
| Status | Approved |
| Created | 30-07-2026 |
| Last Updated | 18-08-2026 |

---

# 1. Purpose

Prioritized **documented** work not yet shipped, derived from the planning corpus (roadmap, M15 completion, post-M15 investigation stories). Use this to sequence releases after [M15](M15-WEB-PLATFORM.md).

**Current public release:** `v2.11.0` (P2 desktop Timeline/Crash parity shipped). Phase A + P1 + P1.1 + P2 complete — see §5–§6.

---

# 2. Recommended sequence

```text
v2.2.1  → shipped (M15.4 thin auth)
v2.2.2  → shipped (API key hashing)
v2.3.0  → shipped (Create an Investigation)
v2.4.0  → shipped (Understand Everything)
v2.5.0  → shipped (See What Happened)
v2.6.0  → shipped (Understand Why It Crashed)
v2.7.0  → shipped (Story 5 Connect the Evidence)
v2.7.1  → shipped (post-Story 5 hotfix bundle)
v2.8.0  → shipped (Story 6 Discover the Connections)
v2.9.0  → shipped (P1 Crash Timeline)
v2.9.1  → shipped (timeline hotfix #171/#172)
v2.10.0 → shipped (P1.1 TID pstack dialects #144)
v2.10.1 → shipped (storage hotfix #163/#164)
v2.11.0 → shipped (P2 desktop Timeline/Crash parity)
Next    → P2.1 (§5) · then #144 Option B (§5) · P3 ADR-010 reactive only (§5)
```

**Queue discipline:** P2.1 (product UX), #144 Option B (developer extensibility), and P3 domain architecture are **three separate tracks** — do not combine into one milestone.

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

# 5. Active queue — post–Phase A (P1–P5)

| Priority | Item | Type | Notes |
|----------|------|------|-------|
| **P0** | **Discover the Connections** (Story 6) | **Shipped** — `v2.8.0` | Ephemeral correlation suggestions; accept → evidence links |
| **P0** | **Connect the Evidence** (Story 5) | **Shipped** — `v2.7.0` | Timeline-first Evidence Links |
| **P0.2** | Evidence Groups | **Implementation** | Named clusters of related events |
| **P0.3** | Correlation View | **Deferred** | When links are rich |
| **P1** | Crash Timeline | **Shipped** — `v2.9.0` | `crash.summary` on timeline projection |
| **P1.1** | Alternate pstack dialects | **Shipped** — `v2.10.0` ([#144](https://github.com/AntonyCyriac/logscope/issues/144)) | TID `symbol - /path` dialect; plugin hook deferred |
| **P2** | Desktop Timeline/Crash parity | **Shipped** — `v2.11.0` | Qt Timeline + Crash via `InvestigationController`; matrix: [`V211-DESKTOP-PARITY-SCENARIOS.md`](V211-DESKTOP-PARITY-SCENARIOS.md) |
| **P2.1** | Desktop Evidence & Suggestions chrome | **Next** — G1 pending (`v2.12.0`) | Make desktop investigations as interactive as web; see §5.1 |
| **#144-B** | `register_crash_analyzer` plugin hook | **Backlog** — architecture | [#144](https://github.com/AntonyCyriac/logscope/issues/144) Option B; charter when a real second analyzer/dialect should not live in core |
| **P3** | Domain model + events (ADR-010 charter) | **Reactive** — not scheduled | Charter only when P2.1 or #144-B expose a decision that needs formalization; not a milestone on its own |
| **P4** | Investigation Query Language | **Research only** | Questions to answer — don't build yet |
| **P5** | AI Investigation Assistant | **Research only** | Evidence-based sequence synthesis — don't build yet |

Tactical planning docs are added when each **implementation** item (P0–P2.1, #144-B) is chartered for release (G0).

### P2.1 — Desktop Evidence & Suggestions (next)

**Definition:** Make desktop investigations as interactive as web investigations.

**Type:** Product work (user-facing parity) — **not** combined with #144-B.

```text
Timeline → Related Evidence → Suggested connections → accept/dismiss → jump between evidence
```

| # | Acceptance criterion |
|---|---------------------|
| 1 | Related Evidence panel on desktop (Story 5) |
| 2 | Create/remove evidence links in UI |
| 3 | Suggested Connections panel — accept/dismiss (Story 6) |
| 4 | Background Timeline/Crash refresh (performance; v2.11.0 sync limitation) |
| 5 | No new domain model — consume existing Investigation / EvidenceLink / Suggestion contracts |
| 6 | Story Gate path extended on desktop for evidence + suggestion loop |

**Out of P2.1:** new parsers, `register_crash_analyzer`, correlation algorithm changes, redesign.

### #144 Option B — `register_crash_analyzer` (after P2.1, when justified)

**Type:** Architecture / developer extensibility — **not** product UX.

```text
Crash analyzer registry → pstack parser · GDB parser · custom analyzer
```

**Trigger:** A real second crash dialect or analyzer appears that should **not** belong in core. Do not implement merely because it is in the backlog.

### P3 — domain architecture (reactive only)

**Not a scheduled milestone.** Let the product create architectural pressure; then extract the architecture. P2.1 and #144-B may inform what a future domain-events charter actually needs — do not pre-charter ADR work speculatively.

### P2 acceptance criteria (G0 — measurable; shipped `v2.11.0`)

| # | Criterion |
|---|-----------|
| 1 | Desktop bottom dock: **Timeline** + **Crash** tabs (same labels as web) |
| 2 | Timeline renders `TimelineProjectionResult` including `crash.summary` (P1) |
| 3 | Crash tab renders ephemeral `CrashReport` (Story 4) |
| 4 | Story Gate on desktop: investigation + app.log + pstack → timeline → crash → jump |
| 5 | Headless `logscope_desktop_tests` covers AC.4; web Playwright remains green |
| 6 | No new timeline/crash capabilities; no desktop-specific domain models |

**Out of P2:** new parsers (#144), correlation changes, redesign, AI, enterprise.

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
| 1.9.0 | 07-08-2026 | `v2.7.1` shipped; [#144](https://github.com/AntonyCyriac/logscope/issues/144) pstack dialects backlogged as P1.1. |
| 2.0.0 | 12-08-2026 | `v2.8.0` shipped — Story 6 Discover the Connections; Phase A complete (Stories 1–6). |
| 2.4.0 | 18-08-2026 | `v2.10.1` shipped — storage hotfix (#163, #164). |
| 2.5.0 | 18-08-2026 | P2 desktop Timeline/Crash parity — G0 chartered (target `v2.11.0`). |
| 2.6.0 | 18-08-2026 | P2 G1 approved — ADR-008-M14.1 + V211 scenario matrix. |
| 2.7.0 | 18-08-2026 | `v2.11.0` shipped — P2 desktop Timeline/Crash parity. |
| 2.8.0 | 18-08-2026 | Post-`v2.11.0` queue locked: **P2.1** (product) → **#144-B** (extensibility, when justified) → **P3** (reactive charter only). |
