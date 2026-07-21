# Search Engine

## Purpose

The Search module provides query representation, parsing, and matching over indexed log lines.

It implements **FR-002.1** search capabilities for the investigation workflow.

## Components

| Component | Description |
|-----------|-------------|
| `SearchQuery` | Text, regex, and boolean expression tree |
| `SearchQueryParser` | Parses boolean search expressions |
| `SearchEngine` | Matches queries against `IndexedLine` entries |
| `SearchHistory` | Bounded recent-query history for sessions |
| `validateSearchConfiguration` | Validates `search.saved.*` configuration keys |

## Dependencies

- `scope_analysis` — `LineIndex`, `IndexedLine`
- `scope_foundation` — string utilities, `Result<T>`
- `scope_runtime` — configuration store

## CMake Target

- `scope_search` — static library
- `scope_search_tests` — unit tests
