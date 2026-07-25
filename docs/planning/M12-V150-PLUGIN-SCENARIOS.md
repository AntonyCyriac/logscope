# M12 v1.5.0 — Plugin Scenarios

| Field | Value |
|-------|-------|
| Document | M12 v1.5.0 Plugin Scenarios |
| Category | Project Planning |
| Version | 1.0.0 |
| Status | Complete |
| Created | 25-07-2026 |
| Last Updated | 25-07-2026 |

---

# 1. Purpose

Acceptance scenarios for **v1.5.0** — M12 Dynamic Plugins. Every row must pass before release.

**Target release:** `v1.5.0`

**Features in scope:**

| ID | Feature |
|----|---------|
| P1 | Plugin loader (`.so`/`.dll`) |
| P2 | Report provider plugins |
| P3 | Parser provider plugins |
| P4 | Search provider plugins |
| P5 | Storage backend plugins |
| P6 | CLI discovery (`extensions list`/`describe`) |
| P7 | Failure isolation (FR-004.5) |
| P8 | Cross-platform sample plugin CI |

See [M12-DYNAMIC-PLUGINS.md](M12-DYNAMIC-PLUGINS.md), [ADR-006](../architecture/decisions/ADR-006-Plugin-Loading.md).

---

# 2. Legend

| Column | Meaning |
|--------|---------|
| **Regression** | Dedicated regression or e2e guard |
| **Test layer** | U = unit, I = integration, E = e2e |

Status: ⬜ planned · 🟡 in progress · ✅ complete

---

# 3. Loader (P1)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| P1.1 | Valid plugin | `plugins.enabled=true`, valid `.so` in path | Loads; extension registered | U+I | Yes | ✅ |
| P1.2 | Missing file | Path to non-existent library | Error logged; core continues | U | | ✅ |
| P1.3 | Missing symbol | Library without `logscope_plugin_register` | Skip plugin; error logged | U | | ✅ |
| P1.4 | API mismatch | Plugin API version > host | Reject plugin; fail closed | U | Yes | ✅ |
| P1.5 | Plugins disabled | `plugins.enabled=false` (default) | No scan; built-ins only | U+E | Yes | ✅ |
| P1.6 | Env path | `LOGSCOPE_PLUGIN_PATH` set | Discovers plugins in env paths | U | | ✅ |

---

# 4. Report providers (P2)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| P2.1 | Register contributor | Sample report plugin loaded | Section appears in registry | U | | ✅ |
| P2.2 | HTML report | Generate report with plugin enabled | Plugin section in HTML output | I | Yes | ✅ |
| P2.3 | Built-in unchanged | No plugins loaded | Default sections unchanged | I | Yes | ✅ |

---

# 5. Parser providers (P3)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| P3.1 | Register parser | Sample parser plugin | Parser in `ParserRegistry` | U | | ✅ |
| P3.2 | Format match | Log matching plugin format id | Plugin parser used | I | Yes | ✅ |
| P3.3 | Profile guardrail | Default profile without plugin format | Built-in detection unchanged | I | | ✅ |

---

# 6. Search providers (P4)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| P4.1 | Register provider | Sample search plugin | Provider in registry | U | | ✅ |
| P4.2 | Named search | Config selects plugin provider | Plugin search invoked | I | Yes | ✅ |
| P4.3 | Default search | No plugin provider | Built-in `SearchEngine` used | I | Yes | ✅ |

---

# 7. Storage backends (P5)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| P5.1 | Register backend | Sample storage plugin | Factory in registry | U | | ✅ |
| P5.2 | Plugin backend | `storage.backend=plugin:<id>` | Plugin `IndexStore` created | I | Yes | ✅ |
| P5.3 | SQLite default | `storage.backend=sqlite` or unset | `SqliteIndexStore` unchanged | I+E | Yes | ✅ |
| P5.4 | FTS regression | SQLite + persist + FTS query | Existing M11 FTS path works | I | Yes | ✅ |

---

# 8. CLI (P6)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| P6.1 | List dynamic | Plugin loaded | `extensions list` shows plugin id | E | Yes | ✅ |
| P6.2 | Describe dynamic | `extensions describe <id>` | Version, API version, path shown | E | | ✅ |
| P6.3 | Describe built-in | Built-in id | Unchanged metadata | E | | ✅ |

---

# 9. Isolation (P7)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| P7.1 | Init failure | Plugin register returns error | Extension `failed`; built-ins ready | U | Yes | ✅ |
| P7.2 | Mixed load | One bad + one good plugin | Good plugin works | I | Yes | ✅ |

---

# 10. Cross-platform (P8)

| ID | Scenario | Trigger | Expected | Test | Reg | Status |
|----|----------|---------|----------|------|-----|--------|
| P8.1 | Build samples | CI matrix Ubuntu/Win/macOS | Sample plugins compile | CI | Yes | ✅ |
| P8.2 | Load sample | Integration test per OS | Sample plugin loads | I | | ✅ |

---

# 11. Release gate

All P1–P8 rows marked ✅ before tagging `v1.5.0`.
