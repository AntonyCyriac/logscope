# LogScope API Reference

LogScope is a C++17 log analysis platform. This site is generated from public headers under `core/` and `apps/`.

## Modules

| Library | Responsibility |
|---------|----------------|
| `scope_foundation` | Errors, results, paths, timestamps, strings |
| `scope_runtime` | Configuration, diagnostics |
| `scope_configuration` | Properties file loading and validation |
| `scope_source` | Log source readers and format detection |
| `scope_analysis` | Analysis pipeline and line indexing |
| `scope_search` | Text and boolean search |
| `scope_query` | Filter DSL evaluation |
| `scope_storage` | SQLite-backed persistent indexes |
| `scope_analytics` | Frequency, clustering, timeline analysis |
| `scope_investigation` | Investigation engine |
| `scope_reporting` | Report formatting and sections |
| `scope_extension` | Extension registration |
| `scope_workspace` | Sessions and workspace paths |

## Related documentation

- [Configuration Guide](../handbook/CONFIGURATION_GUIDE.md)
- [CLI Reference](../handbook/CLI_REFERENCE.md)
- [Architecture Overview](../architecture/ARCHITECTURE_OVERVIEW.md)
- [Component Catalog](../architecture/COMPONENT_CATALOG.md)

## Building locally

```bash
cmake -S . -B build -DLOGSCOPE_DOCS=ON
cmake --build build --target docs
```

Open `build/docs/api/html/index.html` in a browser.
