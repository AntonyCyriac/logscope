# V2.13.0 Quality & Integrity — Hardening Backlog

| Field | Value |
|-------|-------|
| Document | v2.13.0 Hardening Matrix |
| Category | Project Planning |
| Version | 1.6.0 |
| Status | **Shipped** — `v2.13.0` Wave 1 (`e8be380`) |
| Created | 18-08-2026 |
| Milestone | [v2.13.0 — Quality & Integrity](https://github.com/AntonyCyriac/logscope/milestone/1) |

---

## Purpose

**Hardening theme** for correctness and platform integrity debt filed as [#185](https://github.com/AntonyCyriac/logscope/issues/185)–[#203](https://github.com/AntonyCyriac/logscope/issues/203). **Not** combined with P2.1 (shipped `v2.12.0`/`v2.12.1`) or **#144-B** (`register_crash_analyzer`).

**G0 defines the release slice** — do **not** commit to fixing all 19 issues in `v2.13.0` before G0 sign-off. Waves below are a **priority backlog**, not a locked scope.

**Product sequence (locked):**

| Track | Meaning |
|-------|---------|
| P2.1 | User value (shipped `v2.12.0` + `v2.12.1`) |
| **v2.13.0** | **Trust / correctness** (shipped Wave 1) |
| #144-B | Extensibility when real demand justifies it |
| P3 / next bet | Reactive or next product milestone |

**Product story:**

> `v2.12.0` made desktop investigation parity real.  
> `v2.13.0` makes the underlying platform trustworthy.

**Queue discipline:** One GitHub milestone (`v2.13.0 — Quality & Integrity`), priority labels (`priority-p0` / `priority-p1` / `priority-p2`) — no per-issue mini-milestones. Wave 1 **shipped** in `v2.13.0`; deferred items remain in backlog.

---

## G0 framing question

> **Which existing correctness failures most threaten trust in an investigation?**

G0 answers this and charters the **minimum slice** for `v2.13.0`. Expected bias: **storage/index integrity** (P0) before plugin ergonomics and cosmetic P2 items.

**G0 outputs:** chartered issue list · acceptance criteria · explicit deferrals to `v2.13.x` or backlog.

---

## G0 Wave 1 — in scope for `v2.13.0` (approved 18-08-2026)

| Issue | Subsystem | Summary | Gate |
|-------|-----------|---------|------|
| [#188](https://github.com/AntonyCyriac/logscope/issues/188) | storage | Same-size in-place rewrite → stale index reused | G1→G5 |
| [#189](https://github.com/AntonyCyriac/logscope/issues/189) | storage | Incremental append after mid-file edit keeps stale prefix | G1→G5 |
| [#195](https://github.com/AntonyCyriac/logscope/issues/195) | storage | Unsupported `schema_version` silently rebuilt | G1→G5 |
| [#203](https://github.com/AntonyCyriac/logscope/issues/203) | plugin | Storage `append_line` failure ignored; exit 0 | G1→G5 |

**Product freeze:** no new UX or architectural ambitions. **No `feat/v2.13.0-*` implementation branch until G1.**

**Deferred** (same hardening theme; later wave or `v2.13.x`): #185–#187, #190–#202, #194, #196–#199.

```text
G0 ✅ → G1 ✅ → G2 ✅ → G3 ✅ → G4 ✅ → G5 ✅ v2.13.0
```

## Scenario matrix (Wave 1 — G3)

Status key: **P** planned · **U** unit · **Int** integration · **E** CLI E2E · **R** regression

| ID | Scenario | Unit | Integration | E2E | Notes |
|----|----------|------|-------------|-----|-------|
| QI.1 | [#188](https://github.com/AntonyCyriac/logscope/issues/188) same-size rewrite invalidates stale index | U | Int | — | `SourceSnapshotTest.DetectsSameSizeContentRewrite`, `IndexReuseTest.RebuildsWhenSameSizeContentChanges` |
| QI.2 | [#189](https://github.com/AntonyCyriac/logscope/issues/189) mid-file edit before growth → rebuild | U | Int | — | `SourceSnapshotTest.DetectsMidFileEditBeforeGrowth`, `IndexReuseTest.RebuildsWhenMidFileEditPrecedesGrowth` |
| QI.3 | mtime-only change with same content → reuse allowed | U | Int | — | `SourceSnapshotTest.IgnoresMtimeOnlyChange`, `IndexReuseTest.ReusesUnchangedSource` |
| QI.4 | [#195](https://github.com/AntonyCyriac/logscope/issues/195) unsupported future schema fail-closed | U | Int | — | `SqliteIndexStoreTest.RejectsUnsupportedFutureSchema`, `IndexReuseTest.FailsClosedOnUnsupportedFutureSchema` |
| QI.5 | content hash persisted at finalize | U | — | — | `SqliteIndexStoreTest.FinalizePersistsSourceSnapshotMeta` |
| QI.6 | [#203](https://github.com/AntonyCyriac/logscope/issues/203) plugin `append_line` failure → `Error` | — | Int | — | `PluginProviderIntegrationTest.PropagatesPluginStorageWriteFailures` |
| QI.7 | partial-index WARNING preserved (no hard cap abort) | — | — | E | `CliE2eTest.InvestigatePartialIndexWarnsWhenFilterApplied` |
| QI.8 | storage reuse regression (stdin/dir/file) | — | — | R | `StorageRegressionTest.*` |

## Integrity Gate (blocking G3)

**#188 path:**

```text
Build index for log → same-size in-place rewrite (new token)
  → reopen with --reuse-index → rebuild (not stale hits)
```

**#189 path:**

```text
Build index → mid-file edit + append growth
  → prefix hash mismatch → rebuild (not stale prefix)
```

**#195 path:**

```text
Open index with schema_version > supported
  → Error (no silent rebuild, no DB delete)
```

**#203 path:**

```text
Analyze with plugin:failing + --persist-index
  → append_line non-zero → analyze Error (not exit 0)
```

## Non-goals (NG)

| ID | Scenario | Expected |
|----|----------|----------|
| NG.1 | #144-B `register_crash_analyzer` | Separate milestone |
| NG.2 | P3 domain events charter | Reactive only |
| NG.3 | New investigation UX | Product freeze |
| NG.4 | Deferred P1/P2 hardening (#185–#202 except Wave 1) | `v2.13.x` backlog |

## G1 design summary (approved 18-08-2026)

**Binding principle:** correctness failures become explicit — never a plausible wrong result.

| Issue | G1 decision |
|-------|-------------|
| #188 | `source_content_sha256` — equal size + hash mismatch → rebuild |
| #189 | Prefix byte-range hash before append — mismatch → rebuild |
| #195 | Future schema fail-closed; v1 explicit error; no silent open-failure fallback |
| #203 | Plugin non-zero → `Error` through adapter/writer/engine; CLI/web fail |

**ADR:** [ADR-005-M11.1](../architecture/decisions/ADR-005-M11.1-Index-Integrity-Hardening.md) (Proposed → Accepted at G4).

**EngOS:** `logscope-strategy/engos/artifacts/v2.13.0-quality-integrity/design.md`

## Priority backlog (full matrix)

| Scope | Priority | Issue | Subsystem | Summary |
|-------|----------|-------|-----------|---------|
| **Wave 1** | P0 | [#188](https://github.com/AntonyCyriac/logscope/issues/188) | storage | Same-size in-place rewrite → stale index reused |
| **Wave 1** | P0 | [#189](https://github.com/AntonyCyriac/logscope/issues/189) | storage | Incremental append after mid-file edit keeps stale prefix |
| **Wave 1** | P0 | [#195](https://github.com/AntonyCyriac/logscope/issues/195) | storage | Unsupported `schema_version` silently rebuilt |
| **Wave 1** | P0 | [#203](https://github.com/AntonyCyriac/logscope/issues/203) | plugin | Storage `append_line` failure ignored; exit 0 |
| Deferred | P1 | [#200](https://github.com/AntonyCyriac/logscope/issues/200) | web | `session_ttl_seconds` not enforced on `/analyze` |
| Deferred | P1 | [#201](https://github.com/AntonyCyriac/logscope/issues/201) | web | `job_max_concurrent_per_session` cap not enforced |
| Deferred | P1 | [#194](https://github.com/AntonyCyriac/logscope/issues/194) | web | `GET /api/v1/sessions` 500 when directory omitted |
| Deferred | P1 | [#185](https://github.com/AntonyCyriac/logscope/issues/185) | plugin | `investigation.search_provider` never calls plugin ABI |
| Deferred | P1 | [#198](https://github.com/AntonyCyriac/logscope/issues/198) | query | JSON field `>` / `<` always match nothing |
| Deferred | P1 | [#186](https://github.com/AntonyCyriac/logscope/issues/186) | reporting | `--format json` invalid when lines contain C0 controls |
| Deferred | P1 | [#191](https://github.com/AntonyCyriac/logscope/issues/191) | session | Save/load drops analytics, timeline, clusters sections |
| Deferred | P2 | [#196](https://github.com/AntonyCyriac/logscope/issues/196) | storage | Query cache FNV-1a vs SHA-256; `max_entries=0` stores rows |
| Deferred | P2 | [#197](https://github.com/AntonyCyriac/logscope/issues/197) | query | FTS5 `contains()` misses CJK |
| Deferred | P2 | [#187](https://github.com/AntonyCyriac/logscope/issues/187) | cli | Analytics config overrides `--bucket`/`--top` (inverted) |
| Deferred | P2 | [#190](https://github.com/AntonyCyriac/logscope/issues/190) | cli | Session list ignores non-`*.logscope-session` files |
| Deferred | P2 | [#192](https://github.com/AntonyCyriac/logscope/issues/192) | plugin | Duplicate report section on double-load |
| Deferred | P2 | [#193](https://github.com/AntonyCyriac/logscope/issues/193) | plugin | Old `api_version` loads with no warning |
| Deferred | P2 | [#199](https://github.com/AntonyCyriac/logscope/issues/199) | config | `storage.backend=plugin:<unknown>` passes validation |
| — | — | [#202](https://github.com/AntonyCyriac/logscope/issues/202) | release | `v2.11.0` tag reports `2.10.1` — **fixed forward in `v2.12.0`** |

---

## Out of scope (v2.13.0)

| Item | Track |
|------|-------|
| #144-B `register_crash_analyzer` | Architecture — when a real second analyzer justifies it |
| P3 domain events charter (ADR-010) | Reactive only |
| New investigation stories / desktop UX | Product queue after hardening |

---

## Revision history

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 18-08-2026 | Hardening backlog locked; milestone + priority labels |
| 1.3.0 | 18-08-2026 | G1 approved — integrity hash model, schema fail-closed, plugin Error propagation |
| 1.4.0 | 18-08-2026 | G3 — scenario matrix statuses; Wave 1 merged [#209](https://github.com/AntonyCyriac/logscope/pull/209) (`d211173`) |
| 1.5.0 | 18-08-2026 | G4 — release notes, CHANGELOG, ADR-005-M11.1 Accepted |
| 1.6.0 | 18-08-2026 | G5 — shipped `v2.13.0`; stale freeze language removed |
