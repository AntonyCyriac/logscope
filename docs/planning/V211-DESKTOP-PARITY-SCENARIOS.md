# V2.11.0 Desktop Parity — Scenario Matrix (P2)

P2: Qt **Timeline** + **Crash** tabs consuming existing `projectTimeline()` / `analyzeCrash()` — no domain changes. Status key: **P** planned · **I** implemented · **U** unit · **D** desktop GUI · **E** web E2E regression

| ID | Scenario | Desktop | Unit | Web E2E | Notes |
|----|----------|---------|------|---------|-------|
| DP.1 | New investigation → artifact list empty | I | — | — | Investigation mode |
| DP.2 | Add log + pstack artifacts | I | U | — | Same ingest as CLI |
| DP.3 | Timeline tab lists log events after analyze | I | — | — | `log.line` rows |
| DP.4 | Timeline shows `crash.summary` for pstack | I | — | E | P1 projection consumed |
| DP.5 | `crash.summary` row distinct styling (Crash type) | I | — | — | Match web label |
| DP.6 | Select `crash.summary` → Crash tab active | I | D | — | Story Gate |
| DP.7 | Crash tab shows SIGSEGV + fault thread | I | D | — | Story 4 report |
| DP.8 | Fault-thread click → pstack thread highlight | I | D | — | Embedded viewer |
| DP.9 | Timeline `log.line` → Results highlight | I | D | — | Optional stretch |
| DP.10 | Session mode unchanged (open/analyze/AI) | I | D | — | Regression |
| DP.11 | Timeline/Crash disabled without investigation | I | D | — | Empty state |
| DP.12 | Async Timeline/Crash refresh | I | D | — | Shipped `v2.12.0` (was sync in v2.11.0) |
| DP.13 | Investigations interoperable with CLI container | I | U | — | Open round-trip test |
| DP.14 | No REST/CLI JSON shape changes | I | U | — | Regression |
| DP.15 | Web Playwright Story Gate unchanged | — | — | E | Regression |

## Known limitation (v2.11.0 — resolved in v2.12.0)

> **v2.11.0 used synchronous Timeline/Crash refresh on the UI thread.** P2.1 (`v2.12.0`) wires `TimelineLoadWorker` / `CrashLoadWorker` as the default load path. Domain contracts are unchanged.

Historical note (v2.11.0 only): `TimelinePanel` and `CrashPanel` called `InvestigationController::projectTimeline()` / `analyzeCrash()` on the UI thread when a tab was activated. Do **not** introduce a desktop-specific domain model to optimize — background workers against existing controller APIs is the approved path (now shipped).

## Story Gate (Desktop headless — blocking G3)

```text
Investigation → New
  → Add samples/sample.log
  → Add samples/sample.pstack (or story-gate pstack fixture)
  → Timeline tab → crash.summary visible (SIGSEGV / SessionManager)
  → Select crash.summary row
  → Crash tab → crash-signal shows SIGSEGV
  → Click fault thread → status "Jumped to pstack thread" + highlighted thread
```

## Story Gate (Web regression — blocking G3)

All rows in `V290-CRASH-TIMELINE-SCENARIOS.md` and Story 1–6 Playwright specs remain green — no web code changes expected.

## Non-goals (NG)

| ID | Scenario | Expected |
|----|----------|----------|
| NG.1 | Related Evidence panel on desktop | P2.1 follow-up |
| NG.2 | Create/remove evidence link UI | P2.1 follow-up |
| NG.3 | Correlation suggestions panel | P2.1 follow-up |
| NG.4 | Accept/dismiss suggestion UI | P2.1 follow-up |
| NG.5 | Three-pane IDE center viewer redesign | Out of scope |
| NG.6 | New timeline/crash capabilities | Frozen |
| NG.7 | Parser / #144 plugin hook | After P2 |
| NG.8 | Desktop-specific domain models | Forbidden |

## Revision history

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 18-08-2026 | Initial matrix from G1-approved design |
| 1.1.0 | 18-08-2026 | G3 — statuses updated; synchronous refresh documented as v2.11.0 limitation |
