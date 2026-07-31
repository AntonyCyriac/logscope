# ADR-009: Web Platform REST API

- **Status:** Accepted
- **Date:** 30-07-2026

---

## Context

M0–M14 delivered CLI ([COMPONENT_CATALOG.md](../COMPONENT_CATALOG.md) C09) and Qt desktop (C11) presentation layers over shared **ApplicationService** orchestration ([ADR-008](ADR-008-Desktop-Qt-Presentation.md)). **M15 (`v2.1.0`)** adds a browser-accessible investigation workbench and a **REST API** so remote clients can run the same open → analyze → investigate → export workflows without duplicating domain logic in the web tier.

Requirements:

- Reuse `ApplicationService` (`apps/common/application_service.hpp`) — same methods CLI and desktop call today.
- Offline-first: default deployment is a **local single process** (`logscope-web`); no cloud dependency.
- Single-binary preference where practical (embedded HTTP server + static UI assets).
- Cross-platform: Windows, Linux, macOS (match existing CI matrix).
- CLI remains the primary automation surface; REST must not fork behaviour.
- Parity: investigate counts, analytics buckets, and export sections must match CLI `--format json` on the same fixtures (`samples/sample.log`, `samples/large-app.log`).
- Security baseline for v1: safe defaults on a trusted LAN; enterprise RBAC deferred.

**Planning gate:** [M15-WEB-PLATFORM.md](../../planning/M15-WEB-PLATFORM.md) M15.0 requires this ADR before implementation.

---

## Decision

### 1. Presentation components

| ID | Component | Location | Role |
|----|-----------|----------|------|
| C09 | CLI | `apps/cli/` | Command-line interface (existing) |
| C10 | Application orchestration | `apps/common/` — `scope_application` | Shared pipeline for all presentations |
| C11 | Desktop (Qt Widgets) | `apps/desktop/` — `logscope-desktop` | Native GUI (M14) |
| **C12** | **Web Platform** | `apps/web/` — `logscope-web` | HTTP server, REST handlers, static SPA shell |

C12 owns HTTP transport, request validation, JSON serialization, session routing, and static asset serving. It does **not** own investigation, analytics, reporting, or AI logic.

CMake option: `LOGSCOPE_WEB=ON` (default OFF for minimal CI builds), analogous to `LOGSCOPE_DESKTOP`.

### 2. HTTP server: embedded **cpp-httplib**

