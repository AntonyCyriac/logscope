# ADR-007: AI Integration Architecture

- **Status:** Accepted
- **Date:** 25-07-2026

---

## Context

M10 delivered a field-aware filter DSL ([ADR-004](ADR-004-Query-DSL-Grammar.md)); M9 delivered analytics (frequency, clustering, trends, correlations); M12 delivered dynamic plugins ([ADR-006](ADR-006-Plugin-Loading.md)). **M13 (`v1.5.1`)** adds the first **product-runtime** AI capabilities: natural-language queries, investigation summaries, and anomaly hints.

PRD-001 describes a long-term multi-agent platform. M13 is a **bounded** slice: assistive AI for log investigation only, invoked via `logscope agent investigate`. Operational IDE agents (Cursor, private `.ai/`) are out of scope.

Requirements:

- Core investigation must work **offline** with `ai.enabled=false` (default).
- AI failures must not break existing commands (FR-004.5 isolation pattern).
- NL queries must compile to **validated** filter DSL, not bypass `QueryEvaluator`.
- API keys and outbound network access are **opt-in**.

---

## Decision

### 1. New module `scope_ai`

Location: `core/ai/` — CMake target `scope_ai`.

Depends on: `scope_foundation`, `scope_query`, `scope_investigation`, `scope_analytics` (read-only context types).

Does **not** depend on: `scope_plugin` (plugin AI provider hook is a future stretch, not M13).

### 2. Provider abstraction

```cpp
class AiProvider {
public:
    virtual ~AiProvider() = default;
    virtual Result<std::string> translateNlToFilter(std::string_view nlQuery) const = 0;
    virtual Result<AiSummary> summarize(const AiInvestigationContext& context) const = 0;
    virtual Result<std::vector<AiAnomalyHint>> suggestAnomalies(const AiAnalyticsContext& context) const = 0;
};
```

| Provider ID | Class | When |
|-------------|-------|------|
| `noop` | `NoOpAiProvider` | Default; CI; `ai.enabled=false` |
| `http` | `HttpAiProvider` | `ai.enabled=true`, `ai.provider=http` |

Factory: `createAiProvider(const AiConfig&)` in `ai_provider_registry.cpp`.

### 3. Orchestrator

`AiInvestigationAssistant` coordinates:

1. Optional NL → DSL translation (`translateNlToFilter` → `query::parseFilterQuery` validation)
2. Standard `InvestigationEngine` / `AnalyticsEngine` execution (unchanged)
3. Optional `summarize` / `suggestAnomalies` on bounded structured context

Context bounding: `ai.max_context_lines` (default `200`) caps sample lines sent to HTTP provider.

### 4. Configuration

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `ai.enabled` | boolean | `false` | Master switch; when false, `noop` provider is used |
| `ai.provider` | string | `noop` | `noop` \| `http` |
| `ai.endpoint` | string | `""` | Base URL for OpenAI-compatible API (e.g. `https://api.openai.com/v1` or `http://localhost:11434/v1`) |
| `ai.model` | string | `""` | Model name (provider-specific) |
| `ai.max_context_lines` | integer | `200` | Max log lines included in LLM context |

| Environment | Purpose |
|-------------|---------|
| `LOGSCOPE_AI_API_KEY` | Bearer token for HTTP provider; never logged or stored in config files |

Validation in `config validate`: reject unknown `ai.provider`; warn when `ai.enabled=true` and `ai.provider=http` without endpoint/model.

### 5. CLI surface (M13)

Primary entry:

```bash
logscope agent investigate <log-source> [options]
```

| Flag | Purpose |
|------|---------|
| `--ask "<nl>"` | Natural-language query → filter DSL → investigate |
| `--summarize` | Emit AI investigation summary |
| `--hints` | Emit anomaly hints from analytics context |

Existing `investigate`, `query`, `search`, `analyze` commands are **unchanged** in M13.

Parser/dispatcher: `apps/cli/agent_command.cpp`; extends `CliApplication` and `cli_parser`.

### 6. NL → DSL translation

Flow:

```text
--ask "show errors from yesterday"
    → AiProvider::translateNlToFilter()
    → query::parseFilterQuery()  (reject invalid DSL)
    → InvestigationCriteria::filterExpression
    → InvestigationEngine::investigate()
```

