# V2.9.0 Crash Timeline — Scenario Matrix (P1)

P1: surface `crash.summary` on the investigation timeline from existing `CrashReport` projection. Status key: **P** planned · **I** implemented · **U** unit · **W** web integration · **E** E2E

| ID | Scenario | REST | CLI | Web | Unit | E2E | Notes |
|----|----------|------|-----|-----|------|-----|-------|
| CT.1 | Single pstack → one `crash.summary` when `ready` | I | I | I | U | E | Story Gate path |
| CT.2 | `eventType` literal `crash.summary` | I | I | I | U | — | ADR binding |
| CT.3 | Stable `TimelineEvent.id` across GETs | I | I | — | U | — | `makeTimelineEventId` |
| CT.4 | Timestamp from `sourceModifiedAt` or `importedAt` | I | I | — | U | — | v2.9.1 ([#172](https://github.com/AntonyCyriac/logscope/issues/172)) |
| CT.5 | No `artifact.attached` for pstack/core | I | I | — | U | — | Replaced by CT.1 |
| CT.6 | Other types still emit `artifact.attached` | I | I | — | U | — | Notes, etc. unchanged |
| CT.7 | Two pstack artifacts → two summaries | I | I | I | U | — | N artifacts → N max |
| CT.8 | Chronological merge with log events | I | I | I | U | E | Global sort |
| CT.9 | `unavailable` → degraded `crash.summary` | I | I | I | U | — | Visible row |
| CT.10 | `failed` → degraded `crash.summary` | I | I | I | U | — | Visible row |
| CT.11 | `not_supported` → no summary | I | — | — | U | — | |
| CT.12 | Metadata: `crashReportId`, `status`, `signal` | I | I | — | U | — | No `lineNumber` |
| CT.13 | Timeline row → jump to Crash tab | — | — | I | — | E | Story Gate |
| CT.14 | Evidence link to `crash.summary` id | I | I | I | U | — | Story 5 stretch |
| CT.15 | Story 4 crash-analysis tests unchanged | — | — | — | U | — | No Story 4 reopen |
| CT.16 | No manifest schema bump | I | — | — | U | — | |

## Story Gate (E2E)

```text
Create Investigation → Add app.log → Add pstack
  → Analyze crash (Crash tab or on timeline load)
  → Open Timeline
  → crash.summary visible among log events
  → Select row → Crash tab / pstack evidence
```

**Example narrative:**

```text
09:41:02  app.log         Connection timeout
09:41:03  syslog          Retry started
09:41:05  crash.summary   SIGSEGV — thread 18
```

## Non-goals (NG)

| ID | Scenario | Expected |
|----|----------|----------|
| NG.1 | Parser / GDB / dialect changes | TID dialect shipped `v2.10.0` ([#144](https://github.com/AntonyCyriac/logscope/issues/144)); plugin hook deferred |
| NG.2 | Fault-frame `lineNumber` on event | Omitted in v2.9.0 |
| NG.3 | AI root cause fields | Absent |
| NG.4 | Desktop Timeline parity | Shipped `v2.11.0` — [V211-DESKTOP-PARITY-SCENARIOS.md](V211-DESKTOP-PARITY-SCENARIOS.md) |

## Revision history

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 12-08-2026 | Initial matrix from G1-approved design |
| 1.1.0 | 13-08-2026 | Status update — P1 shipped in `v2.9.0` |
