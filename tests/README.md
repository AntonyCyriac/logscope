# Repository Tests

## Purpose

Repository-level tests validate behavior across module boundaries and through the CLI executable.

## Test Types

| Directory / Script | Target | Description |
|--------------------|--------|-------------|
| `integration/` | `logscope_integration_tests` | Core pipeline across Source, Analysis, Investigation, Reporting, and Configuration |
| `end_to_end/` | `logscope_e2e_tests` | CLI executable smoke tests using `samples/` fixtures |
| `../scripts/run_cli_matrix.py` | Python runner | Bulk-log CLI combination matrix used in CI and release workflows |

## Running

```bash
ctest --test-dir build --output-on-failure
ctest --test-dir build -R integration
ctest --test-dir build -R CliE2e
```

Integration and end-to-end tests run with the repository root as the working directory so `samples/` paths resolve consistently.

## CLI Matrix (bulk logs)

```bash
python3 scripts/generate_bulk_log.py --lines 100000 --format plain --output /tmp/bulk.log
python3 scripts/generate_bulk_log.py --lines 100000 --format jsonl --output /tmp/bulk.jsonl
python3 scripts/run_cli_matrix.py \
  --logscope build/apps/cli/logscope \
  --plain-log /tmp/bulk.log \
  --jsonl-log /tmp/bulk.jsonl
```

See [Testing Guide](../docs/testing/TESTING.md#cli-matrix) for CI and release integration.
