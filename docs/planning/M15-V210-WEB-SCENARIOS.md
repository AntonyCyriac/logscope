# M15 v2.1.0 — Web Platform Scenarios

| Field | Value |
|-------|-------|
| Document | M15 v2.1.0 Web Scenarios |
| Category | Project Planning |
| Version | 0.2.0 |
| Status | Approved — v2.1.0 release gate |
| Created | 30-07-2026 |
| Last Updated | 31-07-2026 |

---

# 1. Purpose

Acceptance scenarios for **v2.1.0** — M15 Web Platform (REST API + browser MVP).

**Target release:** `v2.1.0`

See [M15-WEB-PLATFORM.md](M15-WEB-PLATFORM.md), [ADR-009](../architecture/decisions/ADR-009-Web-Platform-REST.md).

---

# 2. Legend

Status: ⬜ planned · 🟡 in progress · ✅ complete

| Column | Meaning |
|--------|---------|
| **Parity** | REST/UI result matches CLI `--format json` on `samples/` |
| **Test** | U = unit, I = integration, P = parity, W = web smoke, B = browser (optional) |

---

# 3. Application layer (L1) — REST over ApplicationService

| ID | Scenario | Expected | Test | Status |
|----|----------|----------|------|--------|
| L1.1 | Health | `GET /api/v1/health` returns version | U | ✅ |
| L1.2 | Upload source | Multipart upload `sample.log` → open succeeds | I | ✅ |
| L1.3 | Analyze | Returns line counts matching CLI analyze JSON | U+I+P | ✅ |
| L1.4 | Investigate filters | Same `matchCount` as CLI `investigate` | U+I+P | ✅ |
| L1.5 | Analytics | Same bucket counts as CLI `analytics` | U+P | ✅ |
| L1.6 | Session save/load | Round-trip reproduces report sections | I | ✅ |
| L1.7 | Path traversal guard | `sources/open` with `..` rejected | U | ✅ |
| L1.8 | Upload size limit | Payload over `web.max_upload_bytes` → 413 | U | ✅ |

---

# 4. REST API (R1)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| R1.1 | Create session | First request / explicit create | `X-LogScope-Session` issued | I | ✅ |
| R1.2 | API key optional | `web.api_key` set | Missing key → 401 | U+I | ✅ |
| R1.3 | List extensions | `GET /extensions` | Lists registered extensions | I | ✅ |
| R1.4 | Agent investigate | POST ask `errors` + noop AI | Match count = CLI agent | I+P | ✅ |
| R1.5 | Export HTML | POST export `format=html` | Non-empty HTML body | I | ✅ |
| R1.6 | Export PDF | POST export `format=pdf` | Valid PDF bytes | I | ✅ |
| R1.7 | Large log smoke | Open `large-app.log` | Analyze within CI time budget | I | ✅ |

---

# 5. Browser MVP (W1) — M15.2

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| W1.1 | Launch | Navigate to `/` | SPA shell loads | W | ✅ |
| W1.2 | Upload file | Upload `sample.log` | Table populated | W+B | ✅ |
| W1.3 | Analyze | Click Analyze | Level counts visible | W+Parity | ✅ |
| W1.4 | Search | Search box `error` | Filtered rows match CLI `--search` | W+Parity | ✅ |
| W1.5 | Filter DSL | `level == ERROR` | Same as CLI `--filter` | W+Parity | ✅ |
| W1.6 | Export | Export HTML | File download succeeds | W | ✅ |
| W1.7 | Extensions panel | Open extensions view | Lists plugins | W | ✅ |
| W1.8 | AI Ask | Ask `errors` with noop provider | Results update | W | ✅ |

---

# 6. Security (S1)

| ID | Scenario | Expected | Test | Status |
|----|----------|----------|------|--------|
| S1.1 | Default bind | Listens on `127.0.0.1` only unless configured | U | ✅ |
| S1.2 | Server path disabled | `sources/open` without `allow_server_paths` → 403 | U | ✅ |
| S1.3 | Session isolation | Cross-session model access denied | I | ✅ |

---

# 7. Release gate

| ID | Scenario | Expected | Status |
|----|----------|----------|--------|
| REL.1 | L1 + R1 parity rows | Pass on CI `web` job | 🟡 |
| REL.2 | W1 manual / optional Playwright | MVP checklist before tag | ✅ |
| REL.3 | `v2.1.0` tag + GitHub Release | Published | ⬜ |
| REL.4 | Release regression policy | Issues filed per [RELEASE.md §8](../release/RELEASE.md#8-post-release-housekeeping) | ✅ |

---

# 8. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 0.1.0 | 30-07-2026 | Initial scenario matrix; ADR-009 acceptance traceability. |
| 0.2.0 | 31-07-2026 | M15.1 + M15.2 complete for v2.1.0; scenario status updated. |
