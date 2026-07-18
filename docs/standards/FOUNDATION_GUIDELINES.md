# Foundation Guidelines

## Purpose

This document defines the engineering standards and coding conventions for the Scope Platform Foundation library. These guidelines ensure consistency, maintainability, readability, and long-term scalability across all Foundation components.

## Related Standards

- [Engineering Principles](ENGINEERING_PRINCIPLES.md) — project values (read first)
- [C++ Coding Standard](CPP_CODING_STANDARD.md) — repository-wide rules not repeated here
- [API Design Guidelines](API_DESIGN_GUIDELINES.md) — public API contracts
- [Result&lt;T&gt; Design](../architecture/foundation/RESULT.md) — Foundation error handling model

## Handbook

- [Git Conventions](../handbook/GIT_CONVENTIONS.md) — commit and branch naming
- [Pull Request Guide](../handbook/PULL_REQUEST_GUIDE.md) — author checklist and definition of done
- [Code Review Checklist](../handbook/CODE_REVIEW_CHECKLIST.md) — reviewer checklist

---

# 1. Rule of Zero

Prefer compiler-generated special member functions whenever possible.

```cpp
class Uuid
{
public:
    Uuid() noexcept = default;
    ~Uuid() = default;

    Uuid(const Uuid&) = default;
    Uuid(Uuid&&) noexcept = default;

    Uuid& operator=(const Uuid&) = default;
    Uuid& operator=(Uuid&&) noexcept = default;
};
```

Do not implement constructors, destructors, or assignment operators unless custom behavior is required.

---

# 2. Explicit Constructors

Every single-argument constructor must be declared `explicit`.

```cpp
explicit Version(int major);
```

This prevents unintended implicit conversions.

---

# 3. Use `noexcept` Whenever Possible

Functions that are guaranteed not to throw exceptions shall be marked `noexcept`.

```cpp
bool isNil() const noexcept;

bool operator==(const Uuid&) const noexcept;
```

Benefits:

- Documents intent
- Enables compiler optimizations
- Improves exception safety

---

# 4. Use `[[nodiscard]]`

Functions whose return values should not be ignored must use `[[nodiscard]]`.

```cpp
[[nodiscard]]
bool isNil() const noexcept;

[[nodiscard]]
Result<Uuid> parse(std::string_view value);
```

This reduces accidental misuse of APIs.

---

# 5. Prefer `std::string_view`

Use `std::string_view` for read-only string parameters.

Instead of:

```cpp
Result<Uuid> parse(const std::string& value);
```

Prefer:

```cpp
Result<Uuid> parse(std::string_view value);
```

Advantages:

- No unnecessary allocations
- Accepts string literals
- Accepts std::string
- Accepts substrings

---

# 6. Minimize Header Dependencies

Headers should include only the files they directly require.

Example:

```cpp
#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include "result.hpp"
```

Avoid unnecessary includes in public headers.

---

# 7. Keep Helper Functions Local

Implementation-only helper functions belong inside anonymous namespaces within `.cpp` files.

```cpp
namespace
{

constexpr char HEX[] = "0123456789abcdef";

bool isHex(char c) noexcept;

}
```

Do not expose implementation details outside the translation unit.

---

# 8. Avoid Dynamic Allocation

Foundation classes should avoid heap allocations unless there is a compelling architectural reason.

Examples:

- Version stores integers directly.
- Uuid stores sixteen bytes directly.

Keep Foundation lightweight and predictable.

---

# 9. Unit Tests Mirror the Public API

Every public function should have corresponding unit tests.

Example:

```
Uuid_DefaultConstruction
Uuid_IsNil
Uuid_ToString
Uuid_Equality
Uuid_Ordering
```

Test names should clearly indicate the behavior being verified.

---

# 10. Keep Pull Requests Small

