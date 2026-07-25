/**
 * @file test_storage_plugin.cpp
 * @brief Storage provider test plugin (M12).
 */

#include <logscope/plugin/plugin.h>

#include <cstdint>

namespace
{

struct MemoryStoreState
{
    uint64_t lineCount{0U};
    uint64_t totalLines{0U};
};

void destroyMemoryStore(void* instance)
{
    delete static_cast<MemoryStoreState*>(instance);
}

int appendMemoryLine(void* instance, uint64_t /*lineNumber*/, const char* /*content*/)
{
    auto* state = static_cast<MemoryStoreState*>(instance);
    ++state->lineCount;

    return 0;
}

int finalizeMemoryStore(void* instance, uint64_t totalLines)
{
    auto* state = static_cast<MemoryStoreState*>(instance);
    state->totalLines = totalLines;

    return 0;
}

uint64_t storedMemoryLineCount(void* instance)
{
    return static_cast<MemoryStoreState*>(instance)->lineCount;
}

const LogScopeStorageStoreVTable kMemoryStoreVTable{destroyMemoryStore, appendMemoryLine, finalizeMemoryStore,
                                                    storedMemoryLineCount};

struct MemoryBackendState
{
};

void destroyMemoryBackend(void* instance)
{
    delete static_cast<MemoryBackendState*>(instance);
}

int createMemoryStore(void* instance, void** outStore)
{
    (void)instance;

    if (outStore == nullptr)
    {
        return 1;
    }

    auto* store = new LogScopeStorageStore();
    store->instance = new MemoryStoreState();
    store->vtable = &kMemoryStoreVTable;
    *outStore = store;

    return 0;
}

const LogScopeStorageBackendVTable kMemoryBackendVTable{destroyMemoryBackend, createMemoryStore};

LogScopeStorageBackend* createStorageBackend()
{
    auto* backend = new LogScopeStorageBackend();
    backend->instance = new MemoryBackendState();
    backend->vtable = &kMemoryBackendVTable;

    return backend;
}

} // namespace

extern "C" int logscope_plugin_register(const LogScopeHostApi* host)
{
    if (host == nullptr)
    {
        return 1;
    }

    const LogScopePluginInfo info{LOGSCOPE_PLUGIN_API_VERSION, "test.storage", "1.0.0",
                                  "Storage provider test plugin."};

    if (host->register_extension(host->context, &info) != 0)
    {
        return 1;
    }

    return host->register_storage_backend(host->context, "memory", &createStorageBackend);
}
