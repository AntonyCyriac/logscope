# Storage Layer

`scope_storage` implements M11 SQLite-backed persistent investigation indexes with hybrid memory spill.

**v1.4.2** adds batched index writes for `--persist-index`:

- WAL mode and reused prepared `INSERT`
- Transaction batching (5,000 lines per commit)
- Progress logging every 10,000 persisted lines via `HybridIndexWriter` (`[INFO] [analysis] Indexed N lines to persistent store`)

Performance regression: `BM_IndexStoreAppend/100000` in [`tests/benchmarks/baseline.json`](../../tests/benchmarks/baseline.json).

See [M11 planning doc](../../docs/planning/M11-STORAGE-LAYER.md) and [ADR-005](../../docs/architecture/decisions/ADR-005-Storage-Architecture.md).
