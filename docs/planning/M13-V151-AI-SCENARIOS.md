# M13 v1.5.1 — AI Assistant Scenarios

| Field | Value |
|-------|-------|
| Document | M13 v1.5.1 AI Scenarios |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | In progress |
| Created | 25-07-2026 |
| Last Updated | 25-07-2026 |

---

# 1. Purpose

Acceptance scenarios for **v1.5.1** — M13 AI Assistant. Every row must pass before release.

**Target release:** `v1.5.1`

**Features in scope:**

| ID | Feature |
|----|---------|
| A1 | `AiProvider` abstraction + `noop` provider |
| A2 | `ai.*` configuration keys + validation |
| A3 | Natural-language → filter DSL translation |
| A4 | Investigation summaries |
| A5 | Anomaly hints |
| A6 | `logscope agent investigate` CLI |
| A7 | HTTP / OpenAI-compatible provider |
| A8 | Failure isolation (AI errors do not break core investigate) |

See [M13-AI-ASSISTANT.md](M13-AI-ASSISTANT.md), [ADR-007](../architecture/decisions/ADR-007-AI-Integration.md).

---

# 2. Legend

| Column | Meaning |
|--------|---------|
| **Regression** | Dedicated regression or e2e guard |
| **Test layer** | U = unit, I = integration, E = e2e |

Status: ⬜ planned · 🟡 in progress · ✅ complete

---

# 3. Provider abstraction (A1)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| A1.1 | Default provider | `ai.enabled=false` (default) | `noop` provider selected | U | Yes | ✅ |
| A1.2 | Explicit noop | `ai.provider=noop` | Noop methods callable | U | | ✅ |
| A1.3 | Unknown provider | `ai.provider=invalid` | Config validation error | U | | ✅ |
| A1.4 | Factory registry | Register `noop` and `http` ids | `createAiProvider` resolves by id | U | | ✅ |

---

# 4. Configuration (A2)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| A2.1 | Defaults | Empty / missing `ai.*` | `ai.enabled=false`, `ai.provider=noop` | U | Yes | ✅ |
| A2.2 | Enable HTTP | `ai.enabled=true`, `ai.provider=http`, endpoint+model set | Http provider selected | U | | ✅ |
| A2.3 | Missing API key | `ai.provider=http`, no `LOGSCOPE_AI_API_KEY` | Clear error; no silent fallback | U+I | Yes | ✅ |
| A2.4 | Config validate | `logscope config validate` | Validates `ai.*` keys | U+E | | ⬜ |
| A2.5 | Context limit | `ai.max_context_lines=50` | At most 50 lines in provider context | U | | ⬜ |

---

# 5. NL → DSL (A3)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| A3.1 | Noop heuristic | `--ask "errors"`, noop provider | Valid DSL (e.g. `level == ERROR`); investigate runs | U+I | Yes | ✅ |
| A3.2 | Invalid DSL rejected | Provider returns malformed DSL | Error; investigate not run with bad filter | U | Yes | ✅ |
| A3.3 | HTTP translation | Mock server returns `level == ERROR` | Parsed and applied to investigate | I | | ⬜ |
| A3.4 | Existing DSL unchanged | `investigate --filter 'level == ERROR'` | No AI path; same as pre-M13 | I+E | Yes | ⬜ |

---

# 6. Summaries (A4)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| A4.1 | Noop summary | `--summarize`, noop provider | Structured output with summary + evidence sections | U+I | Yes | ✅ |
| A4.2 | Empty investigation | No matching lines | Summary states no matches | U | | ✅ |
| A4.3 | HTTP summary | Mock server returns summary JSON/text | Rendered in agent output | I | | ✅ |
| A4.4 | Evidence cites lines | Summary with matches | Evidence references line numbers | U | | ✅ |

---

# 7. Anomaly hints (A5)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| A5.1 | Spike hint | Log with rate spike, `--hints`, noop | Hint references trend/spike | U+I | Yes | ✅ |
| A5.2 | Cluster hint | Repeated errors, `--hints` | Hint references top cluster | U | | ✅ |
| A5.3 | No analytics signal | Uniform log, `--hints` | Empty or "no anomalies" message | U | | ✅ |
| A5.4 | HTTP hints | Mock server | Hints rendered in output | I | | ✅ |

---

# 8. CLI (A6)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| A6.1 | Agent help | `logscope agent --help` | Shows `investigate` subcommand | E | | ✅ |
| A6.2 | Investigate help | `logscope agent investigate --help` | Documents `--ask`, `--summarize`, `--hints` | E | | ✅ |
| A6.3 | End-to-end | `agent investigate sample.log --summarize` | Exit 0; summary section present | E | Yes | ✅ |
| A6.4 | Combined flags | `--ask "errors" --hints --summarize` | NL investigate + hints + summary | E | Yes | ✅ |
| A6.5 | Existing commands | `logscope investigate` unchanged | No `agent` prefix required; same behaviour | E | Yes | ✅ |

---

# 9. HTTP provider (A7)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| A7.1 | OpenAI-compatible | Mock `/v1/chat/completions` | Request/response parsed | I | | ✅ |
| A7.2 | Local endpoint | `ai.endpoint=http://127.0.0.1:<port>/v1` | Works with mock (Ollama-shaped) | I | | ✅ |
| A7.3 | CI offline | Default CI config | No HTTP calls in matrix | CI | Yes | ✅ |
| A7.4 | Timeout | Mock slow response | Error surfaced; no hang | I | | ✅ |

---

# 10. Isolation (A8)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| A8.1 | HTTP down | Provider unreachable | Core investigate output still shown | I | Yes | ⬜ |
| A8.2 | Summary failure | Mock summarize error | Investigation lines still printed | I | Yes | ⬜ |
| A8.3 | AI disabled | `ai.enabled=false` | `agent investigate` runs pipeline; skips or uses noop AI | E | Yes | ✅ |
| A8.4 | Invalid --ask | Bad NL translation | Non-zero exit; no partial bad filter applied | E | | ✅ |

---

# 11. Release gate

All A1–A8 rows marked ✅ before tagging `v1.5.1`.
