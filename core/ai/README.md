# AI Assistant

`scope_ai` implements M13 pluggable AI providers for investigation assistance.

## Shipped (M13.1–M13.6)

| Component | Purpose |
|-----------|---------|
| `AiProvider` | Virtual interface: NL→DSL, summarize, anomaly hints |
| `NoOpAiProvider` | Deterministic offline provider (default; CI) |
| `HttpAiProvider` | OpenAI-compatible chat completions via cpp-httplib |
| `HttpAiClient` | `/v1/chat/completions` client with bounded timeouts |
| `ai_json_util` | Minimal JSON helpers for HTTP request/response parsing |
| `NlQueryTranslator` | NL → DSL with `parseFilterQuery` validation |
| `AiInvestigationAssistant` | Config + provider holder; NL translation, summaries, and anomaly hints |
| `ai_context_builder` | Bounded evidence samples from investigation output |
| `ai_analytics_context_builder` | Bounded signals from analytics trends/clusters/correlations |
| `ai_summary_formatter` | Renders `AiSummary` sections for CLI/agent output |
| `ai_anomaly_hint_formatter` | Renders `AiAnomalyHint` list for CLI/agent output |
| `logscope agent investigate` | Primary M13 CLI entry (`--ask`, `--summarize`, `--hints`) |
| `ai.*` config keys | See [CONFIGURATION_GUIDE](../../docs/handbook/CONFIGURATION_GUIDE.md) |

Architecture: [ADR-007](../../docs/architecture/decisions/ADR-007-AI-Integration.md) · [M13 planning](../../docs/planning/M13-AI-ASSISTANT.md)
