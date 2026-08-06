# Web & desktop UI design (agents)

| Field | Value |
|-------|--------|
| Document | Web & desktop UI design |
| Category | Handbook |
| Status | Approved |
| Audience | AI assistants, implementers, testers |
| Last Updated | 06-08-2026 |

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

| Web | Desktop |
|-----|---------|
| Left artifacts | Left **Workspace** navigator (sessions / extensions) |
| Center artifact viewer | **Results** tab: log table + filters |
| Bottom Timeline / Crash | *Not yet on desktop* |
| Bottom AI | **AI** tab |
| — | **Analytics** tab (desktop-only until projected on web) |

Use **QTabWidget** for bottom area (Results | AI | Analytics), not side-by-side split panels.

When adding a web bottom-tab feature, consider desktop tab parity in the same milestone or document the gap in `tests/e2e/web/README.md`.

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
- [ ] Desktop: if overlapping feature, update bottom tabs — don't add a fourth always-visible column.

---

## Related docs

- [AGENTS.md](../../AGENTS.md) — assistant bootstrap
- [CODE_MAP.md](CODE_MAP.md) — file locations
- [BROWSER_MCP.md](BROWSER_MCP.md) — manual browser demos
- [TESTING.md](../testing/TESTING.md) — Playwright section
