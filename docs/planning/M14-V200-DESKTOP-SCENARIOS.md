# M14 v2.0.0 — Desktop Application Scenarios

| Field | Value |
|-------|-------|
| Document | M14 v2.0.0 Desktop Scenarios |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | Complete |
| Created | 30-07-2026 |
| Last Updated | 30-07-2026 |

---

# 1. Purpose

Acceptance scenarios for **v2.0.0** — M14 Desktop Application.

**Target release:** `v2.0.0`

See [M14-DESKTOP-APPLICATION.md](M14-DESKTOP-APPLICATION.md), [ADR-008](../architecture/decisions/ADR-008-Desktop-Qt-Presentation.md).

---

# 2. Legend

Status: ⬜ planned · 🟡 in progress · ✅ complete

| Column | Meaning |
|--------|---------|
| **Parity** | Desktop action matches CLI output/counts on `samples/` |
| **Test** | U = unit, I = integration, D = desktop smoke |

---

# 3. Application layer (L1)

| ID | Scenario | Expected | Test | Status |
|----|----------|----------|------|--------|
| L1.1 | Open source | `ApplicationService::openSource` returns dataset | U | ✅ |
| L1.2 | Analyze | Returns `AnalysisModel` with line counts | U | ✅ |
| L1.3 | Investigate filters | Same match count as CLI `investigate` | U+I | ✅ |
| L1.4 | Analytics | Same bucket counts as CLI `analytics` | U | ✅ |
| L1.5 | Session save/load | Round-trip reproduces report sections | U+I | ✅ |

---

# 4. Desktop shell (D1)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| D1.1 | Launch | Start `logscope-desktop` | Main window visible | D | ✅ |
| D1.2 | Open file | File → Open `samples/sample.log` | Log table populated | D | ✅ |
| D1.3 | Analyze | Toolbar Analyze | Level counts in status | D+Parity | ✅ |
| D1.4 | Virtualized table | 10k+ lines | Scroll without hang | D | ✅ |

---

# 5. Filters (D2)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| D2.1 | Search | Search box `error` | Filtered rows match CLI `--search` | D+Parity | ✅ |
| D2.2 | Filter DSL | `level == ERROR` | Same as CLI `--filter` | D+Parity | ✅ |
| D2.3 | Level + time | Level error + time range | Matches CLI flags | D | ✅ |

---

# 6. Analytics (D3)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| D3.1 | Timeline tab | Run analytics | Timeline bars visible | D | ✅ |
| D3.2 | Frequency tab | Open sample | Top levels shown | D | ✅ |

---

# 7. Export (D4)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| D4.1 | HTML export | Export HTML | File written; non-empty | D | ✅ |
| D4.2 | PDF export | Export PDF | Binary PDF created | D | ✅ |

---

# 8. Sessions (D5)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| D5.1 | Save session | Session → Save | `.logscope-session` created | D+I | ✅ |
| D5.2 | Load session | Load saved session | Report sections reproduced | D+E | ✅ |

---

# 9. Extensions (D6)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| D6.1 | List plugins | Extensions panel | Lists registered extensions | D | ✅ |
| D6.2 | Bad plugin path | Open with bad path in config | Analyze still succeeds | D+Reg | ✅ |

---

# 10. AI (D7)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| D7.1 | Summarize noop | AI panel + `ai-noop.properties` | Summary text shown | D | ✅ |
| D7.2 | Ask NL | Ask "errors" | Investigate results update | D | ✅ |

---

# 11. Live tail (D8)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| D8.1 | Tail growing file | Append lines to open file | New rows appear | U+D | ✅ |
| D8.2 | Stop tail | Toggle off | No further reads | D | ✅ |

---

# 12. Themes (D9)

| ID | Scenario | Trigger | Expected | Test | Status |
|----|----------|---------|----------|------|--------|
| D9.1 | Dark theme | View → Dark | Palette changes | D | ✅ |
| D9.2 | Light theme | View → Light | Palette changes | D | ✅ |

---

# 13. Release gate

| ID | Scenario | Expected | Status |
|----|----------|----------|--------|
| REL.1 | Scenario rows above | Pass on CI smoke | 🟡 |
| REL.2 | `v2.0.0` tag + GitHub Release | Published | ⬜ |
| REL.3 | Desktop binary in release workflow | Linux/Win/macOS artifacts | ✅ |
