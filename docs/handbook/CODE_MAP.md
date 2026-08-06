# LogScope code map (assistant quick reference)

Pointers for AI assistants — **where code and tests live**. Session bootstrap: [AGENTS.md](../../AGENTS.md).

## Layout

| Path | Role |
|------|------|
| `apps/cli/` | CLI binary |
| `apps/desktop/` | Qt desktop (`LOGSCOPE_DESKTOP=ON`) — IDE-aligned bottom tabs: Results \| AI \| Analytics — see [WEB_UI_DESIGN.md](WEB_UI_DESIGN.md) |
| `apps/web/` | REST server + SPA (`LOGSCOPE_WEB=ON`) |
| `apps/common/application_service.*` | Shared orchestration (C10) |
| `core/` | Domain libraries (analysis, investigation engine, workspace container, …) |
| `tests/` | Integration, e2e, regression, benchmarks |
| `samples/` | `sample.log`, `large-app.log`, `web.properties` |

## Web (`apps/web/`)

| Area | Paths |
|------|--------|
| Routes | `web_server.cpp` |
| Investigations (v2.3.0+ / M15.5–M15.7) | `investigation_store.*` — `/api/v1/investigations`, artifacts, open/switch, **`GET …/timeline`** |
| Shared workspaces (v2.2.0 compat) | `workspace_store.*` — delegates to `InvestigationStore`; `/api/v1/workspaces` alias |
| Async analyze jobs | `analyze_job_queue.*` |
| Sessions | `session_store.*` |
| SPA assets | `ui/dist/` — IDE three-pane layout: left artifacts, center viewer, bottom dock (Timeline \| Crash \| AI \| Results) — see [WEB_UI_DESIGN.md](WEB_UI_DESIGN.md) |
| Web E2E (Playwright) | `tests/e2e/web/` — Story Gate browser automation (CI `web` job) |

## Domain — investigation container (Story 1–3)

| Area | Paths |
|------|--------|
| Investigation aggregate | `core/workspace/investigation_container.{hpp,cpp}` — manifest, artifacts, `projectTimeline()` |
| Artifact handlers | `core/workspace/artifact_handler.{hpp,cpp}` — `artifactTypeSupportsSessionOpen()` |
| Timeline projection (Story 3) | `timeline_event.hpp`, `artifact_projector.{hpp,cpp}`, `timeline_projector.{hpp,cpp}` — `IArtifactProjector`, merge + sort |
| Manifest I/O | `core/workspace/investigation_manifest_io.cpp` — **required** by `investigation_store.cpp` (`loadManifest`/`saveManifest`) |
| M3 analysis engine | `core/investigation/` — **not** the Story 1 container; avoid `investigation.hpp` name under `workspace/` |

## CLI — investigation commands (v2.3.0+)

`logscope investigation create|add|add-note|list|show|open|timeline` — Story 2: `add --type`, `--role`; `open --artifact`; Story 3: `timeline` (`--format json|table`, `--limit`, `--order`) — dispatch in `apps/cli/cli_parser.cpp`, `investigation_command.cpp`.

## Tests

| Label | Location |
|-------|----------|
| `scope_web_tests` | `apps/web/tests/` |
| `scope_workspace_tests` | `core/workspace/tests/` — `investigation_timeline_test.cpp` |
| `logscope_web_integration_tests` | `apps/web/tests/integration/`, `tests/integration/web/` |

```bash
ctest -C Release -L "scope_web_tests|logscope_web_integration_tests" --test-dir build --output-on-failure
```

## Docs for web work

- [UI_ARCHITECTURE.md](../architecture/UI_ARCHITECTURE.md) — one Investigation UI, two shells
- [WEB_UI_DESIGN.md](WEB_UI_DESIGN.md) — shell layout
- [M15-WEB-PLATFORM.md](../planning/M15-WEB-PLATFORM.md)
- [M15-V220-SHARED-INVESTIGATIONS-SCENARIOS.md](../planning/M15-V220-SHARED-INVESTIGATIONS-SCENARIOS.md)
- [V230-CREATE-INVESTIGATION-SCENARIOS.md](../planning/V230-CREATE-INVESTIGATION-SCENARIOS.md)
- [V240-UNDERSTAND-EVERYTHING-SCENARIOS.md](../planning/V240-UNDERSTAND-EVERYTHING-SCENARIOS.md)
- [V250-SEE-WHAT-HAPPENED-SCENARIOS.md](../planning/V250-SEE-WHAT-HAPPENED-SCENARIOS.md) — Story 3 timeline (Story Gate closed)
- [ADR-009](../architecture/decisions/ADR-009-Web-Platform-REST.md) · [M15.3](../architecture/decisions/ADR-009-M15.3-Shared-Investigations.md) · [M15.5](../architecture/decisions/ADR-009-M15.5-Investigation-Container.md) · [M15.6](../architecture/decisions/ADR-009-M15.6-Multi-Source-Investigation.md) · [M15.7](../architecture/decisions/ADR-009-M15.7-Investigation-Timeline.md)
- [openapi-v1.yaml](../api/openapi-v1.yaml) · [v2.5.0 release notes](../release/v2.5.0-RELEASE-NOTES.md)

## CI

`.github/workflows/ci.yml` — jobs run **in parallel**; **Web Build Smoke** for web changes; **clang-tidy** often ~12–15 min.

See [Developer Guide](DEVELOPER_GUIDE.md) for build commands.
