# v2.5.0 — See What Happened Scenarios

| Field | Value |
|-------|-------|
| Document | v2.5.0 See What Happened Scenarios |
| Category | Project Planning |
| Version | 0.1.0 |
| Status | **Story Gate closed — `v2.5.0`** |
| Design reference | [ADR-009-M15.7](../architecture/decisions/ADR-009-M15.7-Investigation-Timeline.md) |
| Created | 06-08-2026 |
| Last Updated | 06-08-2026 |

---

# 1. Purpose

Acceptance scenarios for **v2.5.0** — Story 3: **chronological investigation narrative** via timeline **projection** (merge events, not correlate).

**Demo path (Story Gate):**

```text
Create investigation → Add app.log → Add syslog → Add note
  → Open timeline → See events in chronological order
  → Click event → Jump to source artifact
```

See [V240-UNDERSTAND-EVERYTHING-SCENARIOS.md](V240-UNDERSTAND-EVERYTHING-SCENARIOS.md) (Story 2 baseline).

---

# 2. Legend

Status: ⬜ planned · 🟡 partial / manual · ✅ complete

| Column | Meaning |
|--------|---------|
| **Parity** | REST/CLI event JSON shape matches |
| **Test** | U = unit, I = integration, P = parity, W = web smoke, B = browser (manual) |

---

# 3. Timeline projection (TL)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| TL.1 | Multi-artifact events | `GET .../timeline` after app + syslog + note | Events from **all** artifacts | I | ✅ |
| TL.2 | Chronological order | `order=asc` | Oldest event first | U+I | ✅ |
| TL.3 | Stable event id | Same investigation, two requests | Same `id` for same logical event | U | ✅ |
| TL.4 | Log line events | App log with timestamps | `eventType=log.line`, parsed timestamp | U+I | ✅ |
| TL.5 | Note event | Note artifact | `note.created` at `importedAt` | U | ✅ |
| TL.6 | Core marker | Core artifact | `artifact.attached` marker, no analysis | U | ✅ |
| TL.7 | Pagination | Large log + `limit=50` | `truncated: true`, max 50 events | I | ✅ |

---

# 4. Navigation (NV)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| NV.1 | Jump to artifact | Click timeline event (web) | Opens source artifact in workbench | B | ✅ |
| NV.2 | Line hint | Log line event | `source.lineNumber` present | I | ✅ |

---

# 5. REST API (IA)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| IA.1 | Timeline GET | `GET .../investigations/{id}/timeline` | 200, `events[]` domain JSON | I | ✅ |
| IA.2 | Not found | Unknown investigation id | **404** | I | ✅ |
| IA.3 | No correlation | Timeline response | No ID-grouping or relationship fields | U | ✅ |

---

# 6. CLI (CL)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| CL.1 | Timeline command | `investigation timeline <id>` | Human table output | U | ✅ |
| CL.2 | JSON parity | `--format json` | Matches REST event fields | P | 🟡 |

---

# 7. UI (UI)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| UI.1 | Timeline panel | Select investigation | Timeline view visible | B | ✅ |
| UI.2 | Empty state | Investigation with no artifacts | Empty message, not error | B | ✅ |
| UI.3 | No visual correlation | Timeline view | No lines/grouping between events | B | ✅ |

---

# 8. Release (REL)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| REL.1 | CI web job | `scope_web_tests` + integration | TL/IA rows green | W | ✅ |
| REL.2 | Story Gate | Manual demo path | End-to-end narrative + jump | B | ✅ |
| REL.3 | ADR accepted | PR merge | ADR-009-M15.7 **Accepted** | — | ✅ |
