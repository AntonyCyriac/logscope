/**
 * @file plugin.h
 * @brief LogScope plugin C ABI (M12).
 */

#ifndef LOGSCOPE_PLUGIN_H
#define LOGSCOPE_PLUGIN_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOGSCOPE_PLUGIN_API_VERSION 1U
#define LOGSCOPE_HOST_API_VERSION 1U

#if defined(_WIN32)
#    if defined(LOGSCOPE_PLUGIN_EXPORT)
#        define LOGSCOPE_PLUGIN_API __declspec(dllexport)
#    else
#        define LOGSCOPE_PLUGIN_API __declspec(dllimport)
#    endif
#else
#    define LOGSCOPE_PLUGIN_API __attribute__((visibility("default")))
#endif

typedef struct LogScopePluginInfo
{
    uint32_t api_version;
    const char* id;
    const char* version;
    const char* description;
} LogScopePluginInfo;

typedef enum LogScopeParseOutcome
{
    LOGSCOPE_PARSE_BLANK = 0,
    LOGSCOPE_PARSE_VALID = 1,
    LOGSCOPE_PARSE_INVALID = 2
} LogScopeParseOutcome;

typedef struct LogScopeParsedLine
{
    LogScopeParseOutcome outcome;
    const char* level;
    const char* message;
    const char* timestamp;
} LogScopeParsedLine;

typedef struct LogScopeFormatParserVTable
{
    void (*destroy)(void* instance);
    int (*parse_line)(void* instance, const char* line, size_t length, LogScopeParsedLine* out);
} LogScopeFormatParserVTable;

typedef struct LogScopeFormatParser
{
    void* instance;
    const LogScopeFormatParserVTable* vtable;
} LogScopeFormatParser;

typedef struct LogScopeReportFragment
{
    const char* text_body;
    const char* html_body;
    const char* json_key;
    const char* json_body;
} LogScopeReportFragment;

typedef struct LogScopeReportContributorVTable
{
    void (*destroy)(void* instance);
    int (*render)(void* instance, uint64_t total_lines, LogScopeReportFragment* out);
} LogScopeReportContributorVTable;

typedef struct LogScopeReportContributor
{
    void* instance;
    const LogScopeReportContributorVTable* vtable;
} LogScopeReportContributor;

typedef struct LogScopeSearchProviderVTable
{
    void (*destroy)(void* instance);
    int (*search)(void* instance, const char* query, LogScopeReportFragment* out);
} LogScopeSearchProviderVTable;

typedef struct LogScopeSearchProvider
{
    void* instance;
    const LogScopeSearchProviderVTable* vtable;
} LogScopeSearchProvider;

typedef struct LogScopeStorageBackendVTable
{
    void (*destroy)(void* instance);
    int (*create_store)(void* instance, void** out_store);
} LogScopeStorageBackendVTable;

typedef struct LogScopeStorageBackend
{
    void* instance;
    const LogScopeStorageBackendVTable* vtable;
} LogScopeStorageBackend;

typedef struct LogScopeStorageStoreVTable
{
    void (*destroy)(void* instance);
    int (*append_line)(void* instance, uint64_t line_number, const char* content);
    int (*finalize)(void* instance, uint64_t total_lines);
    uint64_t (*stored_line_count)(void* instance);
} LogScopeStorageStoreVTable;

typedef struct LogScopeStorageStore
{
    void* instance;
    const LogScopeStorageStoreVTable* vtable;
} LogScopeStorageStore;

struct LogScopeHostApi;

typedef LogScopeFormatParser* (*LogScopeCreateFormatParserFn)(void);
typedef LogScopeReportContributor* (*LogScopeCreateReportContributorFn)(void);
typedef LogScopeSearchProvider* (*LogScopeCreateSearchProviderFn)(void);
typedef LogScopeStorageBackend* (*LogScopeCreateStorageBackendFn)(void);

struct LogScopeHostApi
{
    uint32_t api_version;
    void* context;

    int (*register_extension)(void* context, const LogScopePluginInfo* info);

    int (*register_report_contributor)(void* context, const char* contributor_id,
                                       LogScopeCreateReportContributorFn create_fn);

    int (*register_format_parser)(void* context, const char* format_id,
                                  LogScopeCreateFormatParserFn create_fn);

    int (*register_search_provider)(void* context, const char* provider_id,
                                    LogScopeCreateSearchProviderFn create_fn);

    int (*register_storage_backend)(void* context, const char* backend_id,
                                    LogScopeCreateStorageBackendFn create_fn);
};

typedef int (*LogScopePluginRegisterFn)(const struct LogScopeHostApi* host);

LOGSCOPE_PLUGIN_API int logscope_plugin_register(const struct LogScopeHostApi* host);

#ifdef __cplusplus
}
#endif

#endif /* LOGSCOPE_PLUGIN_H */
