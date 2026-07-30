# M14 — Desktop CLI Parity Gaps

| Field | Value |
|-------|-------|
| Document | M14 Desktop CLI Parity Gaps |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | Approved |
| Created | 30-07-2026 |
| Last Updated | 30-07-2026 |

---

# 1. Purpose

M14 shipped **workflow parity** (open → analyze → investigate → analytics → export → sessions → extensions → AI → tail) via shared `ApplicationService`. This document tracks **remaining CLI surface** not yet exposed in `logscope-desktop`, prioritized for a **desktop polish** track before or alongside M15 Web.

**Baseline:** `v2.0.1` desktop release.

---

# 2. Legend

| Priority | Meaning |
|----------|---------|
| **P0** | Broken or misleading UI (fields exist but do nothing) |
| **P1** | High-value parity; frequent CLI workflows |
| **P2** | Power-user / maintainer; lower GUI demand |

| Effort | Meaning |
|--------|---------|
| **S** | ~1 day |
| **M** | ~2–4 days |
| **L** | ~1 week+ |

---

# 3. Gap matrix

| ID | Priority | Gap | CLI today | Desktop today | Proposed UI | Effort | Status |
|----|----------|-----|-----------|---------------|-------------|--------|--------|
| GAP.1 | **P0** | Time range filters | `--time-from` / `--time-to` on investigate, search, session | From/To wired to `InvestigationCriteria.timeRange` | Parse with `Timestamp::parse`; apply in `runInvestigate()` | S | ✅ |
| GAP.2 | **P1** | Report section picker | `analyze --sections`, session `--sections` | Export dialog: format + section checkboxes | Checklist in Export dialog (executive, summary, levels, errors, analytics, …) | M | ✅ |
| GAP.3 | **P1** | Persist / reuse index | `--persist-index`, `--reuse-index` | Toolbar **Persist index** / **Reuse index** on Analyze | Analyze toolbar checkboxes; `buildAnalysisConfigForDesktop()` | M | ✅ |
| GAP.4 | **P1** | Configuration editor | Edit `.properties` keys | Load file only (`--config`, File → Load Configuration…) | Settings dialog: view/edit keys, save as, validate on save | L | ⬜ |
| GAP.5 | **P1** | `config validate` | `logscope config validate` | Validate on Load Configuration via `ApplicationService::validateConfiguration()` | Settings → Validate, or validate after Load Configuration | S | ✅ |
| GAP.6 | **P2** | Extension describe | `extensions describe <id>` | List + detail panel (`describeExtension`) | Double-click extension → detail panel (metadata from `describeExtension`) | S | ✅ |
| GAP.7 | **P2** | Full `--stats` parity | `--stats` on analyze, investigate, agent | **View → Run Statistics…** (`RunStatsDialog`, CLI `printRunStats`) | Status bar expansion or Stats dialog (`AnalysisStats`, memory, plugin load) | M | ✅ |
| GAP.8 | **P2** | Log format / profile | `--log-format`, `--profile` | Config + auto-detect only | Open dialog: format auto/plain/jsonl, profile dropdown | S | ⬜ |
| GAP.9 | **P2** | Session save filters | `session save` with `--min-errors`, `--filter`, `--sections`, … | Basic save (model + default report options) | Save Session dialog mirroring session CLI flags | M | ⬜ |
| GAP.10 | **P2** | Stdin analyze | `analyze -` | File picker only | Optional “Open from clipboard / pipe” or document as CLI-only | S | ⬜ |
| GAP.11 | **P2** | Standalone search/query | `search`, `query` commands | Merged into Investigate bar | No change needed (document parity); optional menu aliases | — | — |

---

# 4. Recommended delivery phases

## Phase A — `v2.0.2` desktop polish (P0 + quick P1) ✅ Shipped in tree

| Item | Rationale |
|------|-----------|
| GAP.1 Time range wiring | UI already shows From/To; users expect it to work |
| GAP.5 Config validate on load | Cheap trust signal when loading properties |
| GAP.6 Extension describe | Small UX win for plugin authors |

**Target:** patch release `v2.0.2`; no API breaks.

## Phase B — `v2.0.2` desktop polish (P1 depth) ✅ Shipped in tree

