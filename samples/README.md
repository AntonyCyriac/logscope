# LogScope samples

Example logs and configuration files for trying LogScope without writing your own fixtures.

| File | Purpose |
|------|---------|
| `sample.log` | Plain-text log with errors, warnings, and info lines |
| `large-app.log` | ~1 MB plain-text log (~14k lines) for persist-index, tail, and desktop stress |
| `sample.jsonl` | JSON Lines sample |
| `logscope.properties` | General configuration example |
| `ai-noop.properties` | Offline AI provider (CI, no network) |
| `ai-ollama.properties` | Local Ollama HTTP provider |
| `ai-openai.properties.example` | OpenAI-compatible cloud API template (copy and customize) |

---

## Basic analysis

```bash
logscope analyze samples/sample.log
logscope investigate --filter "level == ERROR" samples/sample.log
```

### Large log (~1 MB)

Use `large-app.log` to exercise persistent indexes, reuse, and desktop performance:

```bash
logscope analyze --stats --persist-index samples/large-app.log
logscope analyze --reuse-index --filter "level == ERROR" samples/large-app.log
```

Desktop: **Open…** → `samples/large-app.log`, enable **Persist index**, then **Analyze**.

---

## AI-assisted investigation (`agent investigate`)

Default config has `ai.enabled=false`. Use a sample properties file or copy `ai-openai.properties.example`.

### Offline — `ai-noop.properties`

Deterministic keyword heuristics; no HTTP calls. Best for CI and quick local checks.

```bash
logscope agent investigate --config samples/ai-noop.properties --summarize samples/sample.log
logscope agent investigate --config samples/ai-noop.properties \
  --ask "errors" --hints --summarize samples/sample.log
```

### Local Ollama — `ai-ollama.properties`

Requires [Ollama](https://ollama.com) running and a pulled model (default config uses `llama3.2`).

**Linux / macOS:**

```bash
export LOGSCOPE_AI_API_KEY=ollama   # any non-empty value
logscope agent investigate --config samples/ai-ollama.properties --summarize samples/sample.log
logscope agent investigate --config samples/ai-ollama.properties --hints samples/sample.log
```

**Windows (PowerShell):**

```powershell
$env:LOGSCOPE_AI_API_KEY = "ollama"
logscope agent investigate --config samples/ai-ollama.properties --summarize samples/sample.log
```

**Tip:** `--summarize` and `--hints` work well with Ollama. For filters, prefer explicit DSL (`--filter "level == ERROR"`) over `--ask` — smaller models may return invalid DSL.

```bash
logscope agent investigate --config samples/ai-ollama.properties \
  --filter "level == ERROR" --summarize --hints samples/sample.log
```

### Cloud OpenAI — `ai-openai.properties.example`

1. Copy the example file: `cp samples/ai-openai.properties.example my-openai.properties`
2. Set your API key in the environment (never in the properties file):

```bash
export LOGSCOPE_AI_API_KEY="sk-..."
logscope agent investigate --config my-openai.properties --summarize samples/sample.log
```

```powershell
$env:LOGSCOPE_AI_API_KEY = "sk-..."
logscope agent investigate --config my-openai.properties --summarize samples/sample.log
```

---

## Related docs

- [Configuration Guide §9](../docs/handbook/CONFIGURATION_GUIDE.md#9-ai-configuration-v151) — `ai.*` keys
- [User Manual §5.10](../docs/handbook/USER_MANUAL.md#510-ai-assisted-investigation-v151) — AI workflow
- [CLI Reference — agent investigate](../docs/handbook/CLI_REFERENCE.md#agent-investigate)
