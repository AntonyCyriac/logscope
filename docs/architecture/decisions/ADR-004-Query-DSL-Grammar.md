# ADR-004: Query DSL Grammar

- **Status:** Accepted
- **Date:** 24-07-2026

---

## Context

M7 delivers boolean **text** search (`--query "error AND timeout"`) and orthogonal CLI filters (`--level`, `--time-from`, `--message`, `--json-key`). M10 requires a **field-aware structured DSL** over `IndexedLine` so users can express predicates such as `level == ERROR AND contains(message, "timeout")` in one expression.

Architectural guardrails require an ADR before query DSL implementation. M11 (SQLite) and M13 (natural language) may build on this evaluator later.

---

## Decision

LogScope introduces a dedicated **`scope_query`** module with:

1. A small lexer and recursive-descent parser producing a `QueryNode` AST
2. A `QueryEvaluator` that matches predicates against `IndexedLine`
3. CLI flag `--filter <dsl>` on `investigate` and `search`, plus a `logscope query` subcommand
4. Config keys `query.saved.<name>` for validated saved filter expressions

### Grammar (v1)

```text
expr       := or_expr
or_expr    := and_expr ( OR and_expr )*
and_expr   := unary_expr ( AND unary_expr )*
unary_expr := NOT unary_expr | primary
primary    := comparison | function_call | '(' expr ')'

comparison := IDENT op literal
op         := == | != | > | >= | < | <=
function   := contains '(' IDENT ',' string ')'
             | hasKey '(' string ')'
literal    := STRING | NUMBER | LEVEL_KEYWORD
```

### Supported fields (v1)

| Field | Source |
|-------|--------|
| `level` | `IndexedLine.level` |
| `time`, `timestamp` | `IndexedLine.timestamp` |
| `message` | `IndexedLine.messageExcerpt` |
| `content` | `IndexedLine.contentExcerpt` |
| `correlationId` | `IndexedLine.correlationId` |
| `line` | `IndexedLine.lineNumber` |

Level keywords: `ERROR`, `WARN`/`WARNING`, `INFO`, `OTHER`, `BLANK` (case-insensitive).

### Compatibility with M7

- Existing `--query` continues to use `parseSearchQuery()` (boolean text AST)
- `--filter` uses `parseFilterQuery()` (field DSL AST)
- Pipeline order: text search → time range → field flags → DSL filter (all AND-ed when multiple are active)
- M7 behavior is unchanged when `--filter` is omitted

### Error model

Parse failures return `foundation::Result<QueryNode>` with messages `line:col: <detail>` when position is known.

---

## Non-goals (M10)

- SQL syntax (`SELECT ... FROM logs`)
- Arbitrary JSON path predicates (e.g. `service == "PCF"`) without index extension
- Natural language queries (M13)
- Query provider plugins (M12)
- Persistent query cache (M11)

---

## Consequences

### Positive

- Unified field predicates replace ad-hoc filter flag combinations for common cases
- Clear extension point for M11 index pushdown and M13 NL translation
- Isolated module; M7 search AST remains stable

### Negative

- Two query syntaxes coexist (`--query` text vs `--filter` DSL) until a future consolidation milestone
- O(n) linear scan over in-memory `LineIndex` (same as M7)

---

## Compliance

- CLI-first: no UI-specific logic in `core/query`
- MIT license (project code only)
- CI: unit tests in `scope_query_tests`, e2e and CLI matrix coverage
