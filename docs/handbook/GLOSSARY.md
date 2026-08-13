# Domain glossary

| Field | Value |
|-------|-------|
| Document | Domain Glossary |
| Category | Handbook |
| Version | 1.1.0 |
| Status | Approved |
| Last Updated | 13-08-2026 |

---

Public vocabulary for LogScope as an **Evidence-Centric Investigation Platform**. Use these terms consistently in docs, APIs, UI labels, and tutorials.

---

## Identity

> LogScope helps engineers **organize**, **connect**, and **understand** evidence from production incidents.

Not a log viewer or observability platform — an **investigation platform** centered on **evidence**.

---

## Terms

| Term | Meaning |
|------|---------|
| **Investigation** | The primary domain object representing one production incident |
| **Evidence** | Any artifact or derived observation that contributes to understanding an incident |
| **Artifact** | A concrete piece of evidence stored in an investigation (log, pstack, core, note, report, …) |
| **Timeline Event** | A time-ordered observation **projected** from evidence |
| **Evidence Link** | A relationship between two timeline events — **not** a conclusion (`v2.7.0` Story 5) |
| **Correlation Suggestion** | Ephemeral proposal to link events that share an exact correlation key; accept → Evidence Link (`v2.8.0` Story 6) |
| **Crash Report** | Structured analysis **projected** from a crash artifact (`pstack` / `core`) |
| **Conclusion** | Human judgment based on evidence — not stored as truth by LogScope |

---

## Engineering principles (public)

1. **Evidence is the source of truth.** Timelines, crash reports, evidence links, and AI insights are **projections** — derived from evidence, not alternate sources of truth.

2. **Relationships are evidence, not conclusions.** A link between events records connection; it does not assert causation.

3. **Conclusions belong to engineers.** LogScope presents evidence and relationships; interpretation stays with the investigator.

---

## User-facing language

| Prefer | Avoid (user-facing) |
|--------|---------------------|
| Investigate incident | Analyze log |
| Evidence analysis | Log analysis |
| Related evidence / Suggested connections | Correlation (engineering term) |

---

## Investigation methodology (Phase A + P1)

| Question | Story / item | Shipped |
|----------|--------------|---------|
| Where is my evidence? | Create an Investigation | `v2.3.0` |
| What evidence do I have? | Understand Everything | `v2.4.0` |
| What happened? | See What Happened | `v2.5.0` |
| Why did it crash? | Understand Why It Crashed | `v2.6.0` |
| How is it connected? | Connect the Evidence | `v2.7.0` |
| What else connects? | Discover the Connections | `v2.8.0` |
| Where is the crash on the timeline? | Crash Timeline (P1) | `v2.9.0` |

See [Next Value-Add Backlog](../planning/NEXT-VALUE-ADD.md) and [Roadmap](../ROADMAP.md).

---

## Related

- [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md) — engineering bootstrap
- [PRODUCT.md](../PRODUCT.md) — product summary
- [UI_ARCHITECTURE.md](../architecture/UI_ARCHITECTURE.md) — investigation UI
