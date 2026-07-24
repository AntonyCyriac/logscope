# ADR-003: PDF Report Generation Library

- **Status:** Accepted
- **Date:** 24-07-2026

---

## Context

M8 requires native PDF report output alongside existing text-based formats. The platform needs a solution that:

- Introduces no system-level dependencies (Windows, macOS, Linux CI)
- Uses a license compatible with MIT
- Supports text layout and simple bar charts for level breakdown
- Minimizes maintenance and integration cost

Candidates evaluated:

| Option | Pros | Cons |
|--------|------|------|
| **pdfgen** (single-file C) | Small, MIT, easy embed | C API, limited layout |
| **libharu** (FetchContent) | Mature PDF features | Larger dependency surface |
| **In-house minimal PDF writer** | Zero third-party code, tailored to M8 scope | Limited typography, no Unicode |

---

## Decision

LogScope adopts an **in-house minimal PDF 1.4 writer** (`MinimalPdfWriter` in `core/reporting`) for M8.

The writer generates catalog/pages/content streams with Helvetica, text lines, and filled rectangles for bar charts. HTML reports (M8.4) remain the primary rich presentation format; PDF covers export and archival use cases.

`pdfgen` and `libharu` remain acceptable alternatives if advanced PDF features are required in a future milestone.

---

## Consequences

### Positive

- No FetchContent or vendored third-party PDF code
- Predictable binary output for CI snapshot tests
- Chart geometry shared conceptually with SVG renderer

### Negative

- No embedded Unicode fonts or complex layouts
- PDF stdout is blocked; users must pass `--output <file>`

---

## Compliance

- License: MIT (project code only)
- CI: builds on Ubuntu, Windows, macOS without extra packages
