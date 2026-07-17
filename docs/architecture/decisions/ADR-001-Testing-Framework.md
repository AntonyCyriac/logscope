# ADR-001: Standard Unit Testing Framework

- **Status:** Accepted
- **Date:** 17-07-2026

---

## Context

The Scope Platform requires a standard unit testing framework that will be used across all modules and products, including LogScope and future applications built on the platform.

The framework should:

- Support modern C++ (C++17 and later)
- Integrate seamlessly with CMake and CTest
- Integrate with GitHub Actions CI
- Support unit tests, fixtures, parameterized tests, and test suites
- Provide optional mocking capabilities
- Be actively maintained with strong community adoption
- Minimize maintenance overhead

Several frameworks were evaluated, including:

- GoogleTest
- Catch2
- doctest
- CxxTest
- Boost.Test

---

## Decision

The Scope Platform adopts **GoogleTest** as the standard unit testing framework.

GoogleMock, which is distributed as part of GoogleTest, may be used whenever mocking is required.

GoogleTest will be integrated using CMake `FetchContent`, ensuring that external dependencies are managed automatically during configuration without storing third-party source code in the repository.

---

## Rationale

GoogleTest was selected because it:

- Is the de facto industry standard for C++ unit testing
- Has excellent integration with CMake, CTest, and GitHub Actions
- Supports advanced testing features such as fixtures, typed tests, and parameterized tests
- Includes GoogleMock for mocking and test doubles
- Is actively maintained and widely adopted in open-source and commercial projects
- Scales well from small libraries to large software platforms

---

## Consequences

### Positive

- Consistent testing approach across the Scope Platform
- Easy integration with CI pipelines
- Improved readability and maintainability of tests
- Simplified onboarding for future contributors
- Strong ecosystem and long-term support

### Negative

- Introduces an external dependency
- Slightly increases CMake configuration time during the initial dependency download

---

## Alternatives Considered

### CxxTest

Rejected because it relies on code generation and has significantly lower community adoption today.

### Catch2

A modern and lightweight framework with excellent usability. Considered a strong alternative but not selected due to GoogleTest's broader ecosystem and integrated mocking support.

### doctest

Very lightweight and fast. Better suited for smaller libraries than a long-term multi-product platform.

### Boost.Test

Feature-rich but introduces an unnecessary dependency on Boost for this project.

---

## Review

This decision may be revisited if future project requirements change significantly. Until then, all new unit tests within the Scope Platform should use GoogleTest.
