# V2.13.0 Quality & Integrity — Hardening Backlog

| Field | Value |
|-------|-------|
| Document | v2.13.0 Hardening Matrix |
| Category | Project Planning |
| Version | 1.2.0 |
| Status | **G0 approved** — Wave 1 chartered; G1 pending |
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
| **v2.13.0** | **Trust / correctness** (this theme) |
| #144-B | Extensibility when real demand justifies it |
| P3 / next bet | Reactive or next product milestone |

**Product story:**

> `v2.12.0` made desktop investigation parity real.  
> `v2.13.0` makes the underlying platform trustworthy.

**Queue discipline:** One GitHub milestone (`v2.13.0 — Quality & Integrity`), priority labels (`priority-p0` / `priority-p1` / `priority-p2`) — no per-issue mini-milestones. **Product UX frozen** until hardening slice ships.

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
G0 ✅ → G1 design → G2 #188 #189 #195 #203 → G3 → G4 → G5 v2.13.0
```

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
| 1.2.0 | 18-08-2026 | G0 Wave 1 approved — #188 #189 #195 #203 in scope; product freeze |
