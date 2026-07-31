# M15 v2.2.0 — Shared Investigations Scenarios

| Field | Value |
|-------|-------|
| Document | M15 v2.2.0 Shared Investigations Scenarios |
| Category | Project Planning |
| Version | 0.2.0 |
| Status | **Approved — G1 architecture gate** |
| Design reference | `logscope-strategy/engos/artifacts/M15.3/design.md` · [ADR-009-M15.3](../architecture/decisions/ADR-009-M15.3-Shared-Investigations.md) |
| Created | 31-07-2026 |
| Last Updated | 31-07-2026 |

---

# 1. Purpose

Acceptance scenarios for **v2.2.0** — M15.3 Shared Investigations (shared workspaces API, tail poll, async analyze jobs, SPA wiring).

**Target release:** `v2.2.0`

See [M15-WEB-PLATFORM.md](M15-WEB-PLATFORM.md), [ADR-009](../architecture/decisions/ADR-009-Web-Platform-REST.md), [M15-V210-WEB-SCENARIOS.md](M15-V210-WEB-SCENARIOS.md) (v2.1.0 baseline).

---

# 2. Legend

Status: ⬜ planned · 🟡 in progress · ✅ complete

| Column | Meaning |
|--------|---------|
| **Parity** | REST result matches CLI `--format json` on `samples/` or documented desktop tail semantics |
| **Test** | U = unit, I = integration, P = parity, W = web smoke, B = browser (manual; Playwright deferred) |

---

# 3. Shared workspaces API (SW1)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| SW1.1 | Create workspace | `POST /api/v1/workspaces` with name + session snapshot | Returns workspace id; JSON file persisted under configured storage dir | U+I | ⬜ |
| SW1.2 | List workspaces | `GET /api/v1/workspaces` | Returns array of metadata (id, name, updatedAt); excludes other tenants' paths | I | ⬜ |
| SW1.3 | Get workspace | `GET /api/v1/workspaces/{id}` | Returns metadata + summary counts without cross-session model leak | U+I | ⬜ |
| SW1.4 | Update metadata | `PUT /api/v1/workspaces/{id}` | Name/description updated; persistence round-trip | U+I | ⬜ |
| SW1.5 | Delete workspace | `DELETE /api/v1/workspaces/{id}` | Record removed; subsequent GET → 404 | U+I | ⬜ |
| SW1.6 | Open into session | `POST /api/v1/workspaces/{id}/open` + `X-LogScope-Session` | Active session adopts workspace state; investigate works without re-upload | I+P | ⬜ |
| SW1.7 | Save to shared | Save current session targeting shared workspace id | Snapshot updated; list reflects new `updatedAt` | I | ⬜ |
| SW1.8 | Unknown id | `GET /workspaces/{bad-id}` | 404 + ADR-009 error envelope | U | ⬜ |

---

# 4. Tail poll (T1)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| T1.1 | Start tail | `POST /api/v1/tail/start` after source open | `200`; tail active on session | U+I | ⬜ |
| T1.2 | Poll lines | `GET /api/v1/tail/poll` while file appends | Returns new lines since last poll; non-blocking | I+P | ⬜ |
| T1.3 | Stop tail | `POST /api/v1/tail/stop` | Tail inactive; poll returns empty or terminal state | U+I | ⬜ |
| T1.4 | Double start | Start when tail already running | 409 `InvalidState` | U | ⬜ |
| T1.5 | Poll without start | `GET /api/v1/tail/poll` before start | 409 or empty per G1 design | U | ⬜ |

---

# 5. Async analyze jobs (J1)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| J1.1 | Large log async | `POST /api/v1/analyze` on `large-app.log` (or threshold) | `202 Accepted` + `jobId`; HTTP thread returns promptly | I | ⬜ |
| J1.2 | Job poll running | `GET /api/v1/jobs/{id}` while analyze in flight | `status: running` | I | ⬜ |
| J1.3 | Job poll complete | Poll after analyze finishes | `status: completed` + `AnalysisModel` summary; counts match CLI analyze JSON | I+P | ⬜ |
| J1.4 | Job poll failed | Analyze on invalid state / corrupt input | `status: failed` + error envelope | U+I | ⬜ |
| J1.5 | Small log sync | `POST /api/v1/analyze` on `sample.log` below threshold | `200` synchronous (backward compatible) | I+P | ⬜ |
| J1.6 | Cross-session job | Session B polls Session A job id | 403/404 — no cross-session access | I | ⬜ |

---

# 6. Security & isolation (S2)

| ID | Scenario | Expected | Test | Status |
|----|----------|----------|------|--------|
| S2.1 | Workspace path traversal | Storage path with `..` rejected; no escape from `web.allowed_path_roots` / workspace dir | U | ⬜ |
| S2.2 | Session isolation | Session A cannot open/mutate Session B workspace binding | I | ⬜ |
| S2.3 | Job isolation | Job state scoped to creating session | I | ⬜ |
| S2.4 | API key (unchanged) | When `web.api_key` set, new routes require key (health exempt) | U+I | ⬜ |
| S2.5 | Default bind | Still `127.0.0.1` unless configured (regression) | U | ⬜ |

---

# 7. Parity (P1) — regression + new rows

| ID | Scenario | Expected | Test | Status |
|----|----------|----------|------|--------|
| P1.1 | v2.1.0 L1 rows | L1.1–L1.8 still pass | I+P | ⬜ |
| P1.2 | v2.1.0 R1 rows | R1.1–R1.7 still pass | I+P | ⬜ |
| P1.3 | Post-workspace-open investigate | `matchCount` = CLI after SW1.6 open | P | ⬜ |
| P1.4 | Post-async analyze investigate | Investigate after J1.3 complete = CLI | P | ⬜ |
| P1.5 | Tail line format | Polled lines parseable; counts consistent with append | P | ⬜ |

---

# 8. Browser SPA (W2) — M15.3 UI

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| W2.1 | Shared list | Navigate to shared workspaces view | Lists SW1.2 workspaces | W | ⬜ |
| W2.2 | Open shared | Click open on workspace row | Workbench loads; table/filters restored | W | ⬜ |
| W2.3 | Save shared | Save current investigation to new/existing shared workspace | SW1.7 succeeds; list updates | W | ⬜ |
| W2.4 | Tail panel | Start tail on open log | Polled lines appear in UI | W | ⬜ |
| W2.5 | Async analyze UX | Analyze large log | UI shows job progress; completes with level counts | W | ⬜ |

---

# 9. Release gate

| ID | Scenario | Expected | Status |
|----|----------|----------|--------|
| REL.1 | SW1 + T1 + J1 + S2 + P1 API rows | Pass on CI `web` job | ⬜ |
| REL.2 | W2 manual checklist | MVP UI flows verified pre-tag | ⬜ |
| REL.3 | `v2.2.0` tag + GitHub Release | Published with web bundles | ⬜ |
| REL.4 | v2.1.0 scenario regression | M15-V210 rows still green | ⬜ |
| REL.5 | Release regression policy | Issues filed per [RELEASE.md §8](../release/RELEASE.md#8-post-release-housekeeping) | ⬜ |

---

# 10. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 0.1.0 | 31-07-2026 | Initial scenario matrix; G0 approved for M15.3 @ v2.2.0. |
| 0.2.0 | 31-07-2026 | G1 complete — design contract + ADR-009-M15.3 amendment; implementer unlocked. |
