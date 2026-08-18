# Web & desktop UI design (agents)

| Field | Value |
|-------|--------|
| Document | Web & desktop UI design |
| Category | Handbook |
| Status | Approved |
| Audience | AI assistants, implementers, testers |
| Last Updated | 06-08-2026 |

**Architecture:** This document covers **shell layout and visual rules**. For the full **one Investigation UI, two shells** model (view models, modes, parity), read [UI_ARCHITECTURE.md](../architecture/UI_ARCHITECTURE.md) first.

---

## One sentence

> **LogScope should feel like an IDE for investigations, not a monitoring dashboard.**

Users are **engineers debugging problems**, not operators watching Grafana/Kibana/Splunk-style dashboards.

**Reference surfaces:** VS Code, Cursor, GitHub, CLIs — calm, focused, one thing at a time.

---

## Anti-patterns (do not build)

| Avoid | Why |
|-------|-----|
| Many panels visible at once (Sources, Investigation, Artifacts, Crash, Timeline, Investigate, AI, Results, Tail…) | Overwhelming; user can only focus on one thing |
| Enterprise dashboard widgets, metric cards, chart grids | Wrong product identity |
| Global Investigate / Analyze sections detached from artifacts | Actions should be contextual |
| Vertical stacking of every story feature | UI grows through **tabs**, not more rows of panels |
| Bright “ops center” visual noise | Prefer IDE chrome: subtle borders, monospace content, restrained color |

---

## Layout (web SPA)

Three-pane **Explorer | Editor** pattern:

```text
+-----------------------------------------------------------+
| LogScope | Investigation: <name>              [Analyze]   |
+-----------------------------------------------------------+
| Artifacts (left)     | Main work area (center)             |
| 📄 app.log           | Selected artifact content           |
| per-row: Open /      | (log lines, pstack, note)           |
| Analyze / Investigate|                                   |
+----------------------+-------------------------------------+
| Bottom dock (one tab visible): Timeline | Crash | AI | Results |
+-----------------------------------------------------------+
```

| Pane | Role |
|------|------|
| **Header** | Brand, investigation name, New/Open Investigation, global **Analyze** |
| **Left sidebar** | Artifact list with type icons; add-artifact controls; collapsible session source (upload/path, tail) |
| **Center** | Single work area — selected artifact body; line highlight on timeline/crash jump |
| **Bottom dock** | **One panel at a time** — Timeline, Crash, AI, Results; collapsed by default until tab clicked |

### Progressive disclosure by story

| Story | Minimum UI |
|-------|------------|
| **1–2** | Artifacts → select → center viewer |
| **3** | + **Timeline** bottom tab |
| **4** | + **Crash** bottom tab |
| **AI / Results** | Bottom tabs; enabled after analyze where applicable |

Tabs stay in the bar but may be **disabled** until data exists — do not hide the pattern.

---

## Interaction rules

1. **One visible focus** — center OR one bottom panel expanded, not both fighting for attention (bottom dock collapses when not needed).
2. **Contextual actions** — **Open**, **Analyze**, **Investigate** on each artifact row (or action menu), not a global Investigate panel.
3. **AI** — single line: “Ask about this investigation…” (Cursor-style) in the **AI** tab, not a large dedicated section.
4. **Results** — search/filter/investigate/export live in the **Results** tab; REST JSON uses `matches` (map to table rows in UI).
5. **Analyze** — header action runs session-wide analyze; may auto-run investigate with default `search: error` when inputs empty.
6. **`data-testid`** — add stable hooks on new interactive elements for Playwright (`tests/e2e/web/`).

---

## Desktop alignment (Qt)

Desktop (`apps/desktop/main_window.cpp`) follows the same philosophy where features exist:

| Web | Desktop (`v2.11.0`) |
|-----|---------------------|
| Left artifacts | Left **Artifacts** sidebar (investigation mode) or **Workspace** (session mode) |
| Center artifact viewer | **Results** tab: log table + filters |
| Bottom Timeline / Crash | **Timeline** · **Crash** tabs |
| Bottom AI | **AI** tab |
| — | **Analytics** tab (desktop-only until projected on web) |

