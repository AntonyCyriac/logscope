# Tutorial 3 — Plugins and AI

Optional dynamic plugins and AI-assisted investigation.

## Plugins

List built-in extensions:

```bash
logscope extensions list
logscope extensions describe analysis.log-levels
```

Enable dynamic plugins via configuration (`plugins.enabled=true`, `plugins.paths`). See [Plugin Development Guide](../handbook/PLUGIN_DEVELOPMENT_GUIDE.md).

## AI-assisted investigation (offline)

Default config keeps AI off. Use the noop sample for deterministic offline behaviour:

```bash
logscope agent investigate --config samples/ai-noop.properties --summarize samples/sample.log
logscope agent investigate --config samples/ai-noop.properties \
  --ask "errors" --hints --summarize samples/sample.log
```

## Local Ollama

Requires [Ollama](https://ollama.com) running and a pulled model.

```bash
export LOGSCOPE_AI_API_KEY=ollama
logscope agent investigate --config samples/ai-ollama.properties --summarize samples/sample.log
logscope agent investigate --config samples/ai-ollama.properties --hints samples/sample.log
```

Windows PowerShell:

```powershell
$env:LOGSCOPE_AI_API_KEY = "ollama"
logscope agent investigate --config samples/ai-ollama.properties --summarize samples/sample.log
```

Prefer `--filter "level == ERROR"` over `--ask` with smaller local models.

## OpenAI-compatible cloud API

Copy `samples/ai-openai.properties.example`, set your API key in the environment (never in the file):

```bash
cp samples/ai-openai.properties.example my-openai.properties
export LOGSCOPE_AI_API_KEY="sk-..."
logscope agent investigate --config my-openai.properties --summarize samples/sample.log
```

## Pipeline statistics

After `v1.5.2`, use `--stats` to print parse timing and resource usage:

```bash
logscope analyze --stats samples/sample.log
logscope agent investigate --config samples/ai-noop.properties --stats --summarize samples/sample.log
```

## References

- [samples/README.md](../../samples/README.md)
- [Configuration Guide §9](../handbook/CONFIGURATION_GUIDE.md#9-ai-configuration-v151)
- [User Manual §5.10](../handbook/USER_MANUAL.md#510-ai-assisted-investigation-v151)
