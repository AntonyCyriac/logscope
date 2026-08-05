# Changelog

All notable changes to this project will be documented in this file.

This project follows the principles of [Keep a Changelog](https://keepachangelog.com/).

Pre-M3 history (M0–M2) is preserved in Git history, project documentation, and the `v0.2.0-design-baseline` tag. **Changelog entries begin at M3 – Architecture Realization.**

---

## [Unreleased]

---

## [2.4.0] - 2026-08-05

**Story 2** — Understand Everything (multi-artifact investigations, active log switch, `pstack`/`core` storage).

### Added

- **Multi-source investigations** — artifact index with `isEntry`, optional `metadata.role`
- **Active log switch** — `POST .../open` with `artifactId`; session tracks active artifact; snapshot only on entry open
- Artifact types **`pstack`** (text) and **`core`** (binary + `sizeBytes` metadata; storage only)
- CLI `investigation add --type`, `--role`; `investigation open --artifact`
- Web SPA artifact list, Switch, pstack upload
- ADR-009-M15.6 Multi-Source Investigation

See [v2.4.0 release notes](docs/release/v2.4.0-RELEASE-NOTES.md).

---

## [2.3.0] - 2026-08-05

**Story 1** — Create an Investigation (portable container, log + note artifacts, save/reopen).

### Added

- **Investigation** aggregate in `scope_workspace` — manifest v1, artifact type handlers (`log`, `note`)
- REST `/api/v1/investigations` (+ artifacts, open); `/api/v1/workspaces` alias preserved
- CLI `logscope investigation` subcommands (`create`, `add`, `add-note`, `list`, `show`, `open`)
- Web SPA **Investigations** panel; `investigationId` on session save
- ADR-009-M15.5 Investigation Container

### Changed

- `WorkspaceStore` delegates persistence to `InvestigationStore` (same on-disk root)
- CI: `dorny/paths-filter@v4` (Node 24 runtime)

See [v2.3.0 release notes](docs/release/v2.3.0-RELEASE-NOTES.md).

---

## [2.2.2] - 2026-08-04

**Security patch** — API key hashing at rest for `logscope-web`.

### Added

- `web.api_key_hash` config key (salted SHA-256); `LOGSCOPE_WEB_API_KEY_HASH` environment override
- `logscope-web --hash-api-key <secret>` helper to generate hashes for properties files
- Startup warning when legacy plaintext `web.api_key` is used in config

### Changed

- API key verification uses constant-time compare; clients still send plain key in `X-LogScope-Api-Key`
- CI: clang-tidy enforces `bugprone-unused-result` for ignored `[[nodiscard]]` returns

### Migration

- **No change required** — existing `web.api_key` / `LOGSCOPE_WEB_API_KEY` continue to work
- **Recommended:** run `logscope-web --hash-api-key <secret>`, set `web.api_key_hash`, remove `web.api_key` from committed config

See [v2.2.2 release notes](docs/release/v2.2.2-RELEASE-NOTES.md) and [Securing logscope-web](docs/handbook/SECURING_LOGSCOPE_WEB.md).

---

## [2.2.1] - 2026-07-31

**M15.4 Thin Auth** — session idle TTL, upload temp cleanup, health key policy.

### Added

- Session idle TTL (`web.session_ttl_seconds`) and capacity eviction (`web.max_sessions`); evicted sessions return `401 SESSION_EXPIRED`
- Upload temp cleanup on replace, evict, and shutdown (`session_resource_cleanup`)
- Optional health API key policy (`web.health_requires_api_key`); bind exposure warning when non-loopback without API key
- Handbook [Securing logscope-web](docs/handbook/SECURING_LOGSCOPE_WEB.md)
- ADR amendment [ADR-009-M15.4-Thin-Auth](docs/architecture/decisions/ADR-009-M15.4-Thin-Auth.md)
- **57** web-labelled tests (`scope_web_tests`, integration, parity)

### Changed

- Stale or unknown `X-LogScope-Session` on mutating routes → **401** `SESSION_EXPIRED` (was `400` for some cases)

### Upgrade notes

- Defaults preserve v2.2.0 behavior (`session_ttl_seconds=0`, `health_requires_api_key=false`); no migration required
- See [v2.2.1 release notes](docs/release/v2.2.1-RELEASE-NOTES.md) and [Securing logscope-web](docs/handbook/SECURING_LOGSCOPE_WEB.md)

---

## [2.2.0] - 2026-07-31

**M15.3 Shared Investigations** — shared workspaces API, tail poll, async analyze jobs.

### Added

- Shared workspaces REST API (`POST/GET/PUT/DELETE /api/v1/workspaces`, `POST .../open`) with file-first JSON persistence (`WorkspaceStore`)
- Tail poll REST (`POST /api/v1/tail/start|stop`, `GET /api/v1/tail/poll`) delegating to `ApplicationService` tail APIs
- Async analyze: `POST /api/v1/analyze` returns `202` + `Location` for large sources; `GET /api/v1/jobs/{id}` poll
- `POST /api/v1/sessions/save` optional `workspaceId` to update shared workspace snapshot
- SPA W2 flows: shared workspace list/open/save, tail panel, async analyze UX (`apps/web/ui/dist/`)
- Web config: `web.workspace_dir`, `web.workspaces_list_limit`, `web.async_analyze_threshold_bytes`, `web.job_ttl_seconds`, `web.job_max_concurrent_per_session`
- OpenAPI stub `docs/api/openapi-v1.yaml` (M15.1 + M15.3 routes)
- **47** web tests (`scope_web_tests`, `logscope_web_integration_tests`, `logscope_web_parity_tests`)

### Changed

- `TailingFileLogSource::pollLine` for non-blocking tail poll over HTTP
- `WebServer::stop` stops listener before draining analyze job workers

### Upgrade notes

- Build: `cmake -DLOGSCOPE_WEB=ON && cmake --build build --target logscope-web`
- Release archive: `logscope-web-v2.2.0-*` then `./logscope-web` from extracted directory
- Workspaces default to `{data_dir}/workspaces`; large logs analyze async (default threshold 10 MiB)

---

## [2.1.0] - 2026-07-31

**M15 Web Platform** — REST API and browser MVP over `ApplicationService`.

### Added

- `logscope-web` executable (`apps/web/`) — embedded cpp-httplib server on `127.0.0.1:8080` by default
- REST API `/api/v1/*`: health, workspace sessions, config, upload/open source, analyze, investigate, analytics, export (HTML/PDF/JSON), agent investigate, extensions
- Browser MVP SPA (`apps/web/ui/dist/`) — upload, analyze, search, filter DSL, export, extensions panel, AI ask (noop provider)
- Optional API key (`web.api_key` / `LOGSCOPE_WEB_API_KEY`); health exempt when key configured
- CMake option `LOGSCOPE_WEB=ON`; CI `web` job on Ubuntu
- GitHub Release **`logscope-web-v2.1.0-*`** bundles (binary, SPA `ui/dist`, `samples`)
- **32** web tests (`scope_web_tests`, `logscope_web_integration_tests`) including CLI/REST parity

### Changed

- Shared `logscope_httplib` target for `scope_ai` and web server
- CORS reflects request `Origin` when listed in `web.cors_origins`

### Upgrade notes

- Build web: `cmake -DLOGSCOPE_WEB=ON && cmake --build build --target logscope-web`
- Run from release archive: extract `logscope-web-v2.1.0-*`, then `./logscope-web` from that directory
- Or run: `./build/apps/web/logscope-web` then open `http://127.0.0.1:8080/`
- CLI and desktop unchanged for default workflows

---

## [2.0.6] - 2026-07-30

Hotfix for **v2.0.5 CI** — clang-tidy static analysis on desktop `AiPanel`.

### Fixed

- `AiPanel::runAsk()` — null-guard widget pointers before `m_askEdit->text()` (`clang-analyzer-core.CallAndMessage`).
- `runSummarize()` / `runHints()` — guard `m_outputEdit` consistently.

### Upgrade notes

- No functional change expected; restores green CI tidy gate.

---

## [2.0.5] - 2026-07-30

Hotfix for **v2.0.4 CI / build breaks** — Linux desktop compile failure and directory-analyze e2e regression after `large-app.log`.

### Fixed

- Desktop Linux build: `#include <QDialogButtonBox>` in configuration editor.
- Desktop: handle `[[nodiscard]]` on `openLogFile()` when reloading session source.
- `CliE2eTest.AnalyzeDirectoryProducesCombinedReport` — isolated temp directory instead of `samples/` (which includes `large-app.log`).

### Changed

- Release workflow: artifact archives include version tag in filename (`logscope-vX.Y.Z-…`).
- [RELEASE.md](docs/release/RELEASE.md): versioning policy (patch for bugfix, minor for milestone).

### Upgrade notes

- Replace `v2.0.4` desktop Linux build or wait for this release artifacts.
- CLI unchanged.

---

## [2.0.4] - 2026-07-30

M14.12 desktop CLI parity **Phase C** — configuration editor, open format/profile, session save dialog, clipboard/stdin open.

### Added

- Desktop: Configuration editor, Open dialog (format/profile), Save Session dialog, Open from Clipboard / Stdin.
- `ConfigurationManager::saveToFile()`.
- `samples/large-app.log` (~1 MB fixture).
- `logscope_desktop_tests` expanded to 14 headless GUI scenarios.

### Fixed

- Desktop session load re-opens source when table index is empty after adopt.

### Upgrade notes

- Desktop-only; CLI unchanged.

---

## [2.0.3] - 2026-07-30

Hotfix for **v2.0.2 desktop regression** — empty log table after Open/Analyze and AI Ask `Matches: 0`.

### Fixed

- Log table empty after Open/Analyze when lines were in persistent storage — `fetchIndexedLines()` in `populateTableFromModel()`.
- AI Ask showed `Matches: 0` because `agentInvestigate()` re-analyzed and dropped the in-memory index; reuses existing model when present.
- AI Ask updates the center log table; empty Ask shows a hint instead of silent zero matches.

### Added

- `logscope_desktop_tests` — headless Qt Test (`QT_QPA_PLATFORM=offscreen`): open → 8 rows, Ask `errors` → 4 rows.
- `scope_application_tests` — noop `agentInvestigate` ask and model-reuse cases.
- CI desktop job runs `logscope_desktop_tests`.

### Upgrade notes

- Desktop-only hotfix; no CLI or API changes. Re-download or rebuild `logscope-desktop`.

---

## [2.0.2] - 2026-07-30

M14.12 desktop CLI parity polish — Phase A + B (investigation filters, export sections, index toggles, stats dialog).

### Added

- Desktop **Export Report** dialog with format and section checkboxes (GAP.2); defaults from `report.sections` config.
- Desktop toolbar **Persist index** / **Reuse index** on Analyze (GAP.3).
- Desktop **View → Run Statistics…** — full `--stats` parity via `RunStatsDialog` (GAP.7).
- Desktop investigate **From/To** time range wiring (GAP.1).
- `ApplicationService::validateConfiguration()` on File → Load Configuration… (GAP.5).
- Extension **describe** detail panel when selecting an extension (GAP.6).

### Changed

- Removed redundant export format combo from main toolbar.
- Session save uses default report sections from configuration.
- Planning docs: M14.12 Phase A + B complete.

### Upgrade notes

- No CLI or API changes; desktop UX improvements only. Rebuild or download new desktop binaries.

---

## [2.0.1] - 2026-07-30

Desktop release packaging for Windows and macOS, plus GitHub Actions Node.js 24 runtime updates.

### Added

- Release workflow: `logscope-desktop-windows-amd64.zip` and `logscope-desktop-macos-amd64.tar.gz` (`windeployqt` / `macdeployqt`).

### Changed

- GitHub Actions: `checkout@v7`, `upload-artifact@v7`, `download-artifact@v7`, `action-gh-release@v3` (Node.js 24).
- Handbook and planning docs synced to `v2.0.1` (per-platform desktop builds, CMake options, M14 completion).

### Upgrade notes

- No CLI or API changes from `v2.0.0`; download desktop archives for your OS from GitHub Releases.

---

M14 Desktop Application — Qt Widgets GUI, shared `ApplicationService`, live tail, CLI parity for investigation workflows.

### Added

- `apps/common/` — `scope_application` (`ApplicationService`) shared by CLI and desktop.
- `apps/desktop/` — `logscope-desktop` (Qt6 Widgets): open/analyze, filters, analytics panels, export, sessions, extensions, AI panel, tail toggle, themes.
- `TailingFileLogSource` and `SourceManager::open(path, OpenOptions)` for live tail.
- ADR-008, `M14-DESKTOP-APPLICATION.md`, `M14-V200-DESKTOP-SCENARIOS.md`.
- CI `desktop` job (Ubuntu + Qt6); `scope_application_tests`.

### Changed

- `LogAnalyzer` uses `ApplicationService` for analyze pipeline.
- Component catalog: C11 Desktop, C12 Application orchestration.
- CMake `LOGSCOPE_DESKTOP` option (default OFF).

### Upgrade notes

- CLI behaviour preserved; desktop is an additional install target (`logscope-desktop`).
- Live tail is desktop-first; CLI `--follow` is optional follow-up.

---

## [1.5.2] - 2026-07-30

Phase 1 stabilization — tutorials, regression hardening, `--stats` observability, fuzz expansion, and third-party license CI. **520** automated tests.

### Added

- `docs/tutorials/` and handbook links for analyze, investigate, plugins/AI workflows.
- CLI `--stats` with `AnalysisStats`, `PluginLoadStats`, and process memory RSS snapshot.
- Regression tests: AI summarize isolation, plugin path isolation, storage incremental-append flake fix.
- CLI matrix: `agent investigate` (noop), bad plugin path scenario; session e2e report reproduction.
- `query_filter_fuzz` libFuzzer target; CI fuzz smoke coverage.
- `third_party/manifest.json`, `THIRD_PARTY_LICENSES.md`, and CI `license-scan` job.
- `samples/plugin-bad-path.properties` for plugin isolation matrix testing.

### Changed

- Component catalog diagram includes M11–M13 modules (`scope_ai`, storage, plugins).
- API docs policy: CI `api-docs` artifact (GitHub Pages optional).

### Documentation

- Phase 1 planning closure: `PHASE-1-STABILIZATION.md`, `PHASE-1-V152-SCENARIOS.md`.
- Roadmap and strategic roadmap updated: Phase 1 complete; M14 next.

### Upgrade notes

- No breaking changes for default CLI or configuration behaviour.
- `--stats` is opt-in on `analyze`, `investigate`, and `agent investigate`.

### Known limitations

- API documentation published as CI artifact only (no GitHub Pages yet).
- Performance SIMD / zero-copy work deferred to later milestones.

---

## [1.5.1] - 2026-07-25

M13 AI Assistant — pluggable AI providers, natural-language investigation queries, summaries, anomaly hints, and `logscope agent investigate`. **513** automated tests.

### Added

- `scope_ai` module: `AiProvider`, `NoOpAiProvider`, `HttpAiProvider`, `AiInvestigationAssistant`.
- NL → filter DSL translation with `parseFilterQuery` validation (`NlQueryTranslator`).
- Investigation summaries and anomaly hints with bounded context builders and CLI formatters.
- `logscope agent investigate` with `--ask`, `--summarize`, and `--hints`.
- OpenAI-compatible HTTP client (`HttpAiClient`, cpp-httplib) for local (Ollama) and cloud endpoints.
- Config keys: `ai.enabled`, `ai.provider`, `ai.endpoint`, `ai.model`, `ai.max_context_lines`.
- `samples/ai-noop.properties` for offline AI e2e tests.
- `scope_ai_tests` (42 cases) including HTTP mock-server integration tests.

### Changed

- `config validate` validates `ai.*` keys when present.
- Global CLI help lists `agent investigate`.

### Documentation

- ADR-007 AI Integration (Accepted); M13 planning and acceptance scenarios.
- CLI Reference, User Manual, and module READMEs updated for M13.

### Upgrade notes

- Default config unchanged: `ai.enabled=false` (noop provider; no network).
- Enable HTTP provider with `ai.enabled=true`, `ai.provider=http`, endpoint/model, and `LOGSCOPE_AI_API_KEY`.

### Known limitations

- Full PRD-001 agent roster (`agent design`, `implement`, etc.) remains future work.
- HTTP provider sends bounded log excerpts only; no autonomous remediation.

---

## [1.5.0] - 2026-07-25

M12 Dynamic Plugins — runtime `.so`/`.dll` loading, C ABI provider registration, Plugin SDK, and sample plugins. **462** automated tests.

### Added

- `scope_plugin` module: `SharedLibrary`, `PluginLoader`, `PluginHostApi`, `createConfiguredExtensionManager()`.
- C plugin ABI (`include/logscope/plugin/plugin.h`) with vtable-based parser, report, search, and storage providers.
- `ParserRegistry`, `SearchProviderRegistry`, `StorageBackendRegistry` with host-side adapters.
- Config keys: `plugins.enabled`, `plugins.paths`, `analysis.plugin_format`, `investigation.search_provider`, `storage.backend=sqlite|plugin:<id>`.
- Sample plugins under `examples/plugins/` (report, parser, search, storage) and `logscope_plugin_sdk` CMake helper.
- CI: sample plugin matrix (Ubuntu, Windows, macOS); `plugin_loader_path_fuzz` smoke target.
- Plugin integration tests (`scope_plugin_tests`, 10 cases).

### Changed

- `ExtensionManager` registers dynamic extensions from loaded libraries; `extensions list/describe` shows plugin metadata.
- CLI commands load plugins via `createConfiguredExtensionManager()` (analyze, investigate, query, session, analytics, extensions).
- `ReportSectionRegistry` renders all contributors for a section (built-in + plugin).
- `config validate` checks `plugins.paths` when set.

### Documentation

- ADR-006 Plugin Loading (Accepted); M12 planning docs; PLUGIN_DEVELOPMENT_GUIDE v2.0.0.

### Upgrade notes

- Default config unchanged: plugins opt-in via `plugins.enabled=true` and `plugins.paths`.
- Built-in SQLite storage remains default (`storage.backend=sqlite`).

### Known limitations

- No marketplace, signing, or sandboxing (documented threat model in ADR-006).
- M13 AI Assistant shipped in `v1.5.1`.

---

## [1.4.3] - 2026-07-25

M11 storage remainder — schema v2, compression, JSON field predicates, query cache, incremental append, and FTS5 full-text search. **451** automated tests.

### Added

- Schema v2 persistent index (`meta` snapshot keys, `line_json_fields`, `query_cache`); v1 indexes rebuild on open.
- zlib compression on persisted `content` (`storage.compress_content`, `storage.compress_threshold_bytes`).
- `line_json_fields` EAV table and `QueryPlanner` pushdown for top-level JSON field predicates on JSONL sources.
- `query_cache` materialized filter results (`storage.query_cache.enabled`, `storage.query_cache.max_entries`).
- Incremental append indexing when source log grows (`storage.incremental_append`, default `true`).
- FTS5 `lines_fts` virtual table with `contains()` and M7 text-search pushdown on persisted indexes (`SQLITE_ENABLE_FTS5`).
- `BM_IndexStoreCompressed`, `BM_FtsSearch` benchmarks (informational; not CI-gated).
- `.clang-tidy` with `clang-analyzer-*`; static analysis warnings fail CI.

### Changed

- Opening a schema v1 index rebuilds to schema v2 from the authoritative source log.
- `IndexFingerprint` / `prepareIndexReuse()` distinguish append, rebuild, and unchanged source snapshots.
- `contains(message|content, …)` SQL pushdown uses FTS `MATCH` instead of `LIKE` when a persisted store is attached.

---

## [1.4.2] - 2026-07-24

M11 storage follow-up — bulk index build performance for `--persist-index`. 396 automated tests.

### Added

- Batched SQLite index writes: WAL mode, prepared-statement reuse, and transaction batching (5000 lines per commit).
- Indexing progress log lines every 10,000 persisted lines during analysis.
- `BM_IndexStoreAppend/100000` benchmark and CI regression baseline.
- `AppendsLargeBatchInOrder` storage unit test (12k lines across transaction batches).

### Changed

- `SqliteIndexStore` commits write batches on finalize and teardown to avoid partial transactions.

---

## [1.4.1] - 2026-07-24

M11 – Storage Layer complete (core scope). Introduces `scope_storage`, SQLite-backed hybrid indexing, session index reuse, and basic M10 query pushdown. 392 automated tests.

### Added

- M11.0 `ADR-005` storage architecture and `M11-STORAGE-LAYER.md`.
- M11.1–M11.3 `scope_storage` module: `IndexStore`, `SqliteIndexStore`, `IndexFingerprint`, `HybridIndexWriter`, `IndexReader`, `QueryPlanner`.
- M11.4 Session version `1.3` with `index.fingerprint`, `index.path`, `index.line_count`; fingerprint reuse on load.
- M11.5 `--persist-index`, `--reuse-index`, `--index-path` CLI flags; storage config keys; `BM_IndexStoreAppend`, `BM_QueryPushdown`.
- `scope_storage_tests` with SQLite CRUD, fingerprint, planner, and config coverage (25 cases).

### Changed

- `AnalysisEngine` spills indexed lines to SQLite when capacity is exceeded or persistence is requested.
- `InvestigationEngine` and analytics read through `IndexReader` with optional SQL pushdown.
- Session load reuses persisted indexes when the source fingerprint still matches.

### Deferred to v1.4.2

- zlib compression, query cache, incremental append indexing, `line_json_fields` table, FTS5.

---

## [1.4.0] - 2026-07-24

M10 – Query Language complete. Introduces `scope_query`, field-aware filter DSL, `--filter` on investigate/search, and `logscope query`. 365 automated tests.

### Added

- M10.0 `ADR-004` query DSL grammar and `M10-QUERY-LANGUAGE.md`.
- M10.1–M10.3 `scope_query` module with lexer, parser, `QueryEvaluator`, and unit tests.
- M10.4 Investigation integration, session `filter.expression`, and `query.saved.*` config validation.
- M10.5 `--filter` CLI flag, `logscope query` subcommand, e2e/matrix coverage, `BM_QueryEvaluator`.
- Bulk-log CLI matrix scripts and CI/release smoke coverage (from prior merge).

### Changed

- `InvestigationEngine::investigate()` applies DSL filters after M7 text search and legacy field filters.
- Release workflow runs bulk-log CLI matrix before publishing binaries.

---

## [1.3.1] - 2026-07-24

M9 – Analytics Engine complete. Introduces `scope_analytics`, `logscope analytics`, and analytics/timeline/clusters report sections with time-series charts. 337 automated tests.

### Added

- M9.1 `scope_analytics` module and `FrequencyAnalyzer`; `M9-ANALYTICS-ENGINE.md`.
- M9.2 `ErrorClusterer` with normalized message signatures.
- M9.3 `TimelineAnalyzer`, `TrendAnalyzer`, and timeline bucket sizing.
- M9.4 `CorrelationAnalyzer` refactor; `logscope analytics` CLI; analytics config keys.
- M9.5 Time-series charts, `analytics`/`timeline`/`clusters` report sections; `BM_AnalyticsEngine`.

### Changed

- `InvestigationEngine::findCorrelations` delegates to `CorrelationAnalyzer`.
- `Charts` report section includes timeline chart when timeline data is available.

---

## [1.3.0] - 2026-07-24

M8 – Advanced Reporting complete. Introduces section registry architecture, executive/error/chart sections, HTML and PDF formats, and `--output` file writing. 326 automated tests.

### Added

- M8.1 `ReportSectionRegistry`, `ReportFragment`, `FormatRenderer`; refactored `ReportFormatter`; `M8-ADVANCED-REPORTING.md`.
- M8.2 `executive` and `errors` report sections across text, JSON, CSV, and Markdown.
- M8.3 `chart_model`, ASCII and SVG level bar chart renderers; `charts` section.
- M8.4 `ReportFormat::Html`, self-contained HTML reports, CLI `--output <file>`.
- M8.5 ADR-003 minimal PDF writer, `ReportFormat::Pdf`, `ReportSectionContributor` hook, `formats` footer section.
- Config keys: `report.format`, `report.include_charts`, `report.template`.

### Changed

- `Report` supports binary PDF payloads via `bytes()` and `mimeType()`.
- `reporting.multi-format` extension registers report section contributor on init.

---

## [1.2.0] - 2026-07-21

M7 – Search Engine complete. Introduces a dedicated search subsystem with boolean queries, regex mode, CLI search workflow, and session search history. 318 automated tests.

### Added

- M7.1 `scope_search` module: `SearchQuery`, `SearchEngine`, `text_matcher`, investigation integration.
- M7.2 regex search mode with pattern validation and length limits.
- M7.3 boolean query parser (`AND`, `OR`, `NOT`, quoted terms, parentheses).
- M7.4 CLI `--query`, `--regex`, `--case-sensitive`; `logscope search` subcommand; investigation output metadata.
- M7.5 session serializer v1.2 (`search.query`, `search.history`); `search.saved.*` config keys; `BM_SearchEngine` benchmark.

### Changed

- `InvestigationEngine` delegates content matching to `SearchEngine`.
- `Configuration::keys()` added for search configuration validation.

---

## [1.1.0] - 2026-07-21

M6 – Log Format Intelligence complete. Extends analysis with format detection, JSON Lines parsing, field extraction, content-aware investigation, and configuration-driven format profiles. 296 automated tests.

### Added

- Post-v1 strategic roadmap (`POST_V1_STRATEGIC_ROADMAP.md`) with 10-phase vision, M6–M17 milestone mapping, and version targets.
- M7 Search Engine planning stub (`M7-SEARCH-ENGINE.md`).
- M6.1 format detection: `LogFormat`, `FormatDetector`, report metadata `format`, and CLI `--log-format auto|plain|jsonl`.
- M6.2 JSON Lines parsing: per-line JSON validation, `JsonLinesSummary` stats (valid lines, parse failures, top-level keys), field-aware level mapping, and `samples/sample.jsonl`.
- M6.3 field extraction: `FieldSummary` with time range and top message patterns; plain-text timestamp prefixes; JSON `timestamp`/`message` fields (hand-rolled parser extended; nlohmann/json deferred).
- M6.4 content-aware investigation: bounded `LineIndex`, `searchContent`, `TimeRangeFilter`, `FieldFilter`, correlation summary, `investigate` CLI subcommand, and session serializer v1.1 content filter persistence.
- M6.5 format profiles: `AnalysisConfig`, built-in profiles (`generic-plain`, `generic-json`), configuration keys (`source.format`, `source.json.timestamp_field`, `source.json.level_field`, `investigation.max_indexed_lines`), CLI `--profile`, `config validate` extensions, and `FormatParser` interface.

### Fixed

- Analyze failures now print actionable error messages to stderr (including unsupported binary input).
- Explicit `Error` copy/move operations to silence clang-analyzer false positives in `Result<T>` error propagation.

---

## [1.0.0] - 2026-07-18

First stable production release. Delivers the complete M0–M5 roadmap: core pipeline, feature expansion, and production readiness with 246 automated tests.

### Added

#### M5 – Production Readiness

- M5 planning document and phased roadmap (M5.1–M5.5).
- Google Benchmark harness under `tests/benchmarks/` with baseline JSON and `PERFORMANCE.md`.
- ADR-002: benchmark framework selection.
- Malformed-input unit tests for session serializer, configuration, and CLI parser.
- libFuzzer targets for session serializer and configuration (`LOGSCOPE_FUZZING=ON`).
- Sanitizer build preset (`LOGSCOPE_SANITIZE=ON`) and `clang-tidy` target.
- Multi-OS CI matrix (Ubuntu, Windows, macOS), coverage, benchmark, fuzz, and tidy jobs.
- CMake `install()` and CPack packaging for source distributions.
- Tag-triggered release workflow with per-OS binary artifacts.
- Documentation: `TESTING.md`, `RELEASE.md`, `V1_VALIDATION.md`, `SECURITY_REVIEW.md`, `CLI_REFERENCE.md`.

### Changed

- `SessionSerializer::deserialize` uses safe numeric parsing (`std::from_chars`) instead of throwing `std::stoull`.
- CI expanded from single Ubuntu job to full M5 quality gate pipeline.
- `InvestigationSession` default constructor explicitly deleted.

### Fixed

- Session deserialization no longer throws on invalid numeric fields.
- CMake CMP0135 `FetchContent` timestamp warnings (`DOWNLOAD_EXTRACT_TIMESTAMP`).
- Duplicate static library linkage in integration test target.

---

## [0.4.0] - 2026-07-18

M4 – Feature Expansion complete. Extends the M3 pipeline with analysis depth, richer sources, advanced reporting, extensions, and session persistence, with 230 automated tests.

### Added

#### M4.1 – Analysis Depth

- `LogLevelCounts` and `detectLogLevel` for generic INFO/WARN/ERROR statistics.
- `LogLevelFilter` for investigation filtering by error and warning thresholds.
- Text and JSON reports include per-level line counts.

#### M4.2 – Additional Source Types

- `StdinLogSource` for pipe-friendly workflows (`logscope analyze -`).
- `CompositeLogSource` and directory support for multi-file datasets.
- `FileSystem::listRegularFiles` for directory enumeration.
- Clear unsupported-input errors for empty directories and invalid sources (FR-001.4).

#### M4.3 – Advanced Reporting

- Report sections: summary, level breakdown, and source metadata.
- Section selection via `--sections` CLI flag or `report.sections` configuration key (FR-003.2).
- CSV and Markdown output formats alongside text and JSON (FR-003.4).
- Unified formatting in `core/reporting` for reproducible output (FR-003.6).

#### M4.4 – Extension Ecosystem

- `ExtensionManager` for built-in extension registration and lifecycle (C06).
- Configuration-based enablement via `extensions.<id>.enabled` keys (FR-004.1).
- CLI commands `extensions list` and `extensions describe` (FR-004.5).
- Failure-isolated extension initialization during analyze (FR-004.4).

#### M4.5 – Session / Workspace

- `core/workspace` module with `InvestigationSession` and `SessionStore`.
- Persist analysis metadata, investigation filters, search query, and report preferences.
- CLI commands `session save`, `session load`, and `session list` (FR-002.5).

---

## [0.3.0] - 2026-07-18

M3 – Architecture Realization complete. Delivers the full core pipeline from configuration through CLI, with 186 automated tests.

### Added

#### M3.1 – Repository Architecture

- Introduced workspace model directory layout (`core/`, `docs/`, `apps/`, and related areas).
- Established continuous integration workflow (build and test on push/PR).
- Added CMake `format` target for `clang-format`.

#### M3.2 – Foundation Library

- `Status` type for lightweight operation outcomes.
- `Error` and `ErrorCode` types for structured error reporting.
- `Result<T>` template for type-safe, exception-free error handling.
- `Version` value type with semantic versioning support.
- `Uuid` value type with RFC 4122 parsing, version 4 generation, and comparison operators.
- `Time`, `Date`, and `DateTime` value types with parsing and comparison.
- `Duration` value type for non-negative time intervals with nanosecond precision.
- `Timestamp` value type for absolute UTC instants as Unix nanoseconds since epoch.
- `Clock` wall-clock source returning current UTC `Timestamp`.
- `Stopwatch` monotonic elapsed-time measurement returning `Duration`.
- `Random` seedable pseudo-random number generator.
- `String` utilities for trim, case conversion, prefix/suffix checks, and split.
- `Hash` FNV-1a hashing and hash-combine helpers.
- `Path` and `FileSystem` for filesystem path handling and file operations.
- Unit tests for all Foundation components (69 tests via GoogleTest).
- `scope_foundation` static library and `scope_foundation_tests` test target.

#### M3.10 – Integration and End-to-End Testing

- `logscope_integration_tests` for the Source → Analysis → Investigation → Reporting pipeline.
- `logscope_e2e_tests` for CLI executable smoke tests against `samples/` fixtures.
- Repository `tests/` directory wired into the root CMake build.

#### M3.9 – CLI

- `CliApplication`, `analyze`, and `config validate` subcommands with text and JSON output formats.
- Legacy `logscope <log-file>` invocation preserved for backward compatibility.
- `logscope_cli_tests` unit test target for CLI parsing and output formatting.

#### M3.7 – Investigation Engine

- `InvestigationView`, `LineCountFilter`, and `InvestigationEngine` for inspecting, filtering, and searching analysis models.
- `scope_investigation` library and `scope_investigation_tests` test target.
- CLI `LogAnalyzer` logs an investigation summary before report generation.

#### M3.8 – Reporting Engine

- `Report` and `ReportGenerator` for text reports from `AnalysisModel`.
- `scope_reporting` library and `scope_reporting_tests` test target.
- CLI `LogAnalyzer` refactored to render output via `ReportGenerator`.

#### M3.6 – Analysis Engine

- `AnalysisModel` and `AnalysisEngine` for transforming `SourceDataset` into analysis results.
- `scope_analysis` library and `scope_analysis_tests` test target.
- CLI `LogAnalyzer` refactored to use `AnalysisEngine` for log analysis.

#### M3.5 – Source Manager

- `LogSource`, `FileLogSource`, `SourceDataset`, and `SourceManager` for file-based log source acquisition.
- `scope_source` library and `scope_source_tests` test target.
- CLI `LogAnalyzer` refactored to read logs through `SourceManager`.

#### M3.4 – Configuration Manager

- `ConfigurationManager` for loading properties files, applying `SCOPE_` environment overrides, and validating required keys.
- `scope_configuration` library and `scope_configuration_tests` test target.
- CLI `--config` flag to load configuration from a properties file before analysis.

#### M3.3 – Platform Services (Runtime)

- `Configuration` key-value store.
- `Diagnostics` logging facility with level filtering, category tags, UTC timestamps, and `SCOPE_LOG_*` macros.
- `PluginRegistry` and `ServiceRegistry`.
- `scope_runtime` library and `scope_runtime_tests` test target.
- Runtime and CLI flow tracing via `Diagnostics` (`log.level` / `SCOPE_LOG_LEVEL`).

#### Applications

- CLI migrated from legacy `src/` + `include/` to `apps/cli/`.
- CLI linked to `scope_foundation` and `scope_runtime` using `scope::cli`, `scope::foundation`, and `scope::runtime` namespaces.

#### Documentation

- ADR-001: unit testing framework selection (GoogleTest).
- C++ Coding Standard and API Design Guidelines.
- Foundation Guidelines (renamed from engineering guidelines; moved under `docs/standards/`).
- Handbook workflow docs: Git Conventions, Pull Request Guide, Code Review Checklist.
- Updated Document Map, README, and cross-links across standards documents.

### Changed

- Migrated Foundation unit tests to GoogleTest.
- Reorganized engineering documentation into standards and handbook layers.
- Deduplicated C++ Coding Standard with Foundation Guidelines.
- Renamed `.github/README.md` to `.github/GITHUB.md` so the root README displays on the repository homepage.

---

## Prior Releases

| Tag | Scope |
|-----|-------|
| `v1.0.0` | First stable release: M5 production readiness, multi-OS CI, packaging, and distribution. |
| `v0.4.0` | M4 complete: analysis depth, additional sources, advanced reporting, extensions, and session persistence. |
| `v0.3.0` | M3 complete: core pipeline, CLI framework, integration and end-to-end tests. |
| `v0.2.0-design-baseline` | M0–M2 complete: engineering foundation, product vision, and design baseline. |
