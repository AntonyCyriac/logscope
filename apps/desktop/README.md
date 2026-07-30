# LogScope Desktop (M14)

Qt6 Widgets investigation workbench — `logscope-desktop`.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLOGSCOPE_DESKTOP=ON
cmake --build build --target logscope-desktop
```

Requires Qt6 Widgets (`qt6-base-dev` on Ubuntu, `qt@6` via Homebrew on macOS).

On macOS the build produces `logscope-desktop.app`; run the bundle or the executable inside it:

```bash
open build/apps/desktop/logscope-desktop.app
# or
build/apps/desktop/logscope-desktop.app/Contents/MacOS/logscope-desktop --config samples/ai-ollama.properties
```

## Run

```bash
./build/apps/desktop/logscope-desktop
./build/apps/desktop/logscope-desktop --config samples/logscope.properties
```

Use **File → Load Configuration…** to load a properties file after startup.

## Configuration

Desktop uses the same `.properties` files as the CLI. On startup it loads defaults unless you pass `--config` or use **File → Load Configuration…**.

| Goal | Config file | Extra setup |
|------|-------------|-------------|
| General logging / plugins | `samples/logscope.properties` | — |
| AI offline (CI-safe) | `samples/ai-noop.properties` | — |
| AI with local Ollama | `samples/ai-ollama.properties` | Ollama running; `LOGSCOPE_AI_API_KEY=ollama` |
| AI with OpenAI-compatible API | `samples/ai-openai.properties.example` | `LOGSCOPE_AI_API_KEY` set in environment |

### Ollama (local AI)

1. Install and start [Ollama](https://ollama.com); pull a model (default config uses `llama3.2`).
2. Set the API key env var (any non-empty value):

```bash
export LOGSCOPE_AI_API_KEY=ollama
logscope-desktop --config samples/ai-ollama.properties
```

Windows PowerShell:

```powershell
$env:LOGSCOPE_AI_API_KEY = "ollama"
.\build\apps\desktop\logscope-desktop.exe --config samples\ai-ollama.properties
```

3. Open a log, **Analyze**, then use the AI panel (**Ask**, **Summarize**, **Hints**).

Without a config file, AI is disabled (`ai.enabled=false` by default).

## Features

- Open log file, analyze, investigate (search/query/DSL filters)
- Analytics tabs, report export (HTML/PDF/JSON/…)
- Session save/load/list, extensions list
- AI assistant panel (ask, summarize, hints)
- Live tail toggle, dark/light themes

See [M14-DESKTOP-APPLICATION.md](../../docs/planning/M14-DESKTOP-APPLICATION.md) and [samples/README.md](../../samples/README.md).