Use **QTabWidget** for bottom area (**Timeline · Crash · Results · AI · Analytics** in investigation mode), not side-by-side split panels.

**P2.1 deferred on desktop:** Related Evidence panel, evidence-link UI, correlation suggestions UI.

---

## Web ↔ desktop sync (mandatory for future work)

**Maximize parity.** Web SPA and Qt desktop are two views of the same investigation product — not independent UIs.

### Default rule

When a change is **user-visible** (new tab, artifact action, navigation, analyze/investigate flow, labels, keyboard path):

1. **Implement on both surfaces** in the same milestone/PR when feasible (`apps/web/ui/dist/` **and** `apps/desktop/`).
2. **Reuse the same orchestration** — `ApplicationService` and domain APIs; no duplicate business logic in SPA or Qt.
3. **Match structure** — same tab names, same artifact actions (Open / Analyze / Investigate), same progressive disclosure (bottom tabs, not extra panels).
4. **Test both** — extend Playwright (`tests/e2e/web/`) **and** headless desktop tests (`logscope_desktop_tests`, `logscope_desktop_parity_test`) for the shared flow.

### When one surface must lag

Only ship web-only or desktop-only UI if:

- Platform constraint is documented (e.g. GDB/core crash tooling on web server vs desktop packaging), **and**
- Gap is recorded in `tests/e2e/web/README.md` (desktop parity) or Story scenario matrix, **and**
- A follow-up task exists (issue or next story) — **never silent drift**.

### Agent PR checklist (presentation changes)

- [ ] Updated **web** SPA (`apps/web/ui/dist/`)?
- [ ] Updated **desktop** (`apps/desktop/main_window.cpp`, panels) with equivalent tab/action?
- [ ] Same user-facing names and workflow order on both?
- [ ] Playwright + desktop GUI tests extended (or N/A justified in PR)?
- [ ] Parity gap documented if intentionally deferred?

**Anti-pattern:** Adding Timeline/Crash/Results behavior on web only while desktop keeps an old toolbar layout “for later” without a tracked gap.

---

## Assets & tests

| Path | Purpose |
|------|---------|
| `apps/web/ui/dist/` | Built SPA (`index.html`, `app.js`, `styles.css`) — no separate `src/` |
| `tests/e2e/web/` | Playwright Story Gate specs — **prefer over manual browser MCP** for regression |
| `docs/handbook/BROWSER_MCP.md` | Manual/demo browser workflow when Playwright is insufficient |
| `samples/demo-story-gate.properties` | E2E + demo server config |

**Verification order:** Playwright E2E → web integration tests → optional browser MCP for exploratory UX.

---

## Agent checklist (before changing UI)

- [ ] Does this add another always-visible panel? If yes, move it to a bottom tab or artifact action.
- [ ] Can the user complete Story 1 with only Artifacts + center viewer?
- [ ] Are new controls reachable without scrolling past unrelated sections?
- [ ] Did you add `data-testid` for new buttons, tabs, and dynamic rows?
- [ ] Did you run or extend `tests/e2e/web/` for user-visible flows?
- [ ] **Web ↔ desktop:** Did you update **both** SPA and Qt for the same user-visible behavior (or document a tracked parity gap)?
- [ ] Desktop: if overlapping feature, update bottom tabs — don't add a fourth always-visible column.

---

## Related docs

- [UI_ARCHITECTURE.md](../architecture/UI_ARCHITECTURE.md) — one Investigation UI, two shells; view model direction
- [AGENTS.md](../../AGENTS.md) — assistant bootstrap
- [CODE_MAP.md](CODE_MAP.md) — file locations
- [BROWSER_MCP.md](BROWSER_MCP.md) — manual browser demos
- [TESTING.md](../testing/TESTING.md) — Playwright section
