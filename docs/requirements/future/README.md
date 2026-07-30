# Future Requirements

| Field | Value |
|-------|-------|
| Category | Requirements (Future) |
| Status | Vision / not committed for near-term implementation |
| Last Updated | 30-07-2026 |

---

# Purpose

This directory holds **public future vision** documents. They describe capabilities that are not yet shipped and may require ADRs, milestone plans, and phased delivery before implementation.

They are **not** current engineering requirements. For shipped behavior, see `requirements/functional/` and `requirements/non_functional/`.

---

# Documents

| ID | Document | Summary |
|----|----------|---------|
| PRD-001 | [PRD-001-AI-Engineering-Agents.md](PRD-001-AI-Engineering-Agents.md) | Long-term AI agent platform vision (IDE workflow + future `logscope agent` CLI) |

---

# Relationship to AI Assistants Today

LogScope development already uses AI assistants (Cursor, Claude Code, Copilot) with public context such as [PROJECT_CONTEXT.md](../../handbook/PROJECT_CONTEXT.md).

Detailed agent instructions, long-horizon product vision, and theme research are maintained in a **private strategy repository** (not linked publicly). That operational layer is for **building LogScope**; it is separate from the **product** agent features described in PRD-001.

| Layer | What it is | Where |
|-------|------------|-------|
| **Operational (today)** | Constitution-style prompts for AI IDEs assisting development | Private strategy repo `.ai/` |
| **Product (future)** | Runtime agents invoked by users (`logscope agent …`, investigation AI) | PRD-001; delivery via M13+ |

Do not treat PRD-001 as an implementation checklist for the next milestone. Near-term public work is defined in [ROADMAP.md](../../ROADMAP.md) (M15 Web Platform; M14 shipped at `v2.0.0` / `v2.0.1`).

---

# Graduation Path

When a future requirement becomes committed work:

1. Publish or update a milestone plan under `docs/planning/Mn-*.md`.
2. Add ADRs where architecture gates apply (AI integration requires ADR per strategic roadmap).
3. Move or split stable requirements into `functional/` or `non_functional/` as appropriate.
4. Keep PRD vision sections that remain post-milestone in this directory.

---

# Related Documents

| Document | Use when |
|----------|----------|
| [ROADMAP.md](../../ROADMAP.md) | Near-term milestones (M14) |
| [POST_V1_STRATEGIC_ROADMAP.md](../../planning/POST_V1_STRATEGIC_ROADMAP.md) | Phased post-v1 strategy |
| [PROJECT_CONTEXT.md](../../handbook/PROJECT_CONTEXT.md) | Public agent session bootstrap |
| [PRD-001-AI-Engineering-Agents.md](PRD-001-AI-Engineering-Agents.md) | Full AI agent platform vision |
