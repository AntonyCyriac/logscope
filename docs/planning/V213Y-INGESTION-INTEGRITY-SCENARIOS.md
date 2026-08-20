# v2.13.x Ingestion Integrity — Scenario Matrix

| Field | Value |
|-------|-------|
| Document | Ingestion Integrity Scenario Matrix |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | **G1 approved** — Wave 3 steps 1–2 (implementation pending) |
| ADR | [ADR-013](../architecture/decisions/ADR-013-Evidence-Ingestion-Integrity.md) (**Accepted**) |

---

## Purpose

Acceptance matrix for **Ingestion Integrity (H0 Wave 3)** — discovery census, rotation streams, skip reporting, file identity, instance grouping.

**Binding invariant:**

```text
Every analyzed line MUST be attributable to a discovered input.
Every discovered input MUST have explicit disposition:
  ANALYZED | SKIPPED | UNSUPPORTED | FAILED.
Discovered and analyzed MUST remain separate facts.
```

**G0 question:**

> Can LogScope prove that the evidence it analyzes is the evidence it discovered?

---

## G0 Wave 3 — in scope

| Step | Theme | Gate |
|------|-------|------|
| 1 | Ingestion integrity — discovery census, rotation, skips, dialect/archive | G3 II.1–II.3, II.6–II.8 |
| 2 | File identity + instance grouping | G3 II.4–II.5 |

```text
G0 ✅ → G1 ✅ → G2 ⏳ → G3 → G4 → G5 v2.13.x
```

**Out of scope:** failure-shape steps 3–7; full ADR-012 envelope; #144-B; remaining #185–#201.

---

## Scenario matrix

| ID | Theme | Path / fixture | Expected | Test hook |
|----|-------|----------------|----------|-----------|
| II.1 | Recursive discovery | `bundle/root/subsys/app.log` nested 2 levels | Leaf log ingested; census lists intermediate dirs only as traversal context | `SourceDiscoveryTest.NestedBundleIngestsLeafLogs` |
| II.2 | Rotation stream | `service.log` (27 lines) + `service.log.001`–`.003` (history) | **Sum** of all files analyzed, not live-only; rotation group in census | `RotationGrouperTest` + `IngestionIntegrityTest.RotationStreamFullHistory` |
| II.3 | All skipped | Directory of binary-only candidates | Exit **2**; `candidatesFound > 0`; `analyzed == 0`; skip reasons populated | `SourceDiscoveryTest.AllSkippedIndeterminate` |
| II.4 | File identity | Directory with `a.log` + `b.log` | Filter match includes `sourceFile` + `fileLineNumber`; stream line differs from file line | `IngestionIntegrityTest.MultiFileIdentityOnMatch` |
| II.5 | Instance grouping | `inst-a/app.log` + `inst-b/app.log` under same root | Census `instances[]` has two keys; lines attributed under correct instance | `IngestionIntegrityTest.TwoInstanceBundleGrouping` |
| II.6 | Unknown dialect | File with `DD-MM-YYYY` timestamps, no parser match | Ingest succeeds; `timestampDialect: "unknown"`; warning `UNKNOWN_TIMESTAMP_DIALECT` | `IngestionIntegrityTest.UnknownDialectReported` |
| II.7 | Archive path | Request path ends in `.tar.gz` | Exit **1**; message names extract step; not “no log files found” | `SourceDiscoveryTest.ArchivePathRejected` |
| II.8 | Discovered ≠ analyzed | Bundle with mix of text logs + one binary candidate | Exit 0; `complete: false`; `discovery.candidatesFound > discovery.analyzed`; binary in `skipped` | `IngestionIntegrityTest.PartialSkipIncompleteFlag` |

---

## Fixtures

### II.2 — rotation (blocking repro)

```text
proxy/service.log          # 27 lines (post-rotate tail)
proxy/service.log.001      # 6500 lines
proxy/service.log.002      # 6200 lines
```

```bash
logscope analyze proxy/
logscope analyze --format json proxy/ | jq '.sourceMetadata.discovery,.sourceMetadata.analysis'
```

**Expected:** `analysis.streamLineCount` ≈ 12727 (not 27). Census shows rotation group with ordered members. `analysis.complete` true only if no other skips.

### II.4 — file identity

```text
dir/a.log:
  2026-01-01 ERROR first in a
dir/b.log:
  2026-01-01 ERROR first in b
```

```bash
logscope query --filter 'level == ERROR' dir/
```

**Expected:** Two matches; each cites distinct `sourceFile` (or relative path) and `fileLineNumber == 1` within that file; `lineNumber` (stream) differs between matches.

### II.5 — instances

```text
bundle/inst-a/service.log
bundle/inst-b/service.log
```

**Expected:** `discovery.instances` length 2; keys distinguish `inst-a` vs `inst-b`.

---

## Integrity gates (blocking G3)

**Discover/analyze separation:**

- [ ] `DiscoveryCensus` built before first full-file parse line
- [ ] `sourceMetadata.discovery` present on `--format json` analyze output
- [ ] `sourceMetadata.analysis` separate from discovery counts
- [ ] Success exit 0 with `complete:false` possible when skips occurred

**Identity gate:**

- [ ] Persisted index schema v3 stores `source_file` + `file_line_number`
- [ ] Reuse index on v2 databases triggers rebuild or fail-closed per ADR-005

**Exit code gate:**

- [ ] All candidates skipped → exit **2** (not 0)
- [ ] Archive at root → exit **1**

---

## Non-goals (NG)

| ID | Scenario |
|----|----------|
| NG.1 | Run comparison (good vs bad) |
| NG.2 | Trigger vs cascade ranking |
| NG.3 | Numeric aggregation / percentiles |
| NG.4 | ADR-012 full envelope on all CLI commands |
| NG.5 | In-process `.tar.gz` extraction |
| NG.6 | New timestamp dialect implementation |
| NG.7 | Web/desktop investigation UI changes |
| NG.8 | N-of-M divergence report |

---

## Revision history

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 19-08-2026 | G1 scenario matrix — Wave 3 steps 1–2 |
