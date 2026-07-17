# API Design Guidelines

## Purpose

This document defines the principles and conventions for designing public APIs throughout the Scope platform.

The objective is to create APIs that are:

- Simple
- Consistent
- Predictable
- Easy to use
- Easy to extend
- Backward compatible where practical

These guidelines apply to all public interfaces exposed by Core modules, Products, and Applications.

---

# Design Principles

APIs should be:

- Clear over clever
- Explicit over implicit
- Stable over convenient
- Small over feature-rich
- Composable over monolithic

---

# Public vs Private APIs

Public APIs define the contract between modules.

Private implementation details should never leak into public interfaces.

Expose only what consumers need.

Hide implementation whenever possible.

---

# Naming

API names should describe intent rather than implementation.

Good examples:

```cpp
analyze()
loadWorkspace()
generateReport()
```

Avoid names tied to implementation details.

```cpp
processDataInternal()
executeStep2()
```

---

# Ownership

Ownership must always be explicit.

Avoid ambiguous ownership transfer.

Prefer RAII and smart pointers when ownership is shared.

Avoid owning raw pointers.

---

# Error Handling

Expected runtime failures should use Foundation types such as:

- `Status`
- `Result<T>` (future)

Avoid exceptions for expected operational failures.

---

# Input Validation

Public APIs should validate their inputs.

Invalid parameters should produce predictable behavior.

Never leave objects in an inconsistent state.

---

# Dependencies

Keep dependencies minimal.

Public interfaces should expose as few implementation details as possible.

Prefer forward declarations where practical.

---

# Interface Size

Classes should have a focused responsibility.

Large interfaces should be decomposed into smaller abstractions.

Avoid "god classes."

---

# Object Lifetime

Object lifetime should be obvious from the API.

Constructors establish valid objects.

Destructors release owned resources.

Avoid partially initialized objects.

---

# Const Correctness

Mark methods as `const` whenever they do not modify observable state.

Prefer immutable interfaces where practical.

---

# Return Types

Choose return types that clearly express intent.

Examples:

```cpp
Status analyze();

bool isLoaded() const;

std::string name() const;
```

Avoid overloaded meanings for primitive return values.

---

# Parameters

Prefer passing large objects by `const&`.

Pass small trivially copyable types by value.

Avoid output parameters when a return value is clearer.

---

# Extensibility

Design interfaces for extension without requiring modification.

Avoid exposing implementation-specific assumptions.

---

# Thread Safety

Document thread-safety expectations.

Do not assume callers understand concurrency guarantees.

---

# Documentation

Every public API should document:

- Purpose
- Parameters
- Return value
- Error conditions
- Thread-safety (if applicable)

---

# Versioning

Public APIs should evolve carefully.

Avoid breaking changes whenever practical.

Deprecate APIs before removing them.

---

# Testing

Every public API should have corresponding unit tests.

Tests should verify:

- Normal behavior
- Boundary conditions
- Invalid inputs
- Error handling

---

# Philosophy

Good APIs make the correct usage obvious.

Simple APIs are easier to maintain than clever APIs.

Design interfaces for developers who will read the code years later.
