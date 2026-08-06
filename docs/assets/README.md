# README surface screenshots

Homepage images for [README.md](../../README.md). **Refresh all three together** on every release that changes CLI, desktop, or web user-visible behavior — same PR as the README text update. See [Release process](../release/RELEASE.md#2-version-bump-and-readme-snapshots).

| File | Surface | What to show |
|------|---------|----------------|
| `logscope-cli.png` | **CLI** | Terminal with `logscope` commands relevant to the current story line (analyze, investigate, `investigation timeline`, `investigation links`, `investigation crash` as shipped). |
| `logscope-desktop.png` | **Desktop** | `logscope-desktop` with `samples/sample.log` analyzed; bottom tabs **Results · AI · Analytics** visible. |
| `logscope-web.png` | **Web** | `logscope-web` IDE layout: artifacts, center viewer, bottom dock (**Timeline · Crash · AI · Results**), plus current story UI (e.g. Related Evidence panel and timeline connection badges for Story 5+). |

Keep PNGs reasonably sized (README loads in GitHub). Prefer viewport crops over full-page scroll captures.

## Web (automated)

From repo root, with Node 20+:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLOGSCOPE_WEB=ON
cmake --build build --target logscope-web
cd tests/e2e/web && npm ci && npx playwright install chromium
npx playwright test capture-readme-screenshots.spec.ts --project=chromium
```

Writes `docs/assets/logscope-web.png` via Playwright (Story Gate–style investigation with evidence link badge).

Use `LOGSCOPE_WEB_E2E_EXTERNAL=1` if `logscope-web` is already running on port 8080.

## CLI (manual)

1. Build: `cmake --build build --target logscope`
2. Run a short, readable command sequence against `samples/sample.log` and investigation subcommands shipped in this release.
3. Capture the terminal window (no secrets, no huge scrollback).
4. Save as `docs/assets/logscope-cli.png`.

## Desktop (automated)

From repo root (Linux CI / headless):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLOGSCOPE_DESKTOP=ON
cmake --build build --target logscope_capture_readme_desktop
QT_QPA_PLATFORM=offscreen ./build/apps/desktop/tests/logscope_capture_readme_desktop
```

Writes `docs/assets/logscope-desktop.png` with `samples/sample.log` analyzed and bottom tabs **Results · AI · Analytics** visible.

For a manual capture on a visible GUI, build `logscope-desktop`, open `samples/sample.log`, run **Analyze**, and save the window including the bottom tab bar.

## CI note

PRs that touch **only** `*.md` and `docs/assets/**` qualify for **Docs only (CI waived)** — fast merge path for README + snapshot updates.
