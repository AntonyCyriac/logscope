/**
 * @file test_failing_storage_plugin.cpp
 * @brief Storage provider plugin that fails append/finalize for error propagation tests.
 */

#include <logscope/plugin/plugin.h>

#include <cstdint>

namespace
{

struct FailingStoreState
{
    uint64_t lineCount{0U};
};

void destroyFailingStore(void* instance)
{
    delete static_cast<FailingStoreState*>(instance);
}

int appendFailingLine(void* instance, uint64_t /*lineNumber*/, const char* /*content*/)
{
    auto* state = static_cast<FailingStoreState*>(instance);
    ++state->lineCount;

    return 1;
}

int finalizeFailingStore(void* instance, uint64_t totalLines)
{
    (void)instance;
    (void)totalLines;

    return 1;
}

uint64_t storedFailingLineCount(void* instance)
{
    return static_cast<FailingStoreState*>(instance)->lineCount;
}

const LogScopeStorageStoreVTable kFailingStoreVTable{destroyFailingStore, appendFailingLine, finalizeFailingStore,
                                                   storedFailingLineCount};

struct FailingBackendState
{
};

void destroyFailingBackend(void* instance)
{
    delete static_cast<FailingBackendState*>(instance);
}

int createFailingStore(void* instance, void** outStore)
{
    (void)instance;

    if (outStore == nullptr)
    {
        return 1;
    }

    auto* store = new LogScopeStorageStore();
    store->instance = new FailingStoreState();
    store->vtable = &kFailingStoreVTable;
    *outStore = store;

    return 0;
}

const LogScopeStorageBackendVTable kFailingBackendVTable{destroyFailingBackend, createFailingStore};

LogScopeStorageBackend* createStorageBackend()
{
    auto* backend = new LogScopeStorageBackend();
    backend->instance = new FailingBackendState();
    backend->vtable = &kFailingBackendVTable;

    return backend;
}

} // namespace

extern "C" int logscope_plugin_register(const LogScopeHostApi* host)
{
    if (host == nullptr)
    {
        return 1;
    }

    const LogScopePluginInfo info{LOGSCOPE_PLUGIN_API_VERSION, "test.failing.storage", "1.0.0",
                                  "Failing storage provider test plugin."};

    if (host->register_extension(host->context, &info) != 0)
    {
        return 1;
    }

    return host->register_storage_backend(host->context, "failing", &createStorageBackend);
}
