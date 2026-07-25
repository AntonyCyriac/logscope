# M13 – AI Assistant

| Field | Value |
|-------|-------|
| Document | M13 – AI Assistant |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | In progress |
| Created | 25-07-2026 |
| Last Updated | 25-07-2026 |

---

# 1. Purpose

Deliver **M13 – AI Assistant** at **`v1.5.1`**: bounded, assistive AI for log investigation — natural-language queries, investigation summaries, and anomaly hints — via `logscope agent investigate`.

See [ADR-007](../architecture/decisions/ADR-007-AI-Integration.md) and [M13 v1.5.1 Scenarios](M13-V151-AI-SCENARIOS.md).

---

# 2. Dependencies

| Prior milestone | M13 dependency |
|-----------------|----------------|
| M9 — Analytics Engine | Trend, cluster, correlation context for hints |
| M10 — Query Language | NL → filter DSL target ([ADR-004](../architecture/decisions/ADR-004-Query-DSL-Grammar.md)) |
| M11 — Storage Layer | Optional persisted index for large investigations |
| M12 — Dynamic Plugins | Foundation for future AI analyzer plugins (not required for M13 core) |

---

# 3. Phased Delivery

| Phase | Focus | Status |
|-------|-------|--------|
| M13.0 | ADR-007, planning doc, scenarios | ✅ Complete |
| M13.1 | `scope_ai`, `AiProvider`, `noop` provider, `ai.*` config | ⬜ Planned |
| M13.2 | NL → filter DSL translation + validation | ⬜ Planned |
| M13.3 | Investigation summaries | ⬜ Planned |
| M13.4 | Anomaly hints (analytics context) | ⬜ Planned |
| M13.5 | `logscope agent investigate` CLI | ⬜ Planned |
| M13.6 | `HttpAiProvider` (OpenAI-compatible / Ollama) | ⬜ Planned |
| M13.7 | Doc sync, `v1.5.1` release | ⬜ Planned |

---

# 4. Deliverables

## Core (`core/ai/`)

- `AiProvider` interface and `AiProviderRegistry`
- `NoOpAiProvider` — deterministic offline behaviour for CI
- `HttpAiProvider` — OpenAI-compatible chat completions (M13.6)
- `AiInvestigationAssistant` — orchestrator
- `AiConfig` — parse `ai.*` configuration keys
- `nl_query_translator` — NL → DSL with `parseFilterQuery` validation

## CLI

- `logscope agent investigate` — primary M13 entry point
- Flags: `--ask`, `--summarize`, `--hints`
- Reuses existing Source → Analysis → Investigation → Analytics pipeline

## Config

```properties
ai.enabled=false
ai.provider=noop
ai.endpoint=
ai.model=
ai.max_context_lines=200
```

Environment: `LOGSCOPE_AI_API_KEY` (HTTP provider only).

## Tests

- Unit: provider selection, noop outputs, NL validation, config parsing
- Integration: assistant with mock provider; HTTP mock server (M13.6)
- E2E: `agent investigate` with sample logs, `ai.provider=noop`
- CI: **no network**; all matrix jobs use `noop`

---

# 5. Non-goals

- `logscope agent design|implement|test|release|docs|performance` (PRD-001 later agents)
- CrashScope, core dumps, traces, multi-source workspaces
- Autonomous actions (no code changes, no unsupervised remediation)
- GUI (M14), Web (M15)
- AI model training, fine-tuning, or marketplace
- Plugin-only AI (core ships built-in providers; plugin hook is post-M13 stretch)
- Changing default behaviour of `investigate`, `query`, `search`, `analyze`

---

# 6. FR-004 mapping

| FR-004 criterion | M13 implementation |
|------------------|-------------------|
| Configurable behaviour | `ai.enabled`, `ai.provider`, `ai.endpoint`, `ai.model` |
| Extension support | Provider registry; future plugin AI hook |
| Existing functionality preserved | `ai.enabled=false` default; existing commands unchanged |
| Extension failures isolated | AI errors do not abort core investigate (A8 scenarios) |
| Identify extensions | `ai.provider` in config; agent output labels provider |

---

# 7. Engineering conventions

| Convention | Value |
|------------|-------|
| Module | `core/ai` (`scope_ai`) |
| HTTP (M13.6) | cpp-httplib via FetchContent |
| Tests | `scope_ai_tests` + CLI integration + e2e |
| Branch | `feat/v1.5.1-m13-ai-assistant` (implementation phases) |

---

# 8. Related documents

| Document | Purpose |
|----------|---------|
| [ADR-007](../architecture/decisions/ADR-007-AI-Integration.md) | Architecture decision |
| [M13-V151-AI-SCENARIOS.md](M13-V151-AI-SCENARIOS.md) | Acceptance gate |
| [PRD-001](../requirements/future/PRD-001-AI-Engineering-Agents.md) | Long-term agent vision |
| [CLI_REFERENCE.md](../handbook/CLI_REFERENCE.md) | User-facing command docs (M13.7) |
| [CONFIGURATION_GUIDE.md](../handbook/CONFIGURATION_GUIDE.md) | `ai.*` keys (M13.1) |
