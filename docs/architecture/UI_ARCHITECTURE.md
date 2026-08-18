# Investigation UI architecture

| Field | Value |
|-------|--------|
| Document | Investigation UI architecture |
| Category | Architecture |
| Status | Approved |
| Audience | Architects, implementers, testers, AI agents |
| Created | 06-08-2026 |

---

## One principle

> **Desktop and Web are two presentations of the same Investigation experience — not two different products.**

**One Investigation UI, two shells.** Same domain, workflows, terminology, and navigation — only rendering differs.

**Same product. Same workflow. Same terminology. Different shell.**

---

## Layering

```text
                LogScope Core
                     │
     ┌───────────────┼───────────────┐
     │               │               │
 Investigation   REST API      CLI Commands
     │
     ▼
 View Models (target)
     │
 ┌───┴───────────────┐
 │                   │
Desktop shell    Web shell
```

| Layer | Responsibility |
|-------|----------------|
| **Core** | Investigations, artifacts, analyze, timeline, crash, investigate |
| **Orchestration** | `ApplicationService` — shared by CLI, REST, desktop |
| **View models** | Shell-agnostic investigation state and commands (roadmap) |
| **Shells** | Qt (`apps/desktop/`) and SPA (`apps/web/ui/dist/`) — render only |

Do not duplicate business logic in `app.js` or Qt UI code.

---

## Investigation modes (not pages)

```text
Investigation
│
├── Artifacts
├── Timeline
├── Crash
├── Results
└── AI
```

Modes are **tabs/views** over one investigation. New stories add modes on **both shells** when user-visible.

| Mode | Web | Desktop |
|------|-----|---------|
| Artifacts + viewer | ✅ | ✅ (`v2.11.0`) |
| Timeline | ✅ | ✅ (`v2.11.0`) |
| Crash | ✅ | ✅ (`v2.11.0`) |
| Results | ✅ | ✅ |
| AI | ✅ | ✅ |

---

## Parity & testing

One scenario path for both shells:

```text
Create Investigation → Add Log → Analyze → Timeline → Crash
```

- Web: Playwright [`tests/e2e/web/`](../../tests/e2e/web/README.md)
- Desktop: `logscope_desktop_parity_test` (P2 Story Gate), `logscope_desktop_tests` (headless Qt)

Use **identical** user-facing labels (Investigation, Artifacts, Timeline, Crash, Results, AI, Open, Analyze, Investigate).

---

## Anti-patterns

- Web as admin dashboard; desktop as separate power-user product
- Different workflows or tab names per shell
- User-visible features on one shell without tracked parity work
- Logic in presentation layer instead of core / `ApplicationService`

Visual/layout rules: [WEB_UI_DESIGN.md](../handbook/WEB_UI_DESIGN.md).

---

## View model roadmap

Target: `InvestigationViewModel` between core and shells — same state for desktop, web, and future surfaces (mobile, embed).

Until then: `ApplicationService` + REST JSON shapes are the **contract** both shells must honor.

---

## Agent checklist

- [ ] New **mode/tab**, not a new product area?
- [ ] Web **and** desktop updated (or gap tracked)?
- [ ] Core owns logic; shells bind only?
- [ ] Both test suites updated?

---

## Related

- [WEB_UI_DESIGN.md](../handbook/WEB_UI_DESIGN.md) — IDE layout, sync mandate
- [CODE_MAP.md](../handbook/CODE_MAP.md) — paths
- [ADR-009 Web Platform REST](decisions/ADR-009-Web-Platform-REST.md)
