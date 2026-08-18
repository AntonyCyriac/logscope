# V2.13.0 Quality & Integrity — Hardening Backlog

| Field | Value |
|-------|-------|
| Document | v2.13.0 Hardening Matrix |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | Approved |
| Created | 18-08-2026 |
| Milestone | [v2.13.0 — Quality & Integrity](https://github.com/AntonyCyriac/logscope/milestone/1) |

---

## Purpose

Single **hardening release** for correctness and platform integrity debt filed as [#185](https://github.com/AntonyCyriac/logscope/issues/185)–[#203](https://github.com/AntonyCyriac/logscope/issues/203). **Not** combined with P2.1 (shipped `v2.12.0`) or **#144-B** (`register_crash_analyzer`).

**Product story:**

> `v2.12.0` made desktop investigation parity real.  
> `v2.13.0` makes the underlying platform trustworthy.

**Queue discipline:** One milestone, priority labels (`priority-p0` / `priority-p1` / `priority-p2`), subsystem tracked in issue titles — no per-issue mini-milestones.

---

## Recommended fix order

```text
Wave 1 (P0)  #188 #189 #195 #203   storage/index + silent plugin failure
Wave 2 (P1)  #200 #201 #194       web session enforcement
Wave 3 (P1)  #185 #198 #186 #191   plugin ABI, query, reporting, session
Wave 4 (P2)  #196 #197 #187 #190 #192 #193 #199
Housekeeping #202                 version string corrected forward in v2.12.0 (v2.11.0 tag unchanged)
```

---

## Issue matrix

| Priority | Issue | Subsystem | Summary |
|----------|-------|-----------|---------|
| **P0** | [#188](https://github.com/AntonyCyriac/logscope/issues/188) | storage | Same-size in-place rewrite → stale index reused |
| **P0** | [#189](https://github.com/AntonyCyriac/logscope/issues/189) | storage | Incremental append after mid-file edit keeps stale prefix |
| **P0** | [#195](https://github.com/AntonyCyriac/logscope/issues/195) | storage | Unsupported `schema_version` silently rebuilt |
| **P0** | [#203](https://github.com/AntonyCyriac/logscope/issues/203) | plugin | Storage `append_line` failure ignored; exit 0 |
| **P1** | [#200](https://github.com/AntonyCyriac/logscope/issues/200) | web | `session_ttl_seconds` not enforced on `/analyze` |
| **P1** | [#201](https://github.com/AntonyCyriac/logscope/issues/201) | web | `job_max_concurrent_per_session` cap not enforced |
| **P1** | [#194](https://github.com/AntonyCyriac/logscope/issues/194) | web | `GET /api/v1/sessions` 500 when directory omitted |
| **P1** | [#185](https://github.com/AntonyCyriac/logscope/issues/185) | plugin | `investigation.search_provider` never calls plugin ABI |
| **P1** | [#198](https://github.com/AntonyCyriac/logscope/issues/198) | query | JSON field `>` / `<` always match nothing |
| **P1** | [#186](https://github.com/AntonyCyriac/logscope/issues/186) | reporting | `--format json` invalid when lines contain C0 controls |
| **P1** | [#191](https://github.com/AntonyCyriac/logscope/issues/191) | session | Save/load drops analytics, timeline, clusters sections |
| **P2** | [#196](https://github.com/AntonyCyriac/logscope/issues/196) | storage | Query cache FNV-1a vs SHA-256; `max_entries=0` stores rows |
| **P2** | [#197](https://github.com/AntonyCyriac/logscope/issues/197) | query | FTS5 `contains()` misses CJK |
| **P2** | [#187](https://github.com/AntonyCyriac/logscope/issues/187) | cli | Analytics config overrides `--bucket`/`--top` (inverted) |
| **P2** | [#190](https://github.com/AntonyCyriac/logscope/issues/190) | cli | Session list ignores non-`*.logscope-session` files |
| **P2** | [#192](https://github.com/AntonyCyriac/logscope/issues/192) | plugin | Duplicate report section on double-load |
| **P2** | [#193](https://github.com/AntonyCyriac/logscope/issues/193) | plugin | Old `api_version` loads with no warning |
| **P2** | [#199](https://github.com/AntonyCyriac/logscope/issues/199) | config | `storage.backend=plugin:<unknown>` passes validation |
| — | [#202](https://github.com/AntonyCyriac/logscope/issues/202) | release | `v2.11.0` tag reports `2.10.1` — **fixed forward in `v2.12.0`**; tag not rewritten (housekeeping) |

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
