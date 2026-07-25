# AI Assistant

`scope_ai` implements M13 pluggable AI providers for investigation assistance.

## Shipped (M13.1)

| Component | Purpose |
|-----------|---------|
| `AiProvider` | Virtual interface: NL→DSL, summarize, anomaly hints |
| `NoOpAiProvider` | Deterministic offline provider (default; CI) |
| `HttpAiProvider` | Stub for M13.6 HTTP/OpenAI-compatible backend |
| `AiInvestigationAssistant` | Config + provider holder |
| `ai.*` config keys | See [CONFIGURATION_GUIDE](../../docs/handbook/CONFIGURATION_GUIDE.md) |

Architecture: [ADR-007](../../docs/architecture/decisions/ADR-007-AI-Integration.md) · [M13 planning](../../docs/planning/M13-AI-ASSISTANT.md)
