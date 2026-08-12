# V2.9.0 Crash Timeline — Scenario Matrix (P1)

P1: surface `crash.summary` on the investigation timeline from existing `CrashReport` projection. Status key: **P** planned · **I** implemented · **U** unit · **W** web integration · **E** E2E

| ID | Scenario | REST | CLI | Web | Unit | E2E | Notes |
|----|----------|------|-----|-----|------|-----|-------|
| CT.1 | Single pstack → one `crash.summary` when `ready` | P | P | P | P | E | Story Gate path |
| CT.2 | `eventType` literal `crash.summary` | P | P | P | P | — | ADR binding |
| CT.3 | Stable `TimelineEvent.id` across GETs | P | P | — | P | — | `makeTimelineEventId` |
| CT.4 | Timestamp from `artifact.importedAt` | P | P | — | P | — | v2.9.0 only |
| CT.5 | No `artifact.attached` for pstack/core | P | P | — | P | — | Replaced by CT.1 |
| CT.6 | Other types still emit `artifact.attached` | P | P | — | P | — | Notes, etc. unchanged |
| CT.7 | Two pstack artifacts → two summaries | P | P | P | P | — | N artifacts → N max |
| CT.8 | Chronological merge with log events | P | P | P | P | E | Global sort |
| CT.9 | `unavailable` → degraded `crash.summary` | P | P | P | P | — | Visible row |
| CT.10 | `failed` → degraded `crash.summary` | P | P | P | P | — | Visible row |
| CT.11 | `not_supported` → no summary | P | — | — | P | — | |
| CT.12 | Metadata: `crashReportId`, `status`, `signal` | P | P | — | P | — | No `lineNumber` |
| CT.13 | Timeline row → jump to Crash tab | — | — | P | — | E | Story Gate |
| CT.14 | Evidence link to `crash.summary` id | P | P | P | P | — | Story 5 stretch |
| CT.15 | Story 4 crash-analysis tests unchanged | — | — | — | P | — | No Story 4 reopen |
| CT.16 | No manifest schema bump | P | — | — | P | — | |

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
| NG.1 | Parser / GDB / dialect changes | Story 4 code unchanged — [#144](https://github.com/AntonyCyriac/logscope/issues/144) deferred |
| NG.2 | Fault-frame `lineNumber` on event | Omitted in v2.9.0 |
| NG.3 | AI root cause fields | Absent |
| NG.4 | Desktop Timeline parity | P2 deferred |

## Revision history

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 12-08-2026 | Initial matrix from G1-approved design |
