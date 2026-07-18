# Foundation

## Purpose

The Foundation module provides universal technical capabilities shared by all components of the Scope platform.

Foundation contains no product-specific knowledge and serves as the lowest engineering layer in the architecture.

---

## Responsibilities

- Status codes
- Error handling
- Result types
- Common utilities
- Shared data types

---

## Implemented Components

| Component | Location |
|-----------|----------|
| `Status` | `status.hpp` |
| `Error` / `ErrorCode` | `error.hpp`, `error.cpp` |
| `Result<T>` | `result.hpp`, `result.inl` |
| `Version` | `version.hpp`, `version.cpp` |
| `Uuid` | `uuid.hpp`, `uuid.cpp` |
| `Time` | `time.hpp`, `time.cpp`, `time.inl` |
| `Date` | `date.hpp`, `date.cpp`, `date.inl` |
| `DateTime` | `datetime.hpp`, `datetime.cpp`, `datetime.inl` |
| `Duration` | `duration.hpp`, `duration.cpp`, `duration.inl` |
| `Timestamp` | `timestamp.hpp`, `timestamp.cpp`, `timestamp.inl` |
| `Clock` | `clock.hpp`, `clock.cpp` |
| `Stopwatch` | `stopwatch.hpp`, `stopwatch.cpp` |
| `Random` | `random.hpp`, `random.cpp` |
| `String` utilities | `string.hpp`, `string.cpp` |
| `Path` | `path.hpp`, `path.cpp` |
| `FileSystem` | `filesystem.hpp`, `filesystem.cpp` |

All components are exported through `foundation.hpp` and tested under `tests/`.

---

## Dependencies

None.

---

## Dependents

All other Core modules, Products and Applications.

---

## Design Principles

- Product independent
- Lightweight
- Reusable
- Stable
- Minimal dependencies
