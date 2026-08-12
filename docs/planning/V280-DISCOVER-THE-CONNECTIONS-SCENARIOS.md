# V2.8.0 Discover the Connections — Scenario Matrix (Story 6)

Story 6 P0: ephemeral correlation suggestions with explainability and explicit accept. Status key: **P** planned · **I** implemented · **U** unit · **W** web integration · **E** E2E

| ID | Scenario | REST | CLI | Web | Unit | E2E | Notes |
|----|----------|------|-----|-----|------|-----|-------|
| S6.1 | Two logs share `request_id` → suggestion with basis | I | I | I | U | E | Story Gate positive path |
| S6.2 | Accept suggestion → `RELATED` EvidenceLink | I | I | I | U | E | Delegates to Story 5 API |
| S6.3 | After accept, all pair suggestions suppressed (other keys) | I | I | I | U | — | Binding G1 pair dedup |
| S6.4 | Dismiss → no link; reload may recompute | I | I | I | U | E | Story Gate negative path |
| S6.5 | Existing EvidenceLink → pair not suggested | I | I | I | U | — | Pre-accept dedup |
| S6.6 | Same artifact events → no suggestion | I | I | — | U | — | Cross-artifact only |
| S6.7 | ±30s filter excludes distant key match | I | — | — | U | — | Proximity never sole criterion |
| S6.8 | Missing timestamp → key match still suggested | I | — | — | U | — | |
| S6.9 | Cap truncation (`truncated: true`) | I | I | I | U | — | 50/event, 500/investigation |
| S6.10 | `STALE_SUGGESTION` on accept after timeline change | I | I | I | U | — | |
| S6.11 | UI shows basis (key, value, artifacts, time delta) | — | — | I | — | E | Explainability contract |
| S6.12 | No "Correlation" in user-facing copy | — | — | I | — | E | Suggested connections only |

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
| 1.1.0 | 12-08-2026 | Status update — Story 6 shipped in `v2.8.0` |
