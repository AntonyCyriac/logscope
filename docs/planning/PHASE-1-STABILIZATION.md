# Phase 1 — Stabilize v1.x

| Field | Value |
|-------|-------|
| Document | Phase 1 — Stabilize v1.x |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | In progress |
| Created | 30-07-2026 |
| Last Updated | 30-07-2026 |

---

# 1. Purpose

Close **Post-v1 Phase 1 — Stabilize v1.x** at **`v1.5.2`**: documentation, test hardening, medium observability, and engineering CI improvements before **M14 – Desktop Application**.

See [POST_V1_STRATEGIC_ROADMAP.md](POST_V1_STRATEGIC_ROADMAP.md) §5 Phase 1 and [Phase 1 v1.5.2 Scenarios](PHASE-1-V152-SCENARIOS.md).

**Baseline:** `v1.5.1` — **513** automated tests; M0–M13 feature milestones complete.

---

# 2. Dependencies

| Prior work | Phase 1 dependency |
|------------|-------------------|
| M5 — Production readiness | Test layers, CI matrix, release workflow |
| M11 — Storage Layer | Regression guards for persist-index edge cases |
| M12 — Dynamic Plugins | Plugin load metrics, plugin regression tests |
| M13 — AI Assistant | AI isolation regression, CLI matrix agent scenarios |

Phase 1 does **not** require new product features. M14 starts only after `v1.5.2` is tagged.

---

# 3. Phased Delivery

| Phase | Focus | Status |
|-------|-------|--------|
| P1.0 | Planning doc, scenarios | ✅ Complete |
| P1.1 | Tutorials, architecture diagrams, API docs policy | ✅ Complete |
| P1.2 | Regression expansion, flaky storage test fix | ⬜ Planned |
| P1.3 | CLI matrix + e2e expansion, benchmark baselines | ⬜ Planned |
| P1.4 | Query/filter fuzz target + CI | ⬜ Planned |
| P1.5 | Medium observability (`--stats`, plugin metrics, memory) | ⬜ Planned |
| P1.6 | License scan CI, third-party licenses doc | ⬜ Planned |
| P1.7 | Doc sync, `v1.5.2` release, roadmap closure | ⬜ Planned |

---

# 4. Deliverables

## Documentation (P1.1)

- `docs/tutorials/` — task-oriented walkthroughs (analyze, investigate, plugins/AI)
- [COMPONENT_CATALOG.md](../architecture/COMPONENT_CATALOG.md) — diagram includes M11–M13 modules (`scope_ai`, storage, plugins)
- API docs: CI `api-docs` artifact; GitHub Pages **or** documented artifact-only policy in [docs/api/README.md](../api/README.md)

## Testing (P1.2–P1.4)

- Expanded [tests/regression/](../../tests/regression/) — AI isolation, plugin failure isolation
- Fix `SqliteIndexStoreIncrementalAppendTest.UpdatesFingerprintOnFinalize` on Windows CI
- CLI matrix: `agent investigate`, plugin paths ([scripts/run_cli_matrix.py](../../scripts/run_cli_matrix.py))
- New fuzz target for query/filter parser ([tests/fuzz/](../../tests/fuzz/))

## Observability (P1.5)

- `AnalysisStats` — parse duration, lines, bytes on analyze path
- `PluginLoadStats` — loaded/skipped/failed counts and load time
- `foundation::currentProcessMemoryUsage()` — RSS for `--stats`
- CLI `--stats` on `analyze`, `investigate`, `agent investigate`

## Engineering (P1.6)

- [docs/handbook/THIRD_PARTY_LICENSES.md](../handbook/THIRD_PARTY_LICENSES.md)
- CI license scan job for vendored dependencies

---

# 5. Non-goals

- M14 desktop GUI, Qt, or presentation-layer code
- SIMD, zero-copy parsing, thread-pool tuning (deferred to feature milestones)
- Full execution timeline / interactive performance profiler
- Distro packages (DEB/RPM/Homebrew)
- API compatibility / ABI checker automation
- Marketplace or web platform (M15)

---

# 6. Success criteria

- All rows in [PHASE-1-V152-SCENARIOS.md](PHASE-1-V152-SCENARIOS.md) marked complete
- CI green on Linux, Windows, macOS
- [POST_V1_STRATEGIC_ROADMAP.md](POST_V1_STRATEGIC_ROADMAP.md) Phase 1 gaps updated
- `v1.5.2` tagged; M14 design (P1.0 equivalent) is next

---

# 7. Traceability

| Source | Relationship |
|--------|--------------|
| [POST_V1_STRATEGIC_ROADMAP.md](POST_V1_STRATEGIC_ROADMAP.md) §5, §8 | Strategic Phase 1 definition and gap table |
| [ROADMAP.md](../ROADMAP.md) | `v1.0.x` stabilize track |
| [TESTING.md](../testing/TESTING.md) | Test layer conventions |
| [FR-004 — Extend LogScope](../requirements/functional/FR-004-Extend-LogScope.md) | Plugin failure isolation |