*Implements [EP-011 – Incremental Evolution](ENGINEERING_PRINCIPLES.md#ep-011--incremental-evolution).*

Each pull request should implement a single logical feature.

Example roadmap:

```
PR-005A
✓ UUID Core

PR-005B
✓ UUID Parsing

PR-005C
✓ UUID Generation
```

Smaller pull requests are easier to review, test, and maintain.

---

# Future Additions

This document will evolve as the project grows and may later include:

- Naming conventions
- Header organization
- Namespace conventions
- Error handling strategy (`Result<T>`)
- Logging guidelines
- Testing philosophy
- Documentation standards
- Formatting rules
- API design principles
- Performance considerations
- Thread safety guidelines
- CMake conventions

---

# 11. Prefer Value Semantics

Foundation types should behave like built-in types.

Value objects should:

- Own their state
- Be copyable
- Be movable
- Have deterministic behavior

Example:

```cpp
Uuid id1 = Uuid::generate();
Uuid id2 = id1;

EXPECT_EQ(id1, id2);
```

Avoid shared ownership unless the abstraction explicitly requires it.

---

# 12. Hide Internal Representation

Expose behavior, not implementation.

Avoid APIs like:

```cpp
const std::array<uint8_t,16>& bytes() const;
```

Instead provide meaningful operations:

```cpp
bool isNil() const noexcept;

std::string toString() const;
```

Internal representation may change without affecting users.

---

# 13. Keep APIs Small

Every public function becomes part of the library contract.

Before adding a public API ask:

- Is it necessary?
- Will every user need it?
- Can it be added later?

Smaller APIs are easier to maintain.

---

# 14. Const Correctness

Mark every member function `const` unless it modifies object state.

```cpp
bool isNil() const noexcept;

std::string toString() const;
```

Const correctness improves readability and enables compiler diagnostics.

---

# 15. Prefer Compile-Time Constants

Use `constexpr` whenever possible.

```cpp
constexpr std::size_t UUID_SIZE = 16;

constexpr char HEX[] = "0123456789abcdef";
```

Compile-time constants improve performance and eliminate magic numbers.

---

# 16. Avoid Magic Numbers

Never scatter unexplained literals throughout the implementation.

Instead of:

```cpp
bytes[6] |= 0x40;
```

Prefer:

```cpp
constexpr std::uint8_t VERSION_4_MASK = 0x40;

bytes[6] |= VERSION_4_MASK;
```

Names document intent.

---

# 17. Single Responsibility

*Implements [EP-005 – Single Responsibility](ENGINEERING_PRINCIPLES.md#ep-005--single-responsibility).*

Each class should have one well-defined responsibility.

Examples:

```
Uuid
    Stores UUID values

Version
    Stores semantic versions

Status
    Represents success or failure

Time
    Represents timestamps
```

Parsing, formatting, serialization, and algorithms should remain separate unless they are intrinsic to the type.

---

# 18. Separate Interface and Implementation

Headers describe **what** a component does.

Source files describe **how** it works.

Headers should contain:

- declarations
- documentation
- type definitions

Implementation belongs in `.cpp`.

---

# 19. Prefer Standard Library

Use the C++ Standard Library before introducing third-party dependencies.

Preferred examples:

- std::array
- std::vector
- std::optional
- std::variant
- std::chrono
- std::filesystem
- std::span (C++20)

Avoid reinventing existing abstractions.

---

# 20. No Exceptions in Foundation

*See also [Result&lt;T&gt; Design](../architecture/foundation/RESULT.md).*

Foundation follows explicit error handling using `Result<T>`.

Example:

```cpp
Result<Uuid> uuid = Uuid::parse(text);

if (!uuid)
{
    return uuid.error();
}
```

Avoid throwing exceptions for expected runtime failures.

---

# 21. Self-Documenting Code

Prefer expressive names over comments.

Instead of:

```cpp
x = x + 1;
```

Use:

```cpp
++retryCount;
```

Instead of:

```cpp
auto v = parse(s);
```

Use:

```cpp
auto parsedUuid = parse(uuidString);
```

Good names reduce the need for comments.

---

# 22. Comment the Why, Not the What

Avoid comments that simply repeat the code.

Bad:

```cpp
// Increment i
++i;
```

Good:

```cpp
// Skip the version nibble according to RFC 4122.
```

Comments should explain intent, assumptions, or non-obvious decisions.

---

# 23. Prefer Early Returns

Reduce nesting by returning as soon as an error is detected.

Instead of:

```cpp
if (valid)
{
    ...
}
else
{
    return error;
}
```

Prefer:

```cpp
if (!valid)
{
    return error;
}

...
```

This keeps control flow simple and readable.

---

# 24. Keep Functions Small

A function should perform one logical task.

As a guideline:

- 10–30 lines is ideal.
- Split large functions into private helpers.
- Avoid deeply nested logic.

Small functions are easier to test and review.

---

# 25. Optimize for Readability First

Correctness and maintainability take priority over micro-optimizations.

Only optimize after measuring.

Prefer:

```cpp
std::copy(...)
```

over obscure manual optimizations unless profiling proves a measurable benefit.

Readable code is easier to maintain and often performs well enough.
---

# 26. Prefer Strong Types Over Primitive Types

Avoid using primitive types when a dedicated type better expresses intent.

Instead of:

```cpp
std::string id;
```

Prefer:

```cpp
Uuid id;
```

Instead of:

```cpp
int version;
```

Prefer:

```cpp
Version version;
```

Strong types improve readability, reduce bugs, and make APIs self-documenting.

---

# 27. RAII Always

Every resource must be owned by an object whose lifetime manages it automatically.

Examples:

- Files
- Sockets
- Mutexes
- Memory
- Database Connections

Never manually pair:

```cpp
open();
close();
```

Prefer:

```cpp
File file(path);
```

The destructor performs cleanup automatically.

---

# 28. Never Own Raw Pointers

Ownership must be explicit.

Allowed:

```cpp
std::unique_ptr<T>
std::shared_ptr<T>
std::weak_ptr<T>
```

Raw pointers should only represent:

- optional references
- non-owning observers

Never write:

```cpp
Foo* foo = new Foo();
```

---

# 29. Prefer References

If an object cannot be null, use references.

Instead of:

```cpp
void process(Uuid* uuid);
```

Prefer:

```cpp
void process(const Uuid& uuid);
```

Pointers should indicate optionality.

---

# 30. Minimize Mutability

Variables should be immutable unless mutation is required.

Prefer:

```cpp
const Version version{1,0,0};
```

instead of

```cpp
Version version{1,0,0};
```

Immutability simplifies reasoning and improves thread safety.

---

# 31. Initialize Everything

Never leave variables uninitialized.

Bad:

```cpp
int count;
```

Good:

```cpp
int count{0};
```

Good:

```cpp
std::array<uint8_t,16> bytes{};
```

Initialization prevents undefined behavior.

---

# 32. Avoid Boolean Parameters

Boolean parameters reduce readability.

Instead of:

```cpp
write(file, true);
```

Prefer:

```cpp
write(file, WriteMode::Append);
```

Enums express intent clearly.

---

# 33. Keep Headers Lightweight

Headers are included everywhere.

Avoid:

- unnecessary includes
- implementation details
- heavy templates

Prefer forward declarations where appropriate.

```cpp
class Logger;
```

instead of

```cpp
#include "logger.hpp"
```

when only a declaration is required.

---

# 34. One Public Responsibility Per Header

Each header should expose a single primary abstraction.

Good:

```
uuid.hpp
version.hpp
status.hpp
time.hpp
```

Avoid:

```
utility.hpp
common.hpp
misc.hpp
helpers.hpp
```

These tend to become dumping grounds.

---

# 35. Namespaces Must Reflect Architecture

Namespaces should represent logical modules.

Example:

```cpp
namespace scope::foundation
{

}
```

Avoid nested namespaces without architectural meaning.

---

# 36. Avoid Macros

Use language features instead of macros whenever possible.

Instead of:

```cpp
#define UUID_SIZE 16
```

Prefer:

```cpp
constexpr std::size_t UUID_SIZE = 16;
```

Macros should be limited to:

- include guards (if not using `#pragma once`)
- platform/compiler detection
- controlled build configuration

---

# 37. No Global Mutable State

Avoid global variables.

Bad:

```cpp
Logger logger;
```

Good:

```cpp
Logger& logger = Logger::instance();
```

or inject dependencies explicitly.

Global mutable state complicates testing and concurrency.

---

# 38. Dependency Direction

*See also [Architecture Principles](../architecture/ARCHITECTURE_PRINCIPLES.md).*

Lower-level libraries must never depend on higher-level modules.

Foundation

↓

Core

↓

Runtime

↓

Application

Never allow reverse dependencies.

---

# 39. Public APIs Must Be Stable

Changing a public API has long-term maintenance costs.

Before changing an API ask:

- Is it necessary?
- Will it break existing users?
- Can it be introduced without breaking compatibility?

Stability is a design goal.

---

# 40. Every Class Needs Tests

Every public class should have corresponding unit tests.

Minimum expectations:

```
Construction

Copy

Move

Equality

Invalid Inputs

Edge Cases

Regression Tests
```

Testing is part of implementation, not an optional activity.

---

# Workflow

Day-to-day contribution workflow is documented in the handbook:

- [Git Conventions](../handbook/GIT_CONVENTIONS.md) — commit and branch naming
- [Pull Request Guide](../handbook/PULL_REQUEST_GUIDE.md) — author checklist and definition of done
- [Code Review Checklist](../handbook/CODE_REVIEW_CHECKLIST.md) — reviewer checklist

For project values, see [Engineering Principles](ENGINEERING_PRINCIPLES.md).

---

# Engineering Motto

> **"Write code that your future self—and your teammates—will thank you for."**
