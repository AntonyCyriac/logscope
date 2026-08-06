# v2.6.0 — Understand Why It Crashed Scenarios

| Field | Value |
|-------|-------|
| Document | v2.6.0 Understand Why It Crashed Scenarios |
| Category | Project Planning |
| Version | 0.1.0 |
| Status | **In progress — `v2.6.0`** |
| Design reference | [ADR-009-M15.8](../architecture/decisions/ADR-009-M15.8-Crash-Analysis.md) |
| Created | 06-08-2026 |
| Last Updated | 06-08-2026 |

---

# 1. Purpose

Acceptance scenarios for **v2.6.0** — Story 4: **crash evidence projection** from `pstack` and `core` artifacts via `IArtifactCrashAnalyzer` (evidence only, no AI root cause).

**Demo path (Story Gate):**

```text
Create investigation → Add app.log → Add pstack.txt
  → Understand Why It Crashed → Crash Report
  → Click fault thread → Jump to pstack
```

See [V250-SEE-WHAT-HAPPENED-SCENARIOS.md](V250-SEE-WHAT-HAPPENED-SCENARIOS.md) (Story 3 baseline).

---

# 2. Legend

Status: ⬜ planned · 🟡 partial / manual · ✅ complete

| Column | Meaning |
|--------|---------|
| **Parity** | REST/CLI report JSON shape matches |
| **Test** | U = unit, I = integration, P = parity, W = web smoke, B = browser (manual) |

---

# 3. Pstack analysis (PS)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| PS.1 | Parse threads | `GET .../crash-analysis` on pstack artifact | `threads[]` with frames | U+I | ✅ |
| PS.2 | Signal detection | pstack with `Program received signal SIGSEGV` | `signal: SIGSEGV` | U | ✅ |
| PS.3 | Fault thread | pstack with `[Switching to thread N]` | `faultThreadId` + `isFaultThread: true` | U | ✅ |
| PS.4 | Observations | Parsed pstack | `observations[]` factual bullets, no root cause | U | ✅ |
| PS.5 | Stable report id | Same artifact, two requests | Same `id` | U | ✅ |
| PS.6 | Synthetic fixture | `samples/pstack.txt` in CI | Parser unit tests pass | U | ✅ |

---

# 4. Core analysis (CR)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| CR.1 | GDB absent | `GET .../crash-analysis` on core, no GDB | `status: unavailable`, HTTP **200** | U | 🟡 |
| CR.2 | GDB present | Core + GDB on PATH | `status: complete` or `partial` | U | ⬜ |
| CR.3 | No symbols | Core without debug info | `partial`, observation about symbols | U | ⬜ |
| CR.4 | Never 500 on missing GDB | Core artifact, CI without GDB | HTTP **200**, not 500 | I | 🟡 |

---

# 5. REST API (IA)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| IA.1 | Crash analysis GET | `GET .../artifacts/{id}/crash-analysis` | 200, `report` domain JSON | I | ✅ |
| IA.2 | Not found | Unknown investigation id | **404** | I | ⬜ |
| IA.3 | Not analyzable | Log artifact | **409** `ARTIFACT_NOT_ANALYZABLE` | I | ✅ |
| IA.4 | No root cause fields | Report JSON | No `rootCause`, `confidence`, or AI fields | U | ✅ |
| IA.5 | Unavailable is 200 | Core without GDB | **200** + `status: unavailable` | I | 🟡 |

---

# 6. CLI (CL)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| CL.1 | Crash command | `investigation crash <id> --artifact <pstack-id>` | Human table output | U | ⬜ |
| CL.2 | JSON parity | `--format json` | Matches REST report fields | P | ⬜ |
| CL.3 | Default artifact | Omit `--artifact` with pstack present | Analyzes first pstack/core | U | ⬜ |
| CL.4 | Log rejection | `--artifact` on log id | Error exit, not analyzable | U | ✅ |

---

# 7. Investigation aggregate (IV)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| IV.1 | analyzeCrash entry | `Investigation::analyzeCrash(artifactId)` | Routes to correct analyzer | U | ✅ |
| IV.2 | No manifest persistence | After analyze | `manifest.json` unchanged | U | ⬜ |
| IV.3 | Registry extensibility | Unknown type (note) | `ARTIFACT_NOT_ANALYZABLE` | U | ⬜ |

---

# 8. Web UX (UX) — deferred unless quick

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| UX.1 | Panel title | Open pstack artifact | **Understand Why It Crashed** panel | B | ⬜ |
| UX.2 | Fault thread jump | Click fault thread | Scroll/highlight in pstack view | B | ⬜ |
| UX.3 | Core unavailable message | Core without GDB | Shows warning, not error page | B | ⬜ |

---

# 9. Non-goals (NG)

| ID | Scenario | Expected | Status |
|----|----------|----------|--------|
| NG.1 | No timeline events | Timeline GET unchanged; no `crash.summary` events | ✅ |
| NG.2 | No manifest bump | `schemaVersion: 1` | ✅ |
| NG.3 | No AI root cause | No LLM fields in report | ✅ |
| NG.4 | No correlation | Report has no related-artifact joins | ✅ |

---

# 10. Story Gate checklist

| Step | Status |
|------|--------|
| Create investigation | ⬜ |
| Add app.log | ⬜ |
| Add pstack.txt | ⬜ |
| Crash report via REST or CLI | 🟡 |
| Fault thread identified | ✅ |
| Jump to pstack (web) | ⬜ |