`NoOpAiProvider` uses deterministic keyword heuristics for CI (e.g. `errors` → `level == ERROR`) — not a substitute for LLM quality, but sufficient for offline tests.

HTTP provider sends a structured prompt requesting **filter DSL only**; host validates before execution.

### 7. Summaries and anomaly hints

**Summaries** — input: `InvestigationResult`, `InvestigationView`, optional `AnalyticsResult` (bounded). Output struct `AiSummary`:

```text
summary, reasoning, evidence[], confidence, suggestedActions[]
```

`NoOpAiProvider` composes text from existing `InvestigationView::summary()` and executive report heuristics.

**Anomaly hints** — input: `TrendResult`, error clusters, `CorrelationSummary`. `NoOpAiProvider` surfaces rule-based signals (`TrendResult::hasSpike()`, top clusters). HTTP provider narrates pre-computed analytics; no raw ML in M13.

Optional report hook: `AiSummaryReportContributor` registers via `ReportSectionRegistry` when `ai.enabled=true` and report is generated from `agent investigate`.

### 8. HTTP client

M13 adds a minimal HTTP client for OpenAI-compatible **chat completions** JSON API.

| Approach | Decision |
|----------|----------|
| Dependency | **cpp-httplib** via CMake `FetchContent` (header-only, MIT, used only when `ai.provider=http`) |
| TLS | Platform default; document that production endpoints require HTTPS |
| Timeouts | Connect 10s, read 60s (configurable in code constants, not user-facing in M13) |
| Local models | Ollama-compatible: `ai.endpoint=http://localhost:11434/v1`, `ai.model=<name>` |

CI and default config: **no network**; all automated tests use `noop`.

### 9. Failure isolation (FR-004.5)

| Failure | Behaviour |
|---------|-----------|
| Provider unreachable | Error message to stderr; investigation results still printed if pipeline succeeded |
| Invalid NL translation | Error; no investigate with unvalidated DSL |
| `summarize` / `hints` failure | Skip AI section; core output unchanged |
| `ai.enabled=false` | No HTTP calls; noop or skip AI sections silently |

### 10. Security and privacy

- Treat log excerpts sent to HTTP provider as **sensitive**; document in [SECURITY_REVIEW.md](../../handbook/SECURITY_REVIEW.md).
- No API keys in config files, CLI flags, or logs.
- No automatic retry with key rotation.
- Users must explicitly enable `ai.enabled=true` and set endpoint/model.

### 11. Output format

Agent output follows PRD-001 structure (text sections, not JSON in M13):

```text
Agent: Investigation
Purpose: ...
Summary: ...
Reasoning: ...
Evidence:
  - line 42: ...
Confidence: medium
Suggested Actions:
  - ...
```

---

## Consequences

**Positive**

- Offline-first default; zero behaviour change for existing users.
- NL queries reuse M10 `QueryEvaluator` — no parallel query path.
- Pluggable providers enable local (Ollama) and cloud endpoints without core changes.
- Clear extension point for future plugin AI providers (post-M13).

**Negative**

- HTTP dependency adds FetchContent build time when `scope_ai` links httplib.
- LLM translation quality varies; host validation catches syntax errors only, not semantic mistakes.
- Context bounding may omit relevant lines on very large investigations.

**Neutral**

- Full PRD-001 agent roster (`agent design`, `implement`, etc.) remains future work.

---

## Related

- [PRD-001](../../requirements/future/PRD-001-AI-Engineering-Agents.md)
- [ADR-004](ADR-004-Query-DSL-Grammar.md) — DSL grammar and M13 NL extension point
- [ADR-006](ADR-006-Plugin-Loading.md) — future AI analyzer plugin hook
- [M13-AI-ASSISTANT.md](../../planning/M13-AI-ASSISTANT.md)
- [M13-V151-AI-SCENARIOS.md](../../planning/M13-V151-AI-SCENARIOS.md)
- [POST_V1_STRATEGIC_ROADMAP.md](../../planning/POST_V1_STRATEGIC_ROADMAP.md) — Phase 5
