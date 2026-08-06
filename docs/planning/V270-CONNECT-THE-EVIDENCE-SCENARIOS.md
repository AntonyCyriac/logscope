# V2.7.0 Connect the Evidence — Scenario Matrix (P0.1)

Story 5 P0.1: manual Evidence Links on timeline events. Status key: **P** planned · **I** implemented · **U** unit · **W** web integration · **E** E2E.

## Manifest and aggregate

| ID | Scenario | Steps | Expected | Layer | Status |
|----|----------|-------|----------|-------|--------|
| M1 | v1 read, no links | Load schema v1 manifest | `evidenceLinks` treated as `[]`, stays v1 on no-op save | U | ✅ |
| M2 | First link write | Add link to v1 investigation | `schemaVersion: 2`, `evidenceLinks[]` persisted | U | ✅ |
| M3 | v2 round-trip | Save/reopen v2 manifest | Links, ids, `createdAt` immutable | U | ✅ |
| M4 | schema > 2 | Load future schema | Parse error | U | ⬜ |
| M5 | Unknown link type | Malformed v2 link `type` | Parse error on load | U | ⬜ |

## Validation and duplicates

| ID | Scenario | Steps | Expected | Layer | Status |
|----|----------|-------|----------|-------|--------|
| V1 | Same source/target | POST link A→A | **400** `INVALID_LINK_TARGET` | W | ⬜ |
| V2 | Unknown event id | POST with missing `eventId` | **400** `INVALID_LINK_TARGET` | W | ✅ |
| V3 | Duplicate directed triple | POST same source, target, type twice | **409** `DUPLICATE_EVIDENCE_LINK` | U/W | ✅ |
| V4 | Reverse RELATED | `RELATED` A→B then B→A | Both **201** | U | ⬜ |
| V5 | Same pair, different type | `RELATED` then `SUPPORTS` | Both **201** | U | ⬜ |

## Stale policy

| ID | Scenario | Steps | Expected | Layer | Status |
|----|----------|-------|----------|-------|--------|
| S1 | Endpoint missing from projection | Link with orphan `eventId` in manifest | List `status: "stale"` | U | ✅ |
| S2 | Artifact removed | Remove artifact, list links | Link retained, stale if id gone | U | ⬜ |

## REST API

| ID | Scenario | Endpoint | Expected | Layer | Status |
|----|----------|----------|----------|-------|--------|
| R1 | List empty | `GET …/evidence-links` | **200**, `links: []` | W | ✅ |
| R2 | Create | `POST …/evidence-links` | **201**, `status: active` | W | ✅ |
| R3 | Delete | `DELETE …/evidence-links/{id}` | **204** | W | ✅ |
| R4 | Unknown link | `DELETE` bad id | **404** | W | ⬜ |
| R5 | Timeline unchanged | `GET …/timeline` | No `linkCount` / embedded links | W | ✅ |

## CLI

| ID | Scenario | Command | Expected | Layer | Status |
|----|----------|---------|----------|-------|--------|
| C1 | List JSON | `investigation links list <id> --format json` | Matches REST `data` shape | U | ⬜ |
| C2 | Add | `links add --source --target --type RELATED` | Exit 0, prints link | U | ⬜ |
| C3 | Remove | `links remove --link <id>` | Exit 0 | U | ⬜ |

## Web UI (Story Gate)

| ID | Scenario | Steps | Expected | Layer | Status |
|----|----------|-------|----------|-------|--------|
| UI1 | Decoration | Create RELATED link | Badge on both timeline rows | E | ⬜ |
| UI2 | Related Evidence panel | Select linked event | Panel shows peer + type | E | ⬜ |
| UI3 | Jump | Click related row | Navigates to peer event (syslog row) | E | ⬜ |
| UI4 | Vocabulary | UI copy | "Related Evidence" / "Connections" only | E | ⬜ |

## Story Gate (manual)

```text
Create investigation → app.log + syslog + pstack → Timeline
  → Create Evidence Link (RELATED)
  → Timeline decoration on both events
  → Select event → Related Evidence (1)
  → Click linked entry → jump to syslog row
```
