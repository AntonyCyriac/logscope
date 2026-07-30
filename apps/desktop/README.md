# LogScope Desktop (M14)

Qt6 Widgets investigation workbench — `logscope-desktop`.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLOGSCOPE_DESKTOP=ON
cmake --build build --target logscope-desktop
```

Requires Qt6 Widgets (`qt6-base-dev` on Ubuntu).

## Features

- Open log file, analyze, investigate (search/query/DSL filters)
- Analytics tabs, report export (HTML/PDF/JSON/…)
- Session save/load/list, extensions list
- AI assistant panel (ask, summarize, hints)
- Live tail toggle, dark/light themes

See [M14-DESKTOP-APPLICATION.md](../../docs/planning/M14-DESKTOP-APPLICATION.md).
