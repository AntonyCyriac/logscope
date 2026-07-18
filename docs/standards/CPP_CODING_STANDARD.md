# C++ Coding Standard

## Purpose

This document defines the C++ coding conventions used throughout the LogScope project.

The objective is to produce code that is:

- Readable
- Consistent
- Maintainable
- Easy to review
- Easy to extend

These conventions apply to all C++ source files in the repository.

---

# Scope

This standard applies to:

- Core modules
- Product modules
- Applications
- Tests
- Tools
- Examples

Third-party code is excluded from this standard.

---

# Language Standard

The project uses **C++17**.

Compiler-specific language extensions are not permitted.

The language standard is defined centrally by the root `CMakeLists.txt`.

---

# Repository Organization

The repository is organized into major engineering areas.

```
Repository
│
├── apps/
├── core/
├── products/
├── extensions/
├── tests/
├── docs/
```

Each major engineering area owns its internal modules.

Each module owns its own implementation.

The repository root delegates build management to major engineering areas.

---

# Module Organization

Each module should remain self-contained.

Typical module structure:

```
module/
│
├── README.md
├── CMakeLists.txt
├── *.hpp
├── *.cpp
└── tests/
```

Avoid unnecessary directory nesting.

Additional directories should be introduced only when justified by module complexity.

---

# Namespaces

Namespaces shall follow the project hierarchy.

Example:

```cpp
namespace scope::foundation
{

}
```

Always close namespaces with comments.

```cpp
} // namespace scope::foundation
```

Namespace names shall use lowercase.

Examples:

```cpp
scope::foundation
scope::analysis
scope::reporting
scope::workspace
```

Do not use namespace aliases or `using namespace` directives in public header files.

---

# Header Files

Every public header shall begin with a documentation block.

Example:

```cpp
/**
 * @file status.hpp
 * @brief Defines the Status type used throughout the Scope platform.
 */
```

Use

```cpp
#pragma once
```

instead of include guards.

Headers should compile independently.

Headers should include only the dependencies they require.

Prefer forward declarations where appropriate.

---

# Source File Layout

Source files should follow this structure.

1. File documentation
2. Includes
3. Anonymous namespace (if required)
4. Namespace
5. Implementation

Example:

```cpp
/**
 * @file status.cpp
 */

#include "status.hpp"

namespace
{

}

namespace scope::foundation
{

}
```

---

# Include Order

Include files in the following order.

1. Corresponding header
2. C standard library headers (if required)
3. C++ standard library headers
4. Third-party library headers
5. Project headers

Separate each group with a blank line.

Example:

```cpp
#include "status.hpp"

#include <cassert>
#include <string>

#include <fmt/core.h>

#include "foundation/error.hpp"
```

---

# Naming Convention

## Classes

Use PascalCase.

```cpp
LogAnalyzer
Workspace
ReportGenerator
```

---

## Structs

Use PascalCase.

```cpp
LogEntry
SearchResult
```

---

## Enums

Use `enum class`.

```cpp
enum class Status
{
    Success,
   Failure
};
```

---

## Functions

Use camelCase.

```cpp
loadWorkspace()
generateReport()
```

---

## Variables

Use camelCase.

```cpp
logCount
currentLine
```

---

## Constants

Use the `k` prefix.

```cpp
constexpr int kMaxWorkers = 8;
```

---

## Files

Use lowercase file names.

Examples:

```
status.hpp
status.cpp
log_analyzer.hpp
log_analyzer.cpp
```

---

# Class Layout

Use the following order.

```cpp
class Example
{
public:

protected:

private:
};
```

Constructors appear first.

Public interface before implementation details.

---

# Formatting

The repository `.clang-format` defines formatting rules.

Manual formatting should be avoided.

Do not optimize code layout for personal preference.

---

# Comments

Comments should explain **why**, not **what**.

Good:

```cpp
// Parsing stops here because the remaining input
// belongs to the next log record.
```

Avoid:

```cpp
// Increment i
i++;
```

Public APIs should use Doxygen comments.

---

# Error Handling

Prefer explicit return types over exceptions for expected runtime failures.

- Foundation code uses `Result<T>` — see [Foundation Guidelines](FOUNDATION_GUIDELINES.md) and [Result&lt;T&gt; Design](../architecture/foundation/RESULT.md).
- Public APIs should document failure modes — see [API Design Guidelines](API_DESIGN_GUIDELINES.md).

---

# Dependencies

Minimize dependencies.

Include only what is required.

Prefer forward declarations when possible.

Avoid unnecessary transitive includes.

---

# Modern C++

Follow modern C++17 practices throughout the repository.

Use `constexpr`, `noexcept`, `enum class`, `override`, `final`, `nullptr`, `using` instead of `typedef`, `auto` when it improves readability, and range-based `for` loops where appropriate.

Avoid C-style casts, owning raw pointers, mutable global state, macros for constants, and `using namespace` in header files.

For detailed Foundation patterns (Rule of Zero, `[[nodiscard]]`, `string_view`, and more), see [Foundation Guidelines](FOUNDATION_GUIDELINES.md).

---

# Testing

Every reusable module shall contain its own tests. Tests should remain independent from production code and verify one behavior at a time.

For Foundation test naming, coverage expectations, and examples, see [Foundation Guidelines](FOUNDATION_GUIDELINES.md).

---

# Build System

Each module owns its own `CMakeLists.txt`.

Major engineering areas own their child modules.

The repository root delegates build management to major engineering areas.

Prefer target-based CMake over global configuration.

---

# Simplicity

Prefer the simplest correct solution. Additional abstraction should be introduced only after repeated use demonstrates a clear benefit.

Avoid premature optimization and speculative generalization. See [EP-010 – Simplicity Over Cleverness](ENGINEERING_PRINCIPLES.md#ep-010--simplicity-over-cleverness).

---

# Engineering Philosophy

See [Engineering Principles](ENGINEERING_PRINCIPLES.md) for the project decision framework.

---

# Related Standards

- [DOCUMENT_STANDARD.md](DOCUMENT_STANDARD.md)
- [ENGINEERING_PRINCIPLES.md](ENGINEERING_PRINCIPLES.md)
- [API_DESIGN_GUIDELINES.md](API_DESIGN_GUIDELINES.md)
- [FOUNDATION_GUIDELINES.md](FOUNDATION_GUIDELINES.md)

## Handbook

- [Git Conventions](../handbook/GIT_CONVENTIONS.md)
- [Pull Request Guide](../handbook/PULL_REQUEST_GUIDE.md)
- [Code Review Checklist](../handbook/CODE_REVIEW_CHECKLIST.md)

Future standards may include:

- TESTING_STANDARD.md
