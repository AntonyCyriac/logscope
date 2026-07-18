# Code Review Checklist

| Field | Value |
|-------|-------|
| Document | Code Review Checklist |
| Category | Handbook |
| Version | 1.0.0 |
| Status | Approved |
| Created | 18-07-2026 |
| Last Updated | 18-07-2026 |

---

# 1. Purpose

This document defines the checklist reviewers shall apply before approving a pull request.

For author responsibilities before requesting review, see [Pull Request Guide](PULL_REQUEST_GUIDE.md).

---

# 2. Design

- [ ] Single Responsibility Principle followed
- [ ] Public API is minimal
- [ ] No unnecessary public functions
- [ ] Strong types used where appropriate
- [ ] No abstraction leaks
- [ ] Consistent with existing Foundation architecture

---

# 3. Correctness

- [ ] Implementation satisfies requirements
- [ ] Edge cases handled
- [ ] Invalid inputs validated
- [ ] Error handling uses `Result<T>`
- [ ] No undefined behavior

---

# 4. Readability

- [ ] Self-documenting code
- [ ] Good naming
- [ ] Small functions
- [ ] Minimal nesting
- [ ] Comments explain "why", not "what"

---

# 5. Performance

- [ ] No unnecessary allocations
- [ ] No unnecessary copies
- [ ] Efficient algorithms selected
- [ ] Complexity is acceptable
- [ ] Premature optimization avoided

---

# 6. Modern C++

- [ ] Rule of Zero followed
- [ ] `explicit` constructors
- [ ] `constexpr` used where applicable
- [ ] `noexcept` added where possible
- [ ] `[[nodiscard]]` used appropriately
- [ ] `const` correctness maintained

---

# 7. Testing

- [ ] Unit tests added
- [ ] Existing tests pass
- [ ] Edge cases covered
- [ ] Negative tests included
- [ ] Test names clearly describe behavior

---

# 8. Documentation

- [ ] Doxygen updated
- [ ] Public API documented
- [ ] Complex logic documented
- [ ] No obsolete comments

---

# 9. Build

- [ ] Builds successfully
- [ ] No compiler warnings
- [ ] No formatting issues
- [ ] CMake updated if required

---

# 10. Related Documents

- [Foundation Guidelines](../standards/FOUNDATION_GUIDELINES.md)
- [C++ Coding Standard](../standards/CPP_CODING_STANDARD.md)
- [Pull Request Guide](PULL_REQUEST_GUIDE.md)

---

# 11. Revision History

| Version | Date | Description |
|----------|------------|-----------------------------|
| 1.0.0 | 18-07-2026 | Extracted from Foundation Guidelines. |
