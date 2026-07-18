# Repository Tests

## Purpose

Repository-level tests validate behavior across module boundaries and through the CLI executable.

## Test Types

| Directory | Target | Description |
|-----------|--------|-------------|
| `integration/` | `logscope_integration_tests` | Core pipeline across Source, Analysis, Investigation, Reporting, and Configuration |
| `end_to_end/` | `logscope_e2e_tests` | CLI executable smoke tests using `samples/` fixtures |

## Running

```bash
ctest --test-dir build --output-on-failure
ctest --test-dir build -R integration
ctest --test-dir build -R CliE2e
```

Integration and end-to-end tests run with the repository root as the working directory so `samples/` paths resolve consistently.
