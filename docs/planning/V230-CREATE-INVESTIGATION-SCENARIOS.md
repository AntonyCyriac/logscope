# v2.3.0 — Create an Investigation Scenarios

| Field | Value |
|-------|-------|
| Document | v2.3.0 Create an Investigation Scenarios |
| Category | Project Planning |
| Version | 0.1.0 |
| Status | **Shipped — `v2.3.0`** |
| Design reference | [ADR-009-M15.5](../architecture/decisions/ADR-009-M15.5-Investigation-Container.md) |
| Created | 04-08-2026 |
| Last Updated | 05-08-2026 |

---

# 1. Purpose

Acceptance scenarios for **v2.3.0** — Story 1: portable **Investigation** container with multiple **artifacts** (log + note), save/reopen, REST + CLI + web SPA.

**Demo path (Story Gate):**

```text
Create Investigation → Add log → Add note → Save → Reopen
```

See [ADR-009](../architecture/decisions/ADR-009-Web-Platform-REST.md), [M15-V220-SHARED-INVESTIGATIONS-SCENARIOS.md](M15-V220-SHARED-INVESTIGATIONS-SCENARIOS.md) (`v2.2.0` baseline).

---

# 2. Legend

Status: ⬜ planned · 🟡 partial / manual · ✅ complete

| Column | Meaning |
|--------|---------|
| **Parity** | REST/CLI result matches existing CLI `--format json` on primary log after open |
| **Test** | U = unit, I = integration, P = parity, W = web smoke, B = browser (manual) |

---

# 3. Investigation container (IC)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| IC.1 | Create investigation | `POST /api/v1/investigations` with name | UUID returned; `manifest.json` on disk | U+I | ✅ |
| IC.2 | Add log artifact | `POST .../artifacts` with log upload | Artifact under `artifacts/{id}/data`; manifest updated | U+I | ✅ |
| IC.3 | Add note artifact | `POST .../artifacts` with note body | Note round-trips on GET | U+I | ✅ |
| IC.4 | List investigations | `GET /api/v1/investigations` | Newest `updatedAt` first; bounded list | I | ✅ |
| IC.5 | Open entry artifact | `POST .../open` + session header | Active session has primary log; analyze works | I+P | ✅ |
| IC.6 | Save snapshot | `POST /api/v1/sessions/save` with `investigationId` | `snapshot.session` updated; summary counts | I | ✅ |
| IC.7 | Delete investigation | `DELETE /api/v1/investigations/{id}` | Tree removed; GET → 404 | U+I | ✅ |
| IC.8 | Legacy workspace read | Open dir from v2.2.0 workspace | Synthesized manifest; one log artifact | U | ✅ |

---

# 4. REST API (IA)

| ID | Scenario | Expected | Test | Status |
|----|----------|----------|------|--------|
| IA.1 | Create via REST | 201/200 + `data.id` | I | ✅ |
| IA.2 | List via REST | Array of investigations | I | ✅ |
| IA.3 | Get metadata | Artifact index without inline large bodies | I | ✅ |
| IA.4 | Add artifact | Log and note types accepted | I | ✅ |
| IA.5 | Remove artifact | DELETE artifact; manifest consistent | I | ✅ |
| IA.6 | Open into session | Entry log loaded | I+P | ✅ |
| IA.7 | Delete investigation | 404 on subsequent access | I | ✅ |
| IA.8 | Workspace alias | `GET /api/v1/workspaces` same ids as investigations | I | ✅ |

---

# 5. CLI (CL)

| ID | Scenario | Expected | Test | Status |
|----|----------|----------|------|--------|
| CL.1 | `investigation create` | Manifest + empty artifacts dir | U | ✅ |
| CL.2 | `investigation add` / `add-note` | Artifacts on disk | U | ✅ |
| CL.3 | `investigation list` | Lists child investigations | U | ✅ |
| CL.4 | `investigation open` | Prints entry path + type | U | ✅ |

---

# 6. Security (S1)

| ID | Scenario | Expected | Test | Status |
|----|----------|----------|------|--------|
| S1.1 | Path traversal | No escape from investigation root / allowed roots | U | ✅ |
| S1.2 | Session isolation | Session B cannot mutate Session A investigation | I | ✅ |
| S1.3 | API key | New routes require key when configured | I | ✅ |

---

# 7. Web SPA (UI)

| ID | Scenario | Expected | Test | Status |
|----|----------|----------|------|--------|
| UI.1 | Panel label | **Investigations** (not Shared workspaces) | B | ⬜ |
| UI.2 | Create + list | User can create and see investigations | B | ⬜ |
| UI.3 | Add log + note | Upload and note form work | B | ⬜ |
| UI.4 | Open + save | Open loads workbench; save updates snapshot | B | ⬜ |

---

# 8. Story Gate (SG)

| ID | Scenario | Expected | Status |
|----|----------|----------|--------|
| SG.1 | Five-minute demo | Create → add log → add note → save → reopen without docs | ✅ |
| SG.2 | No Story 2 creep | No multi-log correlation code in Story 1 PR | ✅ |

---

# 9. Release (REL)

| ID | Scenario | Expected | Status |
|----|----------|----------|--------|
| REL.1 | CI green | All scenario rows pass on `web` job | ✅ |
| REL.2 | Tag + binaries | `v2.3.0` GitHub Release with `logscope-web-v2.3.0-*` | ⬜ |

---

## Document history

| Version | Date | Change |
|---------|------|--------|
| 0.1.0 | 04-08-2026 | Initial Story 1 scenario matrix |
