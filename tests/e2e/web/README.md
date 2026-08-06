# LogScope Web E2E (Playwright)

Browser end-to-end tests for the LogScope web SPA (Story Gate demo paths).

**UI principles:** Tests assume the IDE three-pane layout documented in [`docs/handbook/WEB_UI_DESIGN.md`](../../../docs/handbook/WEB_UI_DESIGN.md). Use `data-testid` selectors in specs.

## Prerequisites

1. Build `logscope-web`:

   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLOGSCOPE_WEB=ON
   cmake --build build --target logscope-web
   ```

2. Install Node dependencies:

   ```bash
   cd tests/e2e/web
   npm ci
   npx playwright install chromium
   ```

## Run locally

From `tests/e2e/web`:

```bash
npm test
```

The global setup starts `logscope-web` with:

- `--config samples/demo-story-gate.properties`
- `LOGSCOPE_WEB_UI_DIR=apps/web/ui/dist` (repo root as cwd)

Override paths if needed:

```bash
export LOGSCOPE_WEB_BIN=/path/to/logscope-web
export LOGSCOPE_WEB_BASE_URL=http://127.0.0.1:8080
npm test
```

To attach to an already-running server:

```bash
export LOGSCOPE_WEB_E2E_EXTERNAL=1
npm test
```

## Coverage

| Scenario | Spec |
|----------|------|
| Story 1–2: investigation + log artifact + center viewer | `story-gate.spec.ts` |
| Story 3: timeline events + jump | `story-gate.spec.ts` |
| Story 4: crash SIGSEGV + fault thread pstack jump | `story-gate.spec.ts` |
| Investigate after analyze (default `error` search) | `story-gate.spec.ts` |
| AI ask `errors` (noop config) | `story-gate.spec.ts` |
| **README web screenshot** (Related Evidence + badges) | `capture-readme-screenshots.spec.ts` |

### Refresh README screenshots

From `tests/e2e/web` after building `logscope-web`:

```bash
npx playwright test capture-readme-screenshots.spec.ts --project=chromium
```

Writes `docs/assets/logscope-web.png`. CLI and desktop PNGs: [docs/assets/README.md](../../../docs/assets/README.md). Ship all three with every README release update.

## Desktop parity gaps

Web bottom dock tabs **Timeline** and **Crash** are not yet on desktop. Desktop uses bottom tabs **Results**, **AI**, and **Analytics**.

**Agent rule:** For new user-visible features, implement on **both** web and desktop in the same milestone when feasible. If one surface lags, document the gap here and in the scenario matrix — do not let UIs drift silently. See [`docs/handbook/WEB_UI_DESIGN.md`](../../../docs/handbook/WEB_UI_DESIGN.md) § Web ↔ desktop sync.

## CI

The `web` job in `.github/workflows/ci.yml` runs these tests on Ubuntu after building `logscope-web`.
