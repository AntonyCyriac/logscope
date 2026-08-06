# AI assistant guide (LogScope)

Tool-agnostic bootstrap for **any** coding assistant. No vendor-specific setup or repo config required.

## Start here

At the beginning of a session, load (attach or reference):

| Order | Document | Purpose |
|-------|----------|---------|
| 1 | [docs/handbook/PROJECT_CONTEXT.md](docs/handbook/PROJECT_CONTEXT.md) | Engineering mindset, release, milestones, CI rules |
| 2 | [docs/handbook/CODE_MAP.md](docs/handbook/CODE_MAP.md) | Where CLI, desktop, web, core, and tests live |
| 3 | [docs/release/v2.5.0-RELEASE-NOTES.md](docs/release/v2.5.0-RELEASE-NOTES.md) | Current shipped release (update when tagging) |

Optional: [docs/DOCUMENT_MAP.md](docs/DOCUMENT_MAP.md) · [docs/ROADMAP.md](docs/ROADMAP.md) · [CHANGELOG.md](CHANGELOG.md)

## Principles

- **Living product** — LogScope evolves every day; pin a [release tag](https://github.com/AntonyCyriac/logscope/releases) for stability.
- **Architecture-first** — extend the existing pipeline; do not bypass layers or weaken CI.
- **Public repo only** — code, tests, ADRs, and handbook live here. Do not paste private strategy, EngOS gate labels (G0–G5), or unreleased horizon (enterprise/cloud) into public PRs.
- **No tool branding in git** — do not add vendor footers (e.g. “Made with …”) to commit messages, PR descriptions, or release notes.
- **Shipped-tag regressions** — user-visible bugs in a released `vX.Y.Z` need a [GitHub issue](https://github.com/AntonyCyriac/logscope/issues) before or with the fix (label `bug`). See [Release process — regression issues](docs/release/RELEASE.md#release-regression-issues-required).

## Product vs assistant (do not conflate)

| Layer | Meaning |
|-------|---------|
| **This guide** | Instructions for an AI assistant helping **build** LogScope |
| **LogScope product AI** | User-facing `logscope agent investigate`, summaries, NL queries (`AiProvider`, M13+) |

Future multi-agent product vision: [PRD-001](docs/requirements/future/PRD-001-AI-Engineering-Agents.md) (not current milestone scope).

## Web & desktop UI (IDE for investigations)

LogScope is an **investigation platform**, not a monitoring dashboard. Before changing web or desktop UI, read:

1. [docs/architecture/UI_ARCHITECTURE.md](docs/architecture/UI_ARCHITECTURE.md) — **one Investigation UI, two shells** (view models, modes, parity)
2. [docs/handbook/WEB_UI_DESIGN.md](docs/handbook/WEB_UI_DESIGN.md) — IDE layout and shell guidelines

| Principle | Rule |
|-----------|------|
| Identity | IDE-like (VS Code / Cursor) — calm, one focus area |
| Layout | Left artifacts · center work area · bottom tabs (Timeline \| Crash \| AI \| Results) |
| Disclosure | One visible panel; story features via tabs, not more vertical panels |
| Actions | Open / Analyze / Investigate on **artifacts**, not global Investigate sections |
| Tests | Prefer Playwright [`tests/e2e/web/`](tests/e2e/web/) + `data-testid` hooks |
| **Web ↔ desktop sync** | **One Investigation UI, two shells** — same modes, terminology, workflows; ship both or track gap. See [UI_ARCHITECTURE.md](docs/architecture/UI_ARCHITECTURE.md). |

Desktop bottom tabs: **Results \| AI \| Analytics** (Timeline/Crash web-only until shipped on Qt — tracked gap).

## Web UI browser automation

For Story Gate or SPA demos on `logscope-web`, prefer **Playwright** ([`tests/e2e/web/README.md`](tests/e2e/web/README.md)). For manual checks, read [docs/handbook/BROWSER_MCP.md](docs/handbook/BROWSER_MCP.md) **before** using `cursor-ide-browser`. Never create orphan tabs with `browser_tabs new` alone — use `browser_navigate` to `http://127.0.0.1:8080` or lock an existing tab first.
