# M10 – Query Language

| Field | Value |
|-------|-------|
| Document | M10 – Query Language |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | Approved |
| Created | 24-07-2026 |
| Last Updated | 24-07-2026 |

---

# 1. Purpose

This document defines **M10 – Query Language**, introducing a structured field-aware DSL over indexed log lines, extending M7 boolean text search without replacing it.

Target release: **`v1.4.0`**.

See [ADR-004](../architecture/decisions/ADR-004-Query-DSL-Grammar.md) for grammar and compatibility rules.

---

# 2. Dependency on M6–M9

| Prior output | M10 dependency |
|--------------|----------------|
| M6.4 — Line index | Primary query input (`IndexedLine`) |
| M7 — Search engine | Boolean text search remains; DSL is additive |
| M8 — Reporting | No direct dependency |
| M9 — Analytics | No direct dependency |

---

# 3. Phased Delivery

| Phase | Focus | Status |
|-------|-------|--------|
| M10.0 | ADR-004, planning doc, document map | ✅ Complete |
| M10.1 | `scope_query` module scaffold | ✅ Complete |
| M10.2 | Lexer, parser, AST, unit tests | ✅ Complete |
| M10.3 | `QueryEvaluator` over `IndexedLine` | ✅ Complete |
| M10.4 | Investigation integration, sessions, config | ✅ Complete |
| M10.5 | CLI `--filter`, `logscope query`, `v1.4.0` release | ✅ Complete |

---

# 4. Delivered Capabilities

## Core query

- `QueryNode` AST: comparisons, `contains()`, `hasKey()`, boolean composition
- `parseFilterQuery()` with position-aware errors
- `QueryEvaluator` over `level`, `time`, `message`, `content`, `correlationId`, `line`

## CLI

- `--filter <dsl>` on `investigate` and `search`
- `logscope query` subcommand (DSL-focused, text/json output)
- Config keys: `query.saved.<name>`

## Investigation

- `InvestigationCriteria::filterExpression` integrated in `InvestigationEngine::investigate()`
- Session persistence: `filter.expression`
- M7 `--query` and filter flags remain backward compatible

---

# 5. Engineering Conventions

| Convention | Value |
|------------|-------|
| Module | `core/query` (`scope_query`) |
| Tests | `scope_query_tests` + CLI/e2e coverage |
| Benchmark | `BM_QueryEvaluator` |

---

# 6. Deferred

- SQL-like syntax
- Arbitrary JSON field predicates (`service == "PCF"`) — requires index extension or M11
- Persistent query cache (M11)
- Natural language queries (M13)
- Query provider plugins (M12)
