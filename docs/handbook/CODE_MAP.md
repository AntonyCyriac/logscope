# LogScope code map (assistant quick reference)

Pointers for AI assistants — **where code and tests live**. Session bootstrap: [AGENTS.md](../../AGENTS.md).

## Layout

| Path | Role |
|------|------|
| `apps/cli/` | CLI binary |
| `apps/desktop/` | Qt desktop (`LOGSCOPE_DESKTOP=ON`) |
| `apps/web/` | REST server + SPA (`LOGSCOPE_WEB=ON`) |
| `apps/common/application_service.*` | Shared orchestration (C10) |
| `core/` | Domain libraries (analysis, investigation engine, workspace container, …) |
| `tests/` | Integration, e2e, regression, benchmarks |
| `samples/` | `sample.log`, `large-app.log`, `web.properties` |

## Web (`apps/web/`)

| Area | Paths |
|------|--------|
| Routes | `web_server.cpp` |
| Investigations (v2.3.0 / M15.5) | `investigation_store.*` — `/api/v1/investigations`, artifacts, manifest |
| Shared workspaces (v2.2.0 compat) | `workspace_store.*` — delegates to `InvestigationStore`; `/api/v1/workspaces` alias |
| Async analyze jobs | `analyze_job_queue.*` |
| Sessions | `session_store.*` |
| SPA assets | `ui/dist/` (Investigations panel) |

## Domain — investigation container (Story 1)

| Area | Paths |
|------|--------|
| Investigation aggregate | `core/workspace/investigation_container.{hpp,cpp}` — manifest, artifacts, `IArtifactHandler` |
| M3 analysis engine | `core/investigation/` — **not** the Story 1 container; avoid `investigation.hpp` name under `workspace/` |

## CLI — investigation commands (v2.3.0)

`logscope investigation create|add|add-note|list|show|open` — dispatch in `apps/cli/cli_parser.cpp`.

## Tests

| Label | Location |
|-------|----------|
| `scope_web_tests` | `apps/web/tests/` |
| `logscope_web_integration_tests` | `apps/web/tests/integration/`, `tests/integration/web/` |

```bash
ctest -C Release -L "scope_web_tests|logscope_web_integration_tests" --test-dir build --output-on-failure
```

## Docs for web work

- [M15-WEB-PLATFORM.md](../planning/M15-WEB-PLATFORM.md)
- [M15-V220-SHARED-INVESTIGATIONS-SCENARIOS.md](../planning/M15-V220-SHARED-INVESTIGATIONS-SCENARIOS.md)
- [V230-CREATE-INVESTIGATION-SCENARIOS.md](../planning/V230-CREATE-INVESTIGATION-SCENARIOS.md)
- [ADR-009](../architecture/decisions/ADR-009-Web-Platform-REST.md) · [M15.3 amendment](../architecture/decisions/ADR-009-M15.3-Shared-Investigations.md) · [M15.5 amendment](../architecture/decisions/ADR-009-M15.5-Investigation-Container.md)
- [openapi-v1.yaml](../api/openapi-v1.yaml) (sketch; investigations routes — see ADR-009-M15.5) · [v2.3.0 release notes](../release/v2.3.0-RELEASE-NOTES.md)

## CI

`.github/workflows/ci.yml` — jobs run **in parallel**; **Web Build Smoke** for web changes; **clang-tidy** often ~12–15 min.

See [Developer Guide](DEVELOPER_GUIDE.md) for build commands.
