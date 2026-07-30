# Phase 1 v1.5.2 — Stabilization Scenarios

| Field | Value |
|-------|-------|
| Document | Phase 1 v1.5.2 Stabilization Scenarios |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | In progress |
| Created | 30-07-2026 |
| Last Updated | 30-07-2026 |

---

# 1. Purpose

Acceptance scenarios for **v1.5.2** — Phase 1 stabilization. Every row must pass before release and before M14 design starts.

**Target release:** `v1.5.2`

**Feature areas:**

| ID | Area |
|----|------|
| D1–D3 | Documentation |
| R1–R2 | Regression and flaky fixes |
| S1 | Stress / CLI matrix |
| F1 | Fuzz |
| O1–O3 | Observability |
| E1–E2 | Engineering CI |

See [PHASE-1-STABILIZATION.md](PHASE-1-STABILIZATION.md).

---

# 2. Legend

| Column | Meaning |
|--------|---------|
| **Regression** | Dedicated regression or e2e guard |
| **Test layer** | U = unit, I = integration, E = e2e |

Status: ⬜ planned · 🟡 in progress · ✅ complete

---

# 3. Documentation (D1–D3)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| D1.1 | Tutorials exist | `docs/tutorials/` | At least 3 tutorial files; linked from README | Doc | | ✅ |
| D1.2 | Tutorial index | README / User Manual | Links to tutorials | Doc | | ✅ |
| D2.1 | Component diagram | Open COMPONENT_CATALOG | Includes `scope_ai`, storage, plugin paths | Doc | | ✅ |
| D3.1 | API docs CI | Push to master | `api-docs` artifact uploaded | CI | Yes | ⬜ |
| D3.2 | API docs policy | docs/api/README | Pages or artifact-only documented | Doc | | ✅ |

---

# 4. Regression (R1–R2)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| R1.1 | AI isolation | HTTP/AI summarize fails | Investigation output still printed | U+R | Yes | ✅ |
| R1.2 | Plugin isolation | Invalid plugin in path | Analyze succeeds; plugin skipped | U+R | Yes | ✅ |
| R2.1 | Incremental append | `UpdatesFingerprintOnFinalize` on Windows CI | Passes reliably | U | Yes | ✅ |

---

# 5. Stress and e2e (S1)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| S1.1 | Agent in matrix | CLI matrix job | `agent investigate` with noop config exit 0 | CI | Yes | ✅ |
| S1.2 | Session e2e | `session save` + `load` | Exit 0; report reproduced | E | | ✅ |

---

# 6. Fuzz (F1)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| F1.1 | Query parser fuzz | CI fuzz job | New target runs 1000+ iterations without crash | CI | Yes | ⬜ |

---

# 7. Observability (O1–O3)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| O1.1 | Analyze stats | `analyze --stats sample.log` | Duration, line count in output | U+E | Yes | ⬜ |
| O2.1 | Plugin stats | `analyze --stats` with plugins enabled | Load counts/time when applicable | U+I | | ⬜ |
| O3.1 | Memory snapshot | `--stats` on supported OS | RSS field present | U | | ⬜ |

---

# 8. Engineering CI (E1–E2)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| E1.1 | License doc | THIRD_PARTY_LICENSES.md | Lists FetchContent deps | Doc | | ⬜ |
| E1.2 | License CI | CI license job | Passes on master | CI | Yes | ⬜ |
| E2.1 | ASan Linux | sanitizer job | Green with leak detection | CI | Yes | ⬜ |

---

# 9. Release gate

| ID | Scenario | Expected | Status |
|----|----------|----------|--------|
| REL.1 | All scenario rows above | ✅ complete | ⬜ |
| REL.2 | `v1.5.2` tag + GitHub Release | Published | ⬜ |
| REL.3 | POST_V1 Phase 1 gaps | Closed or accepted partial | ⬜ |
| REL.4 | ROADMAP | M14 next; Phase 1 complete | ⬜ |
