# AI assistant guide (LogScope)

Tool-agnostic bootstrap for **any** coding assistant (Cursor, GitHub Copilot, Claude Code, Codex, Windsurf, etc.). No vendor-specific setup required.

## Start here

At the beginning of a session, load (attach or `@`-reference):

| Order | Document | Purpose |
|-------|----------|---------|
| 1 | [docs/handbook/PROJECT_CONTEXT.md](docs/handbook/PROJECT_CONTEXT.md) | Engineering mindset, release, milestones, CI rules |
| 2 | [docs/handbook/CODE_MAP.md](docs/handbook/CODE_MAP.md) | Where CLI, desktop, web, core, and tests live |
| 3 | [docs/release/v2.2.1-RELEASE-NOTES.md](docs/release/v2.2.1-RELEASE-NOTES.md) | Current shipped release (update when tagging) |

Optional: [docs/DOCUMENT_MAP.md](docs/DOCUMENT_MAP.md) · [docs/ROADMAP.md](docs/ROADMAP.md) · [CHANGELOG.md](CHANGELOG.md)

## Principles

- **Living product** — LogScope evolves every day; pin a [release tag](https://github.com/AntonyCyriac/logscope/releases) for stability.
- **Architecture-first** — extend the existing pipeline; do not bypass layers or weaken CI.
- **Public repo only** — code, tests, ADRs, and handbook live here. Do not paste private strategy or unreleased planning into public PRs.
- **Shipped-tag regressions** — user-visible bugs in a released `vX.Y.Z` need a [GitHub issue](https://github.com/AntonyCyriac/logscope/issues) before or with the fix (label `bug`). See `.cursor/rules/release-regression-issues.mdc` when using Cursor.

## Product vs assistant (do not conflate)

| Layer | Meaning |
|-------|---------|
| **This guide** | Instructions for an AI assistant helping **build** LogScope |
| **LogScope product AI** | User-facing `logscope agent investigate`, summaries, NL queries (`AiProvider`, M13+) |

Future multi-agent product vision: [PRD-001](docs/requirements/future/PRD-001-AI-Engineering-Agents.md) (not current milestone scope).

## Cursor users

Rules in [`.cursor/rules/`](.cursor/rules/) mirror this file. Prefer **this `AGENTS.md`** when sharing context across tools.
