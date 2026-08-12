# V2.8.0 Discover the Connections — Scenario Matrix (Story 6)

Story 6 P0: ephemeral correlation suggestions with explainability and explicit accept. Status key: **P** planned · **I** implemented · **U** unit · **W** web integration · **E** E2E

| ID | Scenario | REST | CLI | Web | Unit | E2E | Notes |
|----|----------|------|-----|-----|------|-----|-------|
| S6.1 | Two logs share `request_id` → suggestion with basis | P | P | P | P | P | Story Gate positive path |
| S6.2 | Accept suggestion → `RELATED` EvidenceLink | P | P | P | P | P | Delegates to Story 5 API |
| S6.3 | After accept, all pair suggestions suppressed (other keys) | P | P | P | P | P | Binding G1 pair dedup |
| S6.4 | Dismiss → no link; reload may recompute | P | P | P | P | P | Story Gate negative path |
| S6.5 | Existing EvidenceLink → pair not suggested | P | P | P | P | P | Pre-accept dedup |
| S6.6 | Same artifact events → no suggestion | P | P | — | P | — | Cross-artifact only |
| S6.7 | ±30s filter excludes distant key match | P | — | — | P | — | Proximity never sole criterion |
| S6.8 | Missing timestamp → key match still suggested | P | — | — | P | — | |
| S6.9 | Cap truncation (`truncated: true`) | P | P | P | P | — | 50/event, 500/investigation |
| S6.10 | `STALE_SUGGESTION` on accept after timeline change | P | P | P | P | — | |
| S6.11 | UI shows basis (key, value, artifacts, time delta) | — | — | P | — | E | Explainability contract |
| S6.12 | No "Correlation" in user-facing copy | — | — | P | — | E | Suggested connections only |

## Story Gate (E2E)

**Positive:**

```text
Create Investigation → Add app.log → Add syslog → Open Timeline
  → Select app error → Suggested connections
  → Review basis (request_id, artifacts, time delta)
  → Accept → RELATED EvidenceLink
  → Related Evidence → Jump to syslog
```

**Negative:**

```text
Suggestion → Dismiss → no EvidenceLink
  → Reload → suggestion may recompute → dismiss again or accept
```

## Revision history

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 12-08-2026 | Initial matrix from G1-approved design |