Use **[cpp-httplib](https://github.com/yhirose/cpp-httplib)** as the embedded HTTP server inside `logscope-web`.

| Option | Verdict | Rationale |
|--------|---------|-----------|
| **cpp-httplib** | **Selected** | Already vendored via `FetchContent` for `scope_ai` HTTP client; header-only; MIT license; thread-pool server; static file + multipart upload support; no extra runtime on Win/Linux/macOS |
| Boost.Beast | Rejected for M15 | Heavy Boost dependency; CMake/CI cost exceeds benefit for MVP |
| Separate reverse-proxy process (nginx + CGI) | Rejected for M15 | Violates single-binary / offline-first local UX; acceptable as **deployment** option later, not core architecture |
| Node/Electron sidecar | Rejected | Second runtime; outside C++ product boundary |

**Process model:** one `logscope-web` executable binds `web.bind_host` / `web.bind_port` (defaults `127.0.0.1:8080`). Listens on loopback by default to reduce accidental exposure.

Shared httplib dependency: promote to a common CMake target (e.g. `logscope_httplib`) consumed by `scope_ai` and `apps/web` to avoid duplicate FetchContent pins.

### 3. REST API surface (`/api/v1`)

All routes are JSON unless noted. Handlers delegate to `ApplicationService` on the active **workspace session** (see §7).

| Method | Path | ApplicationService | Purpose |
|--------|------|-------------------|---------|
| `GET` | `/api/v1/health` | — | Liveness: version, uptime, session count |
| `POST` | `/api/v1/config/load` | `loadConfiguration` | Load properties file (server-side path) |
| `POST` | `/api/v1/config/validate` | `validateConfiguration` | Validate loaded config |
| `POST` | `/api/v1/sources/open` | `openSource` | Open log by **server filesystem path** (gated; §8) |
| `POST` | `/api/v1/sources/upload` | `openSource` | Multipart file upload → temp file → open |
| `POST` | `/api/v1/analyze` | `analyze` | Run analysis; returns `AnalysisModel` summary |
| `POST` | `/api/v1/investigate` | `investigate` | Filter/search; returns `InvestigationResult` |
| `POST` | `/api/v1/analytics` | `runAnalytics` | Analytics buckets/trends |
| `POST` | `/api/v1/export` | `generateReport` | Returns report body or attachment per `format` |
| `GET` | `/api/v1/sessions` | `listSessions` | List saved session files in directory |
| `POST` | `/api/v1/sessions/save` | `saveSession` | Persist workspace session |
| `POST` | `/api/v1/sessions/load` | `loadSession` + `adoptModel` | Restore session into active workspace |
| `GET` | `/api/v1/extensions` | `listExtensions` | Registered extensions |
| `GET` | `/api/v1/extensions/{id}` | `describeExtension` | Extension metadata |
| `POST` | `/api/v1/agent/investigate` | `agentInvestigate` | NL ask / summarize / hints |
| `POST` | `/api/v1/tail/start` | `startTail` | Begin live tail (optional M15.3) |
| `POST` | `/api/v1/tail/stop` | `stopTail` | Stop tail |
| `GET` | `/api/v1/tail/poll` | `pollTailLines` | Fetch appended lines since last poll |

**Versioning:** `/api/v1` prefix; breaking changes require `/api/v2` (no silent breakage).

**OpenAPI:** generate `docs/api/openapi-v1.yaml` in M15.1 from handler contracts (documentation artifact; not runtime dependency).

### 4. Request/response shape and error model

#### JSON conventions

- **Content-Type:** `application/json; charset=utf-8` for API bodies; `multipart/form-data` for upload only.
- **Field naming:** `camelCase` in HTTP JSON (CLI stdout JSON may use existing keys; mappers in C12 normalize to a stable REST schema documented in OpenAPI).
- **Timestamps:** ISO-8601 UTC strings where timestamps appear.
- **Enums:** string tokens matching CLI flags (`plain`, `jsonl`, `html`, `pdf`, log levels, etc.).
- **Parity alignment:** `investigate`, `analytics`, and `export` response **counts and section payloads** SHALL match CLI `--format json` for the same config, source, and criteria. Parity tests are release gates (§10).

#### Success envelope (typical)

```json
{
  "data": { }
}
```

For list endpoints, `data` may be a JSON array. Binary export (`pdf`) returns `Content-Type: application/pdf` with raw body (no wrapper).

#### Error envelope

Map `foundation::Error` / `foundation::ErrorCode` to HTTP status:

| HTTP | ErrorCode (examples) | When |
|------|----------------------|------|
| 400 | `InvalidArgument` | Malformed JSON, bad filter DSL, missing required field |
| 404 | `NotFound` | Unknown extension id, missing session file |
| 409 | `InvalidState` | Analyze before open, tail already running |
| 413 | — | Upload exceeds `web.max_upload_bytes` |
| 500 | `Internal` | Unexpected failure |
| 503 | — | Server shutting down |

```json
{
  "error": {
    "code": "INVALID_ARGUMENT",
    "message": "Human-readable detail",
    "details": { }
  }
}
```

`code` is a stable UPPER_SNAKE token for clients; `message` is safe to display in UI.

#### Pagination (investigate)

M15.1 returns full `InvestigationResult` (same as CLI). M15.2+ may add `limit` / `offset` query fields without changing default behaviour (full result when omitted).

### 5. Auth boundary (v1)

| Concern | M15 decision |
|---------|----------------|
| Authentication | **None by default** — suitable for localhost / trusted LAN |
| Optional API key | When `web.api_key` is set (or `LOGSCOPE_WEB_API_KEY` env), require header `X-LogScope-Api-Key: <key>` on all `/api/v1/*` routes; health may remain open |
| TLS | Out of scope for embedded server v1; terminate TLS at reverse proxy if needed |
| RBAC / multi-tenant / OAuth | **Deferred to M16** |
| CORS | `web.cors_origins` list; default `http://127.0.0.1:8080` and `http://localhost:8080` for dev SPA |

Document in handbook: **do not bind `0.0.0.0` without API key** in untrusted networks.

### 6. Static SPA vs server-rendered (M15.2 MVP)

**Decision: static SPA** served by the same `logscope-web` process.

| Approach | M15 |
|----------|-----|
| **Static SPA** (HTML/JS/CSS in `apps/web/ui/dist/`, served at `/`) | **M15.2 MVP** |
| Server-rendered templates (Qt WebEngine, C++ HTML generation) | Rejected — duplicates desktop, slower iteration |
| Separate frontend repo / CDN-only | Deferred — optional later; M15 ships embedded assets in release binary |

SPA calls `/api/v1/*` via `fetch`. Deep links use client-side routing (`/investigate`, `/export`); server falls back to `index.html` for unknown GET paths under `/` (SPA history API).

Build: lightweight vanilla JS or small Vite bundle; no React requirement for MVP. Asset pipeline is a CMake custom target `logscope_web_ui` (optional `LOGSCOPE_WEB_UI_BUILD=ON`).

### 7. File upload and remote log input

| Input mode | Mechanism | Default |
|------------|-----------|---------|
| **Browser upload** | `POST /api/v1/sources/upload` — `multipart/form-data`, field `file` | **Enabled** |
| **Server path** | `POST /api/v1/sources/open` — `{ "path": "/abs/or/rel/path" }` | **Disabled** unless `web.allow_server_paths=true` |

Upload flow:

1. Validate size ≤ `web.max_upload_bytes` (default **256 MiB**; configurable).
2. Write to `web.upload_temp_dir` (default OS temp / `logscope-upload-*`).
3. Call `openSource(tempPath, options)`.
4. Track temp path on session for cleanup on session expiry or `DELETE /api/v1/sources/current`.

**Path safety (server path mode):**

- Reject `..`, NUL, and non-normalized paths.
- Resolve relative paths only under `web.allowed_path_roots` (list of directories).
- Deny open if resolved path escapes allowed roots (path traversal guard).

### 8. Threading and concurrency

- **httplib thread pool** serves concurrent HTTP connections (default pool size ≈ `std::thread::hardware_concurrency()`).
- **Session model:** each client workspace is a **server-side session** with its own `ApplicationService` instance (or equivalent session object holding service state). Session id returned on create (`POST /api/v1/sessions/workspace` or first upload); subsequent requests pass `X-LogScope-Session: <uuid>` cookie/header.
- **Per-session mutex:** serialize mutating calls (`open`, `analyze`, `investigate`, …) on the same session; concurrent reads on immutable model allowed after analyze completes.
- **No global mutable ApplicationService** — avoids cross-user interference when multiple browser tabs or API clients connect.
- **Long operations:** M15.1 runs analyze synchronously in worker thread with HTTP timeout (`web.request_timeout_seconds`, default 300). M15.3 may add `202 Accepted` + job poll for very large logs (`large-app.log` persist-index path).
- **Tail / follow:** optional; if exposed in M15.3, poll endpoint is non-blocking; tail state is per-session (same as desktop `TailWorker` semantics via service API).
- **Diagnostics:** ensure `scope_diagnostics` logging is thread-safe before enabling concurrent web sessions (see [NEXT-VALUE-ADD.md](../../planning/NEXT-VALUE-ADD.md) thread-safe diagnostics item).

### 9. Non-goals (M15)

| Item | Target |
|------|--------|
| Full RBAC, organizations, audit log | M16 |
| K8s Helm charts, gRPC, OpenTelemetry collectors | M17 |
| Plugin marketplace / `logscope install` UX | M12 non-goal |
| Replacing CLI for scripting | CLI stays primary |
| QML / native desktop changes | M14 complete |
| Multi-user shared investigations DB | M15.3 slice at earliest |
| WebSocket streaming tail (push) | Post-MVP; poll first |
| Public internet multi-tenant SaaS | Out of product scope |

### 10. Test strategy

Testing follows [TESTING.md](../../testing/TESTING.md) layers and extends them for C12. Labels: `scope_web_tests`, `logscope_web_integration_tests`.

#### 10.1 Unit tests — HTTP handlers (mock ApplicationService)

Location: `apps/web/tests/`

- **Mock** `ApplicationService` (GoogleMock) injected into handler/router layer.
- Cover: route registration, JSON parse/validate failures → 400 + error envelope, auth middleware (missing/invalid API key → 401), session header required → 401/400, path traversal rejection on `sources/open`, upload size limit → 413.
- No network I/O in unit tests; call handler functions directly or use httplib `Client` against in-memory server fixture bound to `127.0.0.1:0`.
- Target: one test file per resource group (`health_handler_test.cpp`, `investigate_handler_test.cpp`, …).

#### 10.2 Integration tests — REST against real fixtures

Location: `apps/web/tests/integration/` or `tests/integration/web/`

- Build `logscope-web` with `LOGSCOPE_WEB=ON`; start server on ephemeral port in test fixture.
- **Fixtures:**
  - `samples/sample.log` — functional correctness (open → analyze → investigate).
  - `samples/large-app.log` — performance smoke (analyze completes within CI budget; optional persist-index config).
  - `samples/sample.jsonl` — format profile path.
- Flows: upload multipart → analyze → investigate with `level == ERROR` → export HTML → non-empty body.
- Session round-trip: save → load → investigate count unchanged.
- Extensions: `GET /extensions` lists built-ins; bad plugin path in config does not break analyze (parity with desktop D6.2).

CTest label: `logscope_web_integration_tests`.

#### 10.3 Parity tests — REST vs CLI

Location: `tests/integration/web/cli_rest_parity_test.cpp` (or Python script `scripts/run_rest_parity.py` mirroring `run_cli_matrix.py`)

For each scenario, run CLI with `--format json` and REST with equivalent body; assert:

| Scenario | Assert |
|----------|--------|
| Analyze `sample.log` | Line counts / level histogram match |
| Investigate `--search error` | `matchCount` equal |
| Investigate `--filter 'level == ERROR'` | `matchCount` equal |
| Analytics timeline | Bucket counts match |
| Export JSON report | Section ids and row counts match |
| Agent investigate (noop AI) | Ask `errors` → same match count as CLI |

Fail CI on any drift; fixes require updating either CLI or REST mapper explicitly (no silent tolerance).

#### 10.4 CI job additions

Add **`web`** job to [`.github/workflows/ci.yml`](../../../.github/workflows/ci.yml), patterned after the `desktop` job:

```yaml
web:
  name: Web Build Smoke (Ubuntu)
  runs-on: ubuntu-latest
  steps:
    - uses: actions/checkout@v7
    - name: Configure CMake
      run: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLOGSCOPE_WEB=ON
    - name: Build Web and Tests
      run: cmake --build build --config Release --target logscope-web scope_web_tests logscope_web_integration_tests --parallel
    - name: Run Web Unit Tests
      run: ctest --test-dir build --output-on-failure --build-config Release -L scope_web_tests
    - name: Run Web Integration Tests
      run: ctest --test-dir build --output-on-failure --build-config Release -L logscope_web_integration_tests
```

- Default PR matrix (`build` job) remains `LOGSCOPE_WEB=OFF` for fast feedback.
- Optional follow-up: add `web` smoke on `windows-latest` after M15.1 stabilizes (httplib is cross-platform).
- Parity script can run in `web` job or dedicated `web-parity` job on Ubuntu.

#### 10.5 Headless / browser testing (SPA)

| Phase | Approach |
|-------|----------|
| M15.1 | **API-only CI** — no browser in pipeline |
| M15.2 | Manual smoke checklist in [M15-V210-WEB-SCENARIOS.md](../../planning/M15-V210-WEB-SCENARIOS.md) |
| Post-M15.2 | Optional **Playwright** job: start `logscope-web`, open `/`, upload `sample.log`, assert table row count; run nightly or pre-release only (not blocking PRs initially) |

Rationale: API parity tests give stronger correctness guarantees per unit CI time; Playwright guards UI wiring regressions later.

#### 10.6 Release regression policy

Per [RELEASE.md §8](../../release/RELEASE.md#8-post-release-housekeeping):

- User-visible web defects in a **shipped tag** (empty table, wrong match count vs CLI, broken export, crash) require a **GitHub issue** with label `bug` before or alongside the hotfix.
- Add regression tests in `scope_web_tests` or `logscope_web_integration_tests` when practical (same policy as desktop v2.0.3 `#86`).
- Link issues in `docs/release/vX.Y.Z-RELEASE-NOTES.md` when applicable.

#### 10.7 Performance smoke (`large-app.log`)

In integration or dedicated benchmark step:

- Upload or open `samples/large-app.log` with persist-index enabled.
- Assert analyze returns within **120 s** on Ubuntu CI (adjust after baseline measurement).
- Assert investigate on `level == ERROR` returns within **30 s**.
- Track timing in test log; optional extension to `logscope_benchmarks` for HTTP analyze path in M15.3.

Not a micro-benchmark gate in M15.1 — smoke guard against catastrophic regression only.

#### 10.8 Security tests

| Risk | Test |
|------|------|
| Path traversal on `sources/open` | `../../../etc/passwd` → 400/403, no open |
| Path outside `allowed_path_roots` | absolute path → rejected |
| Oversized upload | generate payload > `max_upload_bytes` → 413 |
| API key enforcement | when key configured, request without header → 401 |
| Session isolation | session A cannot read session B model without id |

Include negative cases in unit and integration suites; document limits in [CONFIGURATION_GUIDE.md](../../handbook/CONFIGURATION_GUIDE.md) when `web.*` keys ship.

### 11. Graduation path from ADR-008

ADR-008 introduced **ApplicationService** so CLI and desktop share one orchestration pipeline. ADR-009 extends that model:

```text
CLI (C09) ────────┐
Desktop (C11) ────┼──► ApplicationService (C10) ──► core engines
Web (C12) ────────┘         ▲
                            │
                     HTTP handlers (thin)
```

Implementation order:

1. **M15.1** — `logscope-web` skeleton, health, session, open/upload, analyze, investigate, extensions; no SPA.
2. **M15.2** — static SPA MVP; export + agent investigate; parity tests green.
3. **M15.3** — shared saved workspaces API, tail poll, async analyze jobs — see [ADR-009-M15.3-Shared-Investigations.md](ADR-009-M15.3-Shared-Investigations.md).
4. **M15.4 / M16** — optional API key hardening, RBAC, TLS deployment guides.

CLI output formatters remain in `apps/cli/`; C12 adds **JSON mappers** from domain types (`AnalysisModel`, `InvestigationResult`, …) to REST schema. Prefer reusing any shared serialization helpers extracted from CLI JSON formatters during M15.1 refactor.

---

## Consequences

### Positive

- Third presentation layer without forking investigation logic.
- cpp-httplib reuse minimizes new dependencies and license review.
- Session-per-workspace model scales to multiple tabs and API clients.
- Parity test matrix prevents web/CLI drift (lesson from desktop v2.0.2 regression).
- Clear M16/M17 boundary for enterprise and cloud features.

### Negative

- Another optional CMake target and CI job (like desktop).
- Session state in memory — server restart loses unsaved work (document; sessions on disk unchanged).
- Synchronous analyze on large logs may block HTTP thread pool until async jobs land (M15.3).
- SPA adds front-end build toolchain (mitigated: optional CMake flag, vendored dist for release).

---

## References

- [ADR-008 Desktop Qt Presentation](ADR-008-Desktop-Qt-Presentation.md)
- [M15-WEB-PLATFORM.md](../../planning/M15-WEB-PLATFORM.md)
- [M15-V210-WEB-SCENARIOS.md](../../planning/M15-V210-WEB-SCENARIOS.md)
- [M14-V200-DESKTOP-SCENARIOS.md](../../planning/M14-V200-DESKTOP-SCENARIOS.md) — scenario format precedent
- [TESTING.md](../../testing/TESTING.md)
- [RELEASE.md §8](../../release/RELEASE.md#8-post-release-housekeeping)
- [application_service.hpp](../../../apps/common/application_service.hpp)
- [ADR-009-M15.3 Shared Investigations](ADR-009-M15.3-Shared-Investigations.md)
- [M15-V220-SHARED-INVESTIGATIONS-SCENARIOS.md](../../planning/M15-V220-SHARED-INVESTIGATIONS-SCENARIOS.md)

---

## Revision history

| Version | Date | Description |
|---------|------|-------------|
| 1.0 | 30-07-2026 | Initial ADR — M15 Web Platform REST (`v2.1.0`) |
| 1.1 | 31-07-2026 | M15.3 amendment reference — shared workspaces, tail poll, async analyze (`v2.2.0`) |
