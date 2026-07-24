# Regression Tests

Guards for **fixed bugs** that must not reappear. Unlike broad integration tests, each case here maps to a known failure mode (often discovered during milestone delivery or release validation).

## When to add a test

- A bug was fixed in production code and could silently return without a targeted test.
- The failure involved cross-module interaction (analysis + storage, stdin + index reuse, etc.).
- Unit tests in the owning module do not fully capture the user-visible regression.

## Running

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure -R regression
```

Regression tests are part of the default `ctest` suite (no separate CMake option).

## Conventions

- File name: `*_regression_test.cpp`
- Test names: describe the bug being prevented (e.g. `AnalyzeStdinWithReuseIndexDoesNotFail`).
- Add a one-line comment citing the milestone or symptom when helpful.

See [Developer Guide](../../docs/handbook/DEVELOPER_GUIDE.md#7-testing) and [Testing Guide](../../docs/testing/TESTING.md).
