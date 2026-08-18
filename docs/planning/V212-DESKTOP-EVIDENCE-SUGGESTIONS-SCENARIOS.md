# V2.12.0 Desktop Evidence & Suggestions — Scenario Matrix (P2.1)

P2.1: Qt **Related Evidence** + **Suggested connections** chrome and async Timeline/Crash refresh — no domain changes. Status key: **P** planned · **I** implemented · **U** unit · **D** desktop GUI · **E** web E2E regression

| ID | Scenario | Desktop | Unit | Web E2E | Notes |
|----|----------|---------|------|---------|-------|
| ES.1 | Timeline row select shows Related Evidence panel | I | — | — | Story 5 |
| ES.2 | Timeline row select shows Suggested connections when suggestions exist | I | — | E | Story 6 fixtures |
| ES.3 | Create evidence link between two timeline events | I | U | — | `addEvidenceLink` |
| ES.4 | Timeline `Related (N)` badge after link create | I | D | — | `timeline-link-badge` |
| ES.5 | Related Evidence row jump to peer log line | I | D | — | Results highlight |
| ES.6 | Remove evidence link clears badge | I | D | — | |
| ES.7 | Accept suggestion creates persisted link | I | U | E | Story 6 positive |
| ES.8 | Dismiss suggestion — no link persisted | I | D | E | Story 6 negative |
| ES.9 | Async timeline load — UI not blocked | I | D | — | Worker wired |
| ES.10 | Async crash load — UI not blocked | I | D | — | Worker wired |
| ES.11 | P2 Story Gate regression | I | D | — | DP.1–DP.15 |
| ES.12 | Session mode unchanged | I | D | — | Regression |
| ES.13 | No REST/CLI JSON shape changes | I | U | — | Regression |
| ES.14 | Web Playwright Story 5/6 unchanged | — | — | E | Regression |

## Story Gate (Desktop headless — blocking G3)

**Story 5 path:**

```text
Investigation → app.log + syslog + pstack
  → Timeline → select app log line
  → Create evidence link to syslog event
  → Related (1) badge visible
  → Related Evidence panel → jump to linked event
  → Remove link → badge cleared
```

**Story 6 path:**

```text
Investigation → story6-app.log + story6-syslog.log
  → Timeline → select line with suggestion
  → Suggested connections panel visible
  → Accept → link in Related Evidence
  → Select another line → Dismiss → no link created
```

## Non-goals (NG)

| ID | Scenario | Expected |
|----|----------|----------|
| NG.1 | `register_crash_analyzer` (#144-B) | Separate milestone |
| NG.2 | P3 domain events charter | Reactive only |
| NG.3 | New correlation heuristics | Frozen |
| NG.4 | Three-pane center viewer redesign | Out of scope |
| NG.5 | Desktop-specific domain models | Forbidden |

## Revision history

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 18-08-2026 | Initial matrix from G1-approved design (pending sign-off) |
| 1.1.0 | 18-08-2026 | G3 — statuses updated; shipped `v2.12.0` |
