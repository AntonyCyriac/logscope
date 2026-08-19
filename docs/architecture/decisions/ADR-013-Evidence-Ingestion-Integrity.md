# ADR-013 — Evidence Ingestion Integrity

| Field | Value |
|-------|-------|
| Status | **Proposed** (pre-G0) |
| Date | 19-08-2026 |
| Deciders | Architecture (G1) — not yet reviewed |
| Related | [ADR-005-M11.1 Index Integrity](ADR-005-M11.1-Index-Integrity-Hardening.md), [ADR-005-M10.1 Query Trust](ADR-005-M10.1-Query-Trust.md), [ADR-012 Agent-Facing Analysis Contract](ADR-012-Agent-Facing-Analysis-Contract.md), [ADR-009-M15.6 Multi-Source Investigation](ADR-009-M15.6-Multi-Source-Investigation.md), [#148](https://github.com/AntonyCyriac/logscope/issues/148) |

---

## Context

Two trust boundaries have been hardened. [ADR-005-M11.1](ADR-005-M11.1-Index-Integrity-Hardening.md) made **index reuse** fail closed when integrity cannot be proven. [ADR-005-M10.1](ADR-005-M10.1-Query-Trust.md) made **query answers** identical across storage paths, under the principle *never return a false empty*.

There is a third boundary **upstream of both**, and it is unguarded: **which bytes enter the analysis at all**. Index integrity is meaningless if the file that mattered was never opened, and query trust is meaningless if the line that mattered was never indexed. Discovery loss is also the least detectable of the three, because every layer downstream inherits it and none of them can see it happened.

### Measured behaviour

Directory ingestion was exercised against real support bundles from containerized deployments (multi-service pods, rotated logs, mixed artifact types):

| Input | Actual result |
|-------|---------------|
| Bundle root (80 files, 26 directories) | `No log files found in directory.` — nothing ingested |
| One leaf directory containing 54,378 lines | **27 lines indexed, exit 0, no warning** |
| A rotated file from that directory, named explicitly | 6,508 lines — parses correctly |
| 12-instance crash bundle, 253,445 lines | nothing ingested |
| Nested subdirectory one level down | not discovered |

Root cause is `isLogFile` / `listLogFilesInDirectory` in `core/source/source_manager.cpp`: discovery is **non-recursive** and matches **`*.log` by extension only**. In the sampled bundles, 82 of 189 files end in `.log`; the remainder are trace files, rotated `*.log.N` history, stack dumps, provisioning payloads, timestamp-suffixed names, and extensionless status files.

The 54,378-line case is the important one. Log rotation had just rolled the live file over, so `service.log` held 27 lines while the pre-incident history sat in `service.log.001`–`.009`. **Rotation inverts informativeness: the live file is usually the least useful, and it is the only one discovery reads.** 0.05% of the available evidence was analysed and the run reported success.

### Secondary defects found in the same path

- **Evidence identity is destroyed by multi-file ingest.** Directory mode concatenates files and renumbers lines sequentially. A match reports `Line 2` with no file attribution, and `sourceMetadata.source` names the directory. There is no way to resolve a cited line back to a file, which makes [ADR-012 §7](ADR-012-Agent-Facing-Analysis-Contract.md) `evidenceId` unimplementable for directories and blocks any per-instance analysis.
- **An unrecognised timestamp dialect silently disqualifies a file.** A 232-line file in `DD-MM-YYYY` order ingests fully but reports `linesWithTimestamp: 0` and no time range, so it cannot participate in timeline or time-range correlation. The failure is reported as "no timestamps", not "unknown dialect".
- **Broken symlinks are silently ignored** (exit 0, no diagnostic). Real bundles contain them.

Empty-versus-missing is already handled correctly: an empty file exits `0` with zero lines, a missing file exits `1`. That behaviour is retained.

This is the same defect class as [#148](https://github.com/AntonyCyriac/logscope/issues/148) — partial results presented as complete — relocated to the file-discovery layer, where it is more damaging because the loss precedes every safeguard added since.

---

## Decision

### 1. Ingestion is a trust boundary

```text
Every candidate file under a requested source is either ingested,
or reported as skipped with a machine-readable reason.
Silent omission is a defect.
```

"Candidate" means any regular file reachable from the requested path within the configured traversal bound. This invariant, not the discovery heuristic, is the contract; heuristics may improve freely as long as omissions remain visible.

### 2. Discovery is recursive and content-informed

- **Recursive by default**, bounded by `source.max_depth` (default `16`) and `source.max_files` (default `10000`). Bundles are organised by subsystem, so the interesting file is rarely at the root.
- **Classification is content-informed, not extension-only.** A file is a candidate when it is a regular file that sniffs as text (reusing the existing binary-detection heuristic), regardless of extension. Extension becomes a hint for ordering and dialect selection, never the gate.
- Binary files (core dumps, memory maps) are **candidates that are skipped with reason `BINARY_CONTENT`**, not files that were never seen.

### 3. Skipped-file accounting is part of the answer

Discovery emits a census that flows into the [ADR-012](ADR-012-Agent-Facing-Analysis-Contract.md) envelope. Any skip sets `complete: false`.

```json
"provenance": {
  "discovery": {
    "root": "...",
    "candidatesFound": 189,
    "ingested": 163,
    "skipped": [
      { "reason": "BINARY_CONTENT", "count": 4 },
      { "reason": "DEPTH_LIMIT", "count": 0 },
      { "reason": "UNREADABLE", "count": 1, "paths": ["..."] }
    ]
  }
}
```

Reason codes are a closed enum: `BINARY_CONTENT`, `UNREADABLE`, `BROKEN_SYMLINK`, `DEPTH_LIMIT`, `FILE_LIMIT`, `SIZE_LIMIT`, `EXCLUDED_BY_CONFIG`. Text surfaces summarise the census in one line; contract surfaces carry it in full.

**`No log files found` becomes `indeterminate` (exit `2`)** when candidates existed but all were skipped — the caller learns that files were present and rejected, rather than that the directory was empty.

### 4. Rotation groups into logical streams

Files matching a rotation family — `X.log`, `X.log.N`, `X.log.NNN`, `X.<timestamp>` — are grouped into one **logical stream** ordered oldest to newest, and ingested as a continuous sequence. Rotation history is the pre-incident record; treating it as unrelated files, or ignoring it, discards the window in which the cause usually lies.

Grouping is reported in provenance so a consumer can see which physical files composed a stream.

### 5. Every line keeps its file identity

Multi-file ingestion MUST preserve origin. Each indexed line carries its source file and that file's own line number:

```json
{ "file": "proxy/service.log.001", "fileLineNumber": 4213, "streamLineNumber": 51204 }
```

`fileLineNumber` is what gets cited and what `evidenceId` binds to; `streamLineNumber` is ordering only. Without this, a citation cannot be re-verified and per-instance comparison is impossible. This is a prerequisite for [ADR-012 §7](ADR-012-Agent-Facing-Analysis-Contract.md) on any multi-file source.

### 6. Unknown dialects are reported, not absorbed

When a file ingests but yields no parseable timestamps, provenance records `timestampDialect: "unknown"` and discovery raises a structured warning naming the file and a sample line. A file that cannot be placed on a timeline must announce that, because its absence from a timeline is otherwise indistinguishable from having nothing to contribute.

Adding dialects is out of scope here; **reporting the gap** is not.

### 7. Archives stay outside the tool, explicitly

LogScope does **not** extract archives. Bundles arrive as `.tar` / `.tar.gz`, and supporting them in-process would add decompression-bomb, path-traversal, and symlink-escape surface to a tool whose threat model ([ADR-006](ADR-006-Plugin-Loading.md)) assumes trusted local input.

The obligation this ADR does accept: when a requested path **is** an archive, fail with `invalid` and an actionable message naming the extraction step, rather than reporting no log files found. Guarded in-process extraction may be reconsidered in a later ADR with explicit limits.

---

## Consequences

### Positive

- The bundle-root case changes from "nothing found" to a usable answer, which is the difference between LogScope being adoptable as a batch analysis step and not.
- The silent-partial class is closed at its earliest origin, so the guarantees added in v2.13.0 and v2.13.1 now rest on a known-complete input set.
- Rotation grouping recovers the pre-incident window that current discovery structurally cannot see.
- Per-line file identity unblocks evidence citation and per-instance comparison for multi-file sources.

### Negative

- **Recursive discovery is a behaviour change.** A directory that previously analysed one file may now analyse many, changing line counts and durations. Requires a release-note callout and a `--no-recursive` escape.
- Content sniffing costs a read of every candidate's head; bounded by `source.max_files` but non-zero on large bundles.
- Per-line file attribution widens the indexed-line record and the persisted schema, implying a schema version bump under ADR-005.
- Some currently-succeeding invocations become `indeterminate` (exit `2`) where they previously exited `0` having analysed nothing useful. That is the intended correction, but it is a visible break.

### Out of scope

- New timestamp dialects (§6 reports the gap only).
- In-process archive extraction (§7).
- Grouping instances to find outliers, and comparing two runs — both depend on this ADR and are specified separately.
- Packet captures and other non-text evidence.

---

## Compliance

G3 scenario matrix (to be authored at G1): `V2XX-INGESTION-INTEGRITY-SCENARIOS.md`.

Scenarios MUST include, at minimum: a nested bundle root ingests files from every depth within the bound (§2); a directory containing a rotation family ingests the full history as one ordered stream (§4); a directory whose candidates are all skipped yields `indeterminate` + exit `2` with a populated census (§3); a match in a multi-file source resolves to file plus that file's own line number (§5); a file with an unrecognised dialect reports `timestampDialect: "unknown"` with a warning rather than zero timestamps (§6); and an archive path fails with an extraction message rather than "no log files found" (§7).