| Item | Rationale |
|------|-----------|
| GAP.2 Report section picker | Matches CLI export flexibility |
| GAP.3 Persist / reuse index | Large-log workflows (M11 storage) |
| GAP.7 Stats panel | Investigator feedback parity with `--stats` |

## Phase C — Post-M14 / pre-M15 (P1–P2)

| Item | Rationale |
|------|-----------|
| GAP.4 Configuration editor | Full “dynamic configure” without external editor |
| GAP.8 Format/profile pickers | Edge formats without hand-editing config |
| GAP.9 Session save options | Power users saving filtered investigations |
| GAP.10 Stdin | Low priority; CLI remains better for pipes |

---

# 5. Implementation notes

## GAP.1 — Time range (P0)

Wire `main_window.cpp` `runInvestigate()`:

```cpp
if (!m_timeFromEdit->text().isEmpty()) {
    const auto ts = foundation::Timestamp::parse(m_timeFromEdit->text().toStdString());
    if (ts) criteria.timeRange = criteria.timeRange.withEarliest(*ts);
}
// same for --time-to / m_timeToEdit
```

Reuse CLI parsing rules ([`cli_parser.cpp`](../../apps/cli/cli_parser.cpp) `--time-from` / `--time-to`).

## GAP.2 — Sections

Use `reporting::ReportSections::parse()` in export dialog; set `ReportOptions.sections` before `generateReport()`. Default from `configurationManager().configuration()` key `report.sections` when unset.

## GAP.3 — Index persistence

Mirror CLI flags in `resolveAnalysisConfig()` / `OpenOptions` — desktop analyze path already calls `resolveAnalysisConfig(m_service.configurationManager().configuration(), …)`. Add UI toggles that set ephemeral overrides or temporary config keys before analyze.

## GAP.4 — Config editor

Read/write via `ConfigurationManager`; avoid duplicating parser — reuse `configuration` module load/save. Consider read-only keys list from [CONFIGURATION_GUIDE.md](../handbook/CONFIGURATION_GUIDE.md).

---

# 6. What already matches CLI

| Capability | Shared path |
|------------|-------------|
| Analyze / investigate / analytics / report | `ApplicationService` |
| Dynamic plugins + AI providers | `loadConfiguration(.properties)` |
| Sessions list/save/load | `ApplicationService` session APIs |
| AI ask / summarize / hints | `agentInvestigate()` |
| Live tail | `startTail()` / `TailingFileLogSource` |

---

# 7. Non-goals (unchanged)

- Plugin marketplace / install UX
- Replacing CLI for scripting and CI
- M15 Web UI (separate milestone)

---

# 8. Acceptance (Phase A)

| Scenario | Expected |
|----------|----------|
| Time range | From/To ISO timestamps filter same rows as CLI `--time-from` / `--time-to` |
| Config load | Invalid properties file shows validate errors when GAP.5 shipped |
| Extension | Selecting extension shows describe metadata when GAP.6 shipped |

---

# 9. Acceptance (Phase B)

| Scenario | Expected |
|----------|----------|
| Export sections | Export dialog sections match CLI `--sections`; defaults from `report.sections` config |
| Persist / reuse | Toolbar toggles pass `--persist-index` / `--reuse-index` equivalent through analyze |
| Run stats | View → Run Statistics shows same block as CLI `--stats` after Analyze |

---

# 10. Related documents

| Document | Role |
|----------|------|
| [M14-DESKTOP-APPLICATION.md](M14-DESKTOP-APPLICATION.md) | M14 scope and deliverables |
| [M14-V200-DESKTOP-SCENARIOS.md](M14-V200-DESKTOP-SCENARIOS.md) | Shipped acceptance scenarios |
| [CLI_REFERENCE.md](../handbook/CLI_REFERENCE.md) | CLI flag reference |
| [apps/desktop/README.md](../../apps/desktop/README.md) | Desktop build and run |

---

# 11. Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 30-07-2026 | Initial prioritized gap list after v2.0.1 desktop release. |
| 1.1.0 | 30-07-2026 | Phase B complete: GAP.2, GAP.3, GAP.7. |
