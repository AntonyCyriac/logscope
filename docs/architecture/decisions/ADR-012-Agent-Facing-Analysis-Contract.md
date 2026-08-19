# ADR-012 — Agent-Facing Analysis Contract

| Field | Value |
|-------|-------|
| Status | **Proposed** (pre-G0) |
| Date | 19-08-2026 |
| Deciders | Architecture (G1) — not yet reviewed |
| Related | [ADR-005-M10.1 Query Trust](ADR-005-M10.1-Query-Trust.md), [ADR-005-M11.1 Index Integrity](ADR-005-M11.1-Index-Integrity-Hardening.md), [ADR-007 AI Integration](ADR-007-AI-Integration.md), [ADR-009 Web Platform REST](ADR-009-Web-Platform-REST.md), [#186](https://github.com/AntonyCyriac/logscope/issues/186), [#194](https://github.com/AntonyCyriac/logscope/issues/194) |

---

## Context

Every LogScope output surface to date targets a **human reader**: text reports, HTML, PDF, the desktop shell, the SPA. `--format json` exists but is a serialization of the same human report, not a contract.

A second consumer class now exists: **automated debugging pipelines** that drive LogScope as a tool and feed its output into later stages (source correlation, regression-range computation, report generation). These pipelines are typically staged, with each stage backed by a purpose-built tool — ticket retrieval, capture analysis, revision-history queries. In observed pipelines of this shape, **log analysis is the stage most often left untooled**, handled by reading files directly, even though it is the stage whose conclusions gate every stage after it.

LogScope is the natural tool for that slot and is not used there. The blocker is not capability; it is contract. An automated consumer inverts LogScope's current priority order:

| Priority | Human reader | Automated consumer |
|----------|--------------|--------------------|
| 1 | Readable presentation | Output always parses |
| 2 | Useful summary | "No" is distinguishable from "don't know" |
| 3 | Correct detail | Evidence carries provenance |
| 4 | — | Answer is stable across runs and storage paths |

Two open defects demonstrate that the contract is absent rather than merely unspecified:

1. **#186** — `--format json` emits raw C0 control bytes, producing documents that `json.loads` / `jq` reject. Crash-truncated and partially-flushed logs — exactly the inputs a debugging pipeline receives — are the common source of those bytes. A machine consumer cannot use the JSON surface at all on such inputs.
2. **#194** — a documented REST list endpoint returns `500 INTERNAL` on a healthy server when an optional parameter is omitted, so a probing client cannot distinguish "nothing to list" from "server broken".

[ADR-005-M10.1](ADR-005-M10.1-Query-Trust.md) §5 established **explicit failure over silent empty** for query semantics. That principle is necessary for humans and *load-bearing* for machines: a human who receives a suspicious empty result re-runs the query, while an automated consumer records "no matches" as a finding and proceeds to a wrong conclusion.

This ADR defines the contract before any new agent-facing command is implemented.

---

## Decision

### 1. Scope of the contract

The contract governs output consumed programmatically:

- CLI invocations with `--format json`
- REST responses under `/api/v1/*`

It does **not** govern text, markdown, HTML, or PDF, which remain presentation surfaces free to change.

### 2. Envelope and schema version

Every contract response carries a stable envelope. Payload shape varies by command; envelope shape does not.

```json
{
  "schemaVersion": 1,
  "tool": { "name": "logscope", "version": "2.13.1" },
  "command": "analyze",
  "status": "answered",
  "complete": true,
  "provenance": { },
  "warnings": [],
  "data": { }
}
```

`schemaVersion` increments only on a breaking change to envelope or payload semantics. Additive fields do not increment it. Consumers pin on `schemaVersion` and MUST tolerate unknown fields.

### 3. Serialization safety (absolute)

```text
Contract output MUST parse as JSON for every input LogScope accepts.
```

No input that LogScope agrees to analyse may produce unparseable output. All bytes below `0x20` other than the shorthand escapes are emitted as `\u00XX`; invalid UTF-8 sequences are replaced and the substitution is recorded in `warnings`. This closes #186 and makes the invariant testable rather than incidental.

### 4. Three-state status, not two

`status` is the field that separates this contract from the current behaviour:

| `status` | Meaning | Exit code |
|----------|---------|-----------|
| `answered` | Analysis completed faithfully. An empty result set is a valid answer. | `0` |
| `indeterminate` | LogScope could not answer faithfully — unreadable source, integrity failure, unsupported semantics. **No result set is offered.** | `2` |
| `invalid` | Caller error — bad flags, unparseable filter, unknown format. | `1` |

Exit code `2` is new. Today an automated consumer cannot separate "ran, found nothing" from "could not run", because both surface as text on stderr with an ambiguous code. Partiality is **not** a fourth status: it is expressed by `complete: false`, so existing scripts that branch on exit `0` keep working while a conforming consumer checks the flag.

### 5. Completeness is explicit

```json
"complete": false,
"warnings": [
  { "code": "LINE_CAP_REACHED", "message": "...", "detail": { "indexed": 500000, "total": 5000000 } }
]
```

`complete: false` means the answer is real but bounded — an in-memory line cap, a truncated source, a capped top-N, a pushdown fallback that scanned a subset. Any consumer quoting a count as evidence MUST check this flag. Warnings are structured (`code` + `message` + optional `detail`), never free prose only, so a pipeline can branch on `code` without string matching.

### 6. Provenance is mandatory

Evidence quoted into a downstream report must be traceable back to its origin without re-running the tool.

```json
"provenance": {
  "source": { "path": "...", "sizeBytes": 12345, "modifiedAt": "..." },
  "format": { "detected": "plain", "timestampDialect": "iso8601" },
  "index": { "used": true, "reused": true, "integrityVerified": true, "schemaVersion": 2 }
}
```

The `index` block is the direct lesson of [ADR-005-M11.1](ADR-005-M11.1-Index-Integrity-Hardening.md): index reuse decisions changed which evidence was returned, and a consumer had no way to see that a reused index was involved. Reuse and integrity state are now part of the answer, not a log line.

### 7. Stable evidence identity

Cited evidence carries an identity that survives being pasted into a ticket and re-resolved later:

```json
{ "lineNumber": 4213, "level": "error", "timestamp": "...", "text": "...", "evidenceId": "sha256:…:4213" }
```

`evidenceId` binds the source fingerprint to the line number. Re-running against a changed source yields a different id, which is the point: a stale citation is detectable rather than silently re-pointed at different content.

### 8. No AI dependency

Contract output MUST be fully available with `ai.enabled=false` (the default, per [ADR-007](ADR-007-AI-Integration.md)). Structured facts are a deterministic product of analysis. An automated consumer that already has its own reasoning layer must be able to obtain evidence without LogScope making a network call, and without log content leaving the process.

### 9. Contract-first for new agent-facing commands

Commands motivated by automated consumers are specified here before implementation. This ADR fixes their **output contracts**; algorithms are deferred to their own ADRs.

**Comparison of two runs** — the primitive automated pipelines most often lack. Given a known-good and a failing source, return the first divergence, signatures present only in one side, and per-cluster count deltas:

```json
"data": {
  "divergence": { "firstDivergentLine": { "good": 812, "bad": 804 }, "confidence": "high" },
  "onlyInBad": [ { "signature": "...", "count": 12, "firstSeen": "..." } ],
  "onlyInGood": [ { "signature": "...", "count": 3 } ],
  "countDeltas": [ { "signature": "...", "good": 4, "bad": 91 } ]
}
```

Absence is first-class: a signature that vanished from the failing run is as diagnostic as one that appeared, and is reported with equal weight.

**Signature extraction** — machine-readable tokens for source correlation, replacing eyeballed transcription:

```json
"data": {
  "signatures": [
    { "kind": "symbol", "value": "ns::Class::method", "occurrences": 3, "lines": [102, 140, 208] },
    { "kind": "sourceRef", "value": "handler.cpp:214", "occurrences": 1, "lines": [214] },
    { "kind": "exception", "value": "std::bad_alloc", "occurrences": 1, "lines": [980] },
    { "kind": "loaderError", "value": "GLIBC_2.34 not found", "occurrences": 1, "lines": [3] }
  ]
}
```

Each `kind` is a closed enum extended only by `schemaVersion` bump. Every signature carries the lines that produced it, so a consumer can quote evidence for any token it greps.

---

## Consequences

### Positive

- LogScope becomes usable as the log-analysis stage of an automated pipeline, a slot it currently cannot fill for contract reasons alone.
- "Could not answer" stops being indistinguishable from "no matches" — the highest-severity failure mode for an automated consumer.
- Provenance makes evidence citable in an external report and re-verifiable later.
- The trust invariant from ADR-005-M10.1 gains a machine-checkable surface, so parity regressions are caught by contract tests rather than by reading reports.

### Negative

- Exit code `2` is a behaviour change for any script currently treating non-zero as one class. Requires a release-note callout.
- The envelope is more verbose than today's JSON; payload is nested under `data`, which is a breaking change for anyone parsing the current top-level shape. Mitigated by `schemaVersion` and a deprecation window.
- Mandatory provenance means analysis must thread source and index metadata to every output path, touching code that currently formats reports without it.

### Out of scope

- Ranking a triggering failure against its downstream cascade (algorithm; separate ADR).
- Grouping a bundle by instance to find outliers (separate ADR).
- Expected-sequence / absent-event assertions (separate ADR; depends on §9 comparison).
- Packet-capture parsing — remains external.
- Any agent runtime, orchestration, or prompt surface inside LogScope.

---

## Compliance

G3 scenario matrix (to be authored at G1): `V2XX-AGENT-CONTRACT-SCENARIOS.md`.

Contract tests MUST include, at minimum: every accepted input produces parseable JSON (§3); an unreadable source yields `indeterminate` + exit `2` and no result set (§4); a line-capped run yields `complete: false` with a structured warning (§5); a reused index is reported as `reused: true` (§6); and full payload availability with `ai.enabled=false` (§8).
