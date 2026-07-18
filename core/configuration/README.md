# Configuration Manager

## Purpose

The Configuration module loads, validates, and provides configuration to LogScope and other Scope products.

It depends on Foundation (paths, filesystem) and Runtime (`Configuration` key-value store).

## Components

| Component | Description |
|-----------|-------------|
| `ConfigurationManager` | Load properties files, apply environment overrides, validate required keys |

## Configuration File Format

Properties-style key=value files:

```properties
# Comments start with #
log.level=info
log.timestamps=true
app.name=logscope
```

## Environment Overrides

Environment variables prefixed with `SCOPE_` override file values. Underscores in the variable name map to dots in the config key:

| Environment variable | Config key |
|---------------------|------------|
| `SCOPE_LOG_LEVEL` | `log.level` |
| `SCOPE_LOG_TIMESTAMPS` | `log.timestamps` |

## Usage

```cpp
#include "configuration.hpp"

auto result = scope::configuration::ConfigurationManager::loadFromFile(path);

if (result) {
    result->applyEnvironment();

    if (result->validate({"log.level"})) {
        scope::runtime::Diagnostics::instance().applyConfiguration(result->configuration());
    }
}
```

## CMake Target

- `scope_configuration` — static library
- `scope_configuration_tests` — unit tests
