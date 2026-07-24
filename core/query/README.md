# Query Engine

`scope_query` implements the M10 field-aware filter DSL over `IndexedLine`.

## Capabilities

- Parse expressions such as `level == ERROR AND contains(message, "timeout")`
- Evaluate predicates against indexed line metadata
- Validate `query.saved.<name>` configuration keys

See [M10 planning doc](../../docs/planning/M10-QUERY-LANGUAGE.md) and [ADR-004](../../docs/architecture/decisions/ADR-004-Query-DSL-Grammar.md).
