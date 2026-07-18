# Extension Manager

## Purpose

The Extension module discovers, configures, and manages supported LogScope extensions (C06).

## Components

| Component | Description |
|-----------|-------------|
| `ExtensionDescriptor` | Built-in extension registration metadata |
| `ExtensionInfo` | Public extension metadata and status |
| `ExtensionManager` | Registration, configuration, and lifecycle |

## Configuration

Extension enablement uses configuration keys:

```properties
extensions.analysis.log-levels.enabled=true
extensions.reporting.multi-format.enabled=false
```

## Usage

```cpp
#include "extension.hpp"

scope::extension::ExtensionManager manager = scope::extension::ExtensionManager::createWithBuiltIns();
manager.applyConfiguration(configuration);
manager.initializeEnabled();

for (const auto& info : manager.listExtensions()) {
    // ...
}
```

## CMake Target

- `scope_extension` — static library
- `scope_extension_tests` — unit tests
