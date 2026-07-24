# Repository Scripts

Utility scripts supporting CI, release validation, and local verification.

| Script | Purpose |
|--------|---------|
| `generate_bulk_log.py` | Generate synthetic plain-text or JSONL bulk logs for stress testing |
| `run_cli_matrix.py` | Run 20 CLI command scenarios against bulk-log fixtures |
| `check_benchmark_regression.py` | Compare benchmark JSON output against committed baseline |

See [Testing Guide](../docs/testing/TESTING.md#cli-matrix) for usage and CI integration.
