# v2.4.0 — Understand Everything Scenarios

| Field | Value |
|-------|-------|
| Document | v2.4.0 Understand Everything Scenarios |
| Category | Project Planning |
| Version | 0.1.0 |
| Status | **Shipped — `v2.4.0`** |
| Design reference | [ADR-009-M15.6](../architecture/decisions/ADR-009-M15.6-Multi-Source-Investigation.md) |
| Created | 05-08-2026 |
| Last Updated | 05-08-2026 |

---

# 1. Purpose

Acceptance scenarios for **v2.4.0** — Story 2: multi-artifact investigations with **artifact index**, **active log switch**, and `pstack` / `core` storage handlers.

**Demo path (Story Gate):**

```text
Create → app log → syslog → pstack → list → switch syslog → investigate
```

See [V230-CREATE-INVESTIGATION-SCENARIOS.md](V230-CREATE-INVESTIGATION-SCENARIOS.md) (Story 1 baseline).

---

# 2. Legend

Status: ⬜ planned · 🟡 partial / manual · ✅ complete

| Column | Meaning |
|--------|---------|
| **Parity** | REST/CLI behavior matches after open + switch |
| **Test** | U = unit, I = integration, P = parity, W = web smoke, B = browser (manual) |

---

# 3. Multi-source investigation (MS)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| MS.1 | Add second log | `POST .../artifacts` with second log path | Two `log` artifacts; entry unchanged | U+I | ✅ |
| MS.2 | Add pstack | `POST .../artifacts` type `pstack` | Text stored; not openable into session | U+I | ✅ |
| MS.3 | Add core | `POST .../artifacts` type `core` | Binary stored; `metadata.sizeBytes` set | U | ✅ |
| MS.4 | Artifact list | `GET .../investigations/{id}` | `isEntry`, `metadata.role` on artifacts | I | ✅ |
| MS.5 | Switch log | `POST .../open` with non-entry `artifactId` | `loadedFromSnapshot: false`; analyze works | I+P | ✅ |
| MS.6 | Reject non-log open | `POST .../open` with pstack id | **409** `ARTIFACT_NOT_OPENABLE` | I | 🟡 |
| MS.7 | Snapshot discipline | Open entry after save; switch log; reopen entry | Snapshot only on entry/default open | I | ✅ |
| MS.8 | Optional role | Add log with `role: system` | `metadata.role` in manifest | U+I | ✅ |

---

# 4. CLI (CL)

| ID | Scenario | Expected | Test | Status |
|----|----------|----------|------|--------|
| CL.5 | `investigation add --type pstack` | Pstack artifact on disk | U | ✅ |
| CL.6 | `investigation open --artifact` | Prints non-entry log path | U | ✅ |
| CL.7 | Type inference `.core` | Defaults to `core` type | U | ✅ |

---

# 5. Web SPA (UI)

| ID | Scenario | Expected | Test | Status |
|----|----------|----------|------|--------|
| UI.1 | Artifact list | Shows types, entry marker, switch for logs | B | 🟡 |
| UI.2 | Switch log | Switch button loads selected log | B | 🟡 |
| UI.3 | Add pstack | Upload + artifact POST | B | 🟡 |

---

# 6. Story Gate sign-off

| Check | Owner | Status |
|-------|-------|--------|
| Five-minute demo path | Implementer | ✅ |
| Unit + integration tests green | Tester | ✅ |
| ADR-009-M15.6 merged | Architect | ✅ |
| OpenAPI updated | Documentation | ✅ |
