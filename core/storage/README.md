# Storage Layer

`scope_storage` implements M11 SQLite-backed persistent investigation indexes with hybrid memory spill.

## Shipped

| Release | Capabilities |
|---------|--------------|
| **v1.4.1** | `IndexStore`, `SqliteIndexStore`, `HybridIndexWriter`, `IndexReader`, `QueryPlanner`, session index reuse |
| **v1.4.2** | Batched writes (WAL, 5k commits), progress every 10k lines, `BM_IndexStoreAppend/100000` |

## v1.4.3 (in progress — design)

| Feature | Status |
|---------|--------|
| Schema v2 migration | Design |
| zlib `content` compression | Design |
| `line_json_fields` + DSL pushdown | Design |
| `query_cache` | Design |
| Incremental append | Design |
| FTS5 full-text search | Design |

Acceptance scenarios: [M11-V143-STORAGE-SCENARIOS.md](../../docs/planning/M11-V143-STORAGE-SCENARIOS.md)

Architecture: [M11 planning doc](../../docs/planning/M11-STORAGE-LAYER.md) · [ADR-005](../../docs/architecture/decisions/ADR-005-Storage-Architecture.md)
