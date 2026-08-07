# Storage Layer

`scope_storage` implements M11 SQLite-backed persistent investigation indexes with hybrid memory spill.

## Shipped

| Release | Capabilities |
|---------|--------------|
| **v1.4.1** | `IndexStore`, `SqliteIndexStore`, `HybridIndexWriter`, `IndexReader`, `QueryPlanner`, session index reuse |
| **v1.4.2** | Batched writes (WAL, 5k commits), progress every 10k lines, `BM_IndexStoreAppend/100000` |
| **v1.4.3** | Schema v2 migration, zlib `content` compression (store only when smaller), `line_json_fields` + DSL pushdown, `query_cache`, incremental append, FTS5 |

Persisted indexes are typically ~5–6× source log size (FTS5 dominates). `storage.compress_content` helps long repetitive lines only; default `compress_threshold_bytes=256` skips typical short plain-text lines.

Acceptance scenarios: [M11-V143-STORAGE-SCENARIOS.md](../../docs/planning/M11-V143-STORAGE-SCENARIOS.md)

Architecture: [M11 planning doc](../../docs/planning/M11-STORAGE-LAYER.md) · [ADR-005](../../docs/architecture/decisions/ADR-005-Storage-Architecture.md)

## Plugin backends (M12 / v1.5.0)

Alternate storage backends can be registered via dynamic plugins (`storage.backend=plugin:<id>`). See [M12-DYNAMIC-PLUGINS.md](../../docs/planning/M12-DYNAMIC-PLUGINS.md) and [ADR-006](../../docs/architecture/decisions/ADR-006-Plugin-Loading.md).
