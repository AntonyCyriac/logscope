# Result<T>

## Status

Implemented

**Source:** `core/foundation/result.hpp`, `core/foundation/result.inl`

---

## Purpose

`Result<T>` represents the outcome of an operation that may either succeed
with a value of type `T` or fail with an `Error`.

It provides a simple, type-safe alternative to:

- output parameters
- status codes
- exceptions for expected failures

---

## Goals

- Type-safe
- Exception-free error handling
- Modern C++17 design
- Support move-only types
- Zero overhead for successful operations
- Easy to read
- Easy to test

---

## Dependencies

```
Status
    ↓
Error
    ↓
Result<T>
```

---

## Public Interface

```cpp
template<typename T>
class Result
{
public:

    Result(const T& value);

    Result(T&& value);

    Result(const Error& error);

    Result(Error&& error);

    bool hasValue() const noexcept;

    bool hasError() const noexcept;

    explicit operator bool() const noexcept;

    const T& value() const;

    T& value();

    const Error& error() const noexcept;
};
```

---

## Behaviour

### Success

```
Result<int>(42)
```

- hasValue() → true
- hasError() → false
- value() → 42

---

### Failure

```
Result<int>(
    Error(
        ErrorCode::FileNotFound,
        "sample.log"))
```

- hasValue() → false
- hasError() → true
- error() → Error(...)

---

## Usage

```cpp
auto result = loadConfiguration();

if (!result)
{
    LOG_ERROR(result.error().message());
    return;
}

auto config = result.value();
```

---

## Storage

The implementation shall use

```cpp
std::variant<T, Error>
```

for internal storage.

No manual unions or raw memory management.

---

## Error Handling

`value()` shall only be called when a value exists.

Calling `value()` on a failed result is considered a programming error and
shall throw `std::logic_error`.

---

## Design Decisions

- Value stored by value
- Error stored by value
- Supports copy semantics
- Supports move semantics
- Supports move-only types
- No heap allocation
- No exceptions for expected failures

---

## Future Enhancements

Potential future additions include:

- map()
- and_then()
- transform()
- or_else()
- value_or()
- emplace()

These are intentionally excluded from the initial implementation to keep the
API small and focused.
