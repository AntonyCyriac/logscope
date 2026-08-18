/**
 * @file plugin_host_api.cpp
 */

#include "plugin_host_api.hpp"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "foundation/error.hpp"
#include "foundation/path.hpp"
#include "index_fingerprint.hpp"
#include "index_store.hpp"
#include "log_macros.hpp"
#include "parser_registry.hpp"
#include "report_section_contributor.hpp"
#include "report_section_renderer.hpp"
#include "search_provider.hpp"
#include "search_provider_registry.hpp"
#include "storage_backend_registry.hpp"
#include "plugin_runtime.hpp"

namespace scope::plugin
{

namespace
{

analysis::JsonLineParseResult toJsonLineParseResult(const LogScopeParsedLine& parsed) noexcept
{
    analysis::JsonLineParseResult result;

    switch (parsed.outcome)
    {
    case LOGSCOPE_PARSE_BLANK:
        result.outcome = analysis::JsonLineParseOutcome::Blank;

        return result;
    case LOGSCOPE_PARSE_INVALID:
        result.outcome = analysis::JsonLineParseOutcome::Invalid;

        return result;
    case LOGSCOPE_PARSE_VALID:
    default:
        break;
    }

    result.outcome = analysis::JsonLineParseOutcome::Valid;

    if (parsed.level != nullptr)
    {
        result.levelValue = parsed.level;
    }

    if (parsed.message != nullptr)
    {
        result.messageValue = parsed.message;
    }

    if (parsed.timestamp != nullptr)
    {
        result.timestampValue = parsed.timestamp;
    }

    return result;
}

class ScopedPluginStorageBackend final
{
  public:
    explicit ScopedPluginStorageBackend(LogScopeStorageBackend* backend) noexcept : m_backend(backend) {}

    ~ScopedPluginStorageBackend()
    {
        if (m_backend == nullptr)
        {
            return;
        }

        if (m_backend->vtable != nullptr && m_backend->vtable->destroy != nullptr)
        {
            m_backend->vtable->destroy(m_backend->instance);
        }

        delete m_backend;
    }

    ScopedPluginStorageBackend(const ScopedPluginStorageBackend&) = delete;
    ScopedPluginStorageBackend& operator=(const ScopedPluginStorageBackend&) = delete;

  private:
    LogScopeStorageBackend* m_backend;
};

class CFormatParserAdapter final : public analysis::FormatParser
{
  public:
    explicit CFormatParserAdapter(LogScopeFormatParser parser) : m_parser(parser) {}

    ~CFormatParserAdapter() override
    {
        if (!pluginProvidersMayDestroy())
        {
            return;
        }

        if (m_parser.vtable != nullptr && m_parser.vtable->destroy != nullptr)
        {
            m_parser.vtable->destroy(m_parser.instance);
        }
    }

    CFormatParserAdapter(const CFormatParserAdapter&) = delete;
    CFormatParserAdapter& operator=(const CFormatParserAdapter&) = delete;

    [[nodiscard]] analysis::JsonLineParseResult parseLine(std::string_view line) const noexcept override
    {
        if (m_parser.vtable == nullptr || m_parser.vtable->parse_line == nullptr)
        {
            analysis::JsonLineParseResult result;
            result.outcome = analysis::JsonLineParseOutcome::Invalid;

            return result;
        }

        LogScopeParsedLine parsed{};
        (void)m_parser.vtable->parse_line(m_parser.instance, line.data(), line.size(), &parsed);

        return toJsonLineParseResult(parsed);
    }

  private:
    LogScopeFormatParser m_parser;
};

class CReportContributorAdapter final : public reporting::ReportSectionContributor
{
  public:
    CReportContributorAdapter(std::string id, LogScopeCreateReportContributorFn createFn)
        : m_id(std::move(id)), m_createFn(createFn)
    {
    }

    ~CReportContributorAdapter() override
    {
        if (!pluginProvidersMayDestroy())
        {
            return;
        }

        if (!m_contributor.has_value())
        {
            return;
        }

        if (m_contributor->vtable != nullptr && m_contributor->vtable->destroy != nullptr)
        {
            m_contributor->vtable->destroy(m_contributor->instance);
        }
    }

    CReportContributorAdapter(const CReportContributorAdapter&) = delete;
    CReportContributorAdapter& operator=(const CReportContributorAdapter&) = delete;

    [[nodiscard]] std::string id() const override
    {
        return m_id;
    }

    [[nodiscard]] reporting::ReportSection section() const noexcept override
    {
        return reporting::ReportSection::SourceMetadata;
    }

    [[nodiscard]] reporting::ReportFragment render(const analysis::AnalysisModel& model) const override
    {
        reporting::ReportFragment fragment;

        ensureContributor();

        if (!m_contributor.has_value() || m_contributor->vtable == nullptr ||
            m_contributor->vtable->render == nullptr)
        {
            return fragment;
        }

        LogScopeReportFragment rendered{};
        (void)m_contributor->vtable->render(m_contributor->instance, model.totalLines(), &rendered);

        if (rendered.text_body != nullptr)
        {
            fragment.textBody = rendered.text_body;
        }

        if (rendered.html_body != nullptr)
        {
            fragment.htmlBody = rendered.html_body;
        }

        if (rendered.json_key != nullptr)
        {
            fragment.jsonKey = rendered.json_key;
        }

        if (rendered.json_body != nullptr)
        {
            fragment.jsonBody = rendered.json_body;
        }

        return fragment;
    }

  private:
    void ensureContributor() const
    {
        if (m_contributor.has_value() || m_createFn == nullptr)
        {
            return;
        }

        LogScopeReportContributor* contributor = m_createFn();

        if (contributor != nullptr && contributor->vtable != nullptr)
        {
            m_contributor = *contributor;
        }
        else
        {
            m_contributor = LogScopeReportContributor{};
        }
    }

    std::string m_id;
    LogScopeCreateReportContributorFn m_createFn{nullptr};
    mutable std::optional<LogScopeReportContributor> m_contributor;
};

class CSearchProviderAdapter final : public search::SearchProvider
{
  public:
    explicit CSearchProviderAdapter(std::string id, LogScopeSearchProvider provider)
        : m_id(std::move(id)), m_provider(provider)
    {
    }

    ~CSearchProviderAdapter() override
    {
        if (!pluginProvidersMayDestroy())
        {
            return;
        }

        if (m_provider.vtable != nullptr && m_provider.vtable->destroy != nullptr)
        {
            m_provider.vtable->destroy(m_provider.instance);
        }
    }

    CSearchProviderAdapter(const CSearchProviderAdapter&) = delete;
    CSearchProviderAdapter& operator=(const CSearchProviderAdapter&) = delete;

    [[nodiscard]] std::string id() const override
    {
        return m_id;
    }

    [[nodiscard]] std::vector<analysis::IndexedLine>
    search(const analysis::LineIndex& /*index*/, const search::SearchQuery& /*query*/) const override
    {
        return {};
    }

  private:
    std::string m_id;
    LogScopeSearchProvider m_provider;
};

class CStorageStoreAdapter final : public storage::IndexStore
{
  public:
    explicit CStorageStoreAdapter(LogScopeStorageStore store, storage::IndexMetadata metadata,
                                  foundation::Path path, std::string backendId)
        : m_store(store), m_metadata(std::move(metadata)), m_path(std::move(path)),
          m_backendId(std::move(backendId))
    {
    }

    ~CStorageStoreAdapter() override
    {
        if (!pluginProvidersMayDestroy())
        {
            return;
        }

        if (m_store.vtable != nullptr && m_store.vtable->destroy != nullptr)
        {
            m_store.vtable->destroy(m_store.instance);
        }
    }

    CStorageStoreAdapter(const CStorageStoreAdapter&) = delete;
    CStorageStoreAdapter& operator=(const CStorageStoreAdapter&) = delete;

    [[nodiscard]] foundation::Result<bool> appendLine(const analysis::IndexedLine& line,
                                                      std::string_view fullContent) override
    {
        if (m_store.vtable == nullptr || m_store.vtable->append_line == nullptr)
        {
            return foundation::Result<bool>(foundation::Error(foundation::ErrorCode::Unknown,
                                                              "Plugin storage store is unavailable."));
        }

        const int status =
            m_store.vtable->append_line(m_store.instance, line.lineNumber, std::string(fullContent).c_str());

        if (status != 0)
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::IOError,
                "Plugin storage backend '" + m_backendId + "' append_line failed with status " +
                    std::to_string(status) + "."));
        }

        return foundation::Result<bool>(true);
    }

    [[nodiscard]] foundation::Result<bool> finalize(std::uint64_t totalLines) override
    {
        if (m_store.vtable == nullptr || m_store.vtable->finalize == nullptr)
        {
            return foundation::Result<bool>(foundation::Error(foundation::ErrorCode::Unknown,
                                                              "Plugin storage store is unavailable."));
        }

        m_metadata.totalLines = totalLines;

        const int status = m_store.vtable->finalize(m_store.instance, totalLines);

        if (status != 0)
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::IOError,
                "Plugin storage backend '" + m_backendId + "' finalize failed with status " +
                    std::to_string(status) + "."));
        }

        return foundation::Result<bool>(true);
    }

    [[nodiscard]] std::uint64_t storedLineCount() const noexcept override
    {
        if (m_store.vtable == nullptr || m_store.vtable->stored_line_count == nullptr)
        {
            return 0U;
        }

        return m_store.vtable->stored_line_count(m_store.instance);
    }

    [[nodiscard]] const foundation::Path& path() const noexcept override
    {
        return m_path;
    }

    [[nodiscard]] const storage::IndexMetadata& metadata() const noexcept override
    {
        return m_metadata;
    }

    [[nodiscard]] foundation::Result<std::vector<analysis::IndexedLine>> fetchAllLines() const override
    {
        return foundation::Result<std::vector<analysis::IndexedLine>>(std::vector<analysis::IndexedLine>{});
    }

    [[nodiscard]] foundation::Result<std::vector<analysis::IndexedLine>>
    fetchLinesWhere(const std::string& /*sqlWhereClause*/) const override
    {
        return foundation::Result<std::vector<analysis::IndexedLine>>(std::vector<analysis::IndexedLine>{});
    }

  private:
    LogScopeStorageStore m_store;
    storage::IndexMetadata m_metadata;
    foundation::Path m_path;
    std::string m_backendId;
};

} // namespace

PluginHostApi::PluginHostApi(extension::ExtensionManager& extensionManager)
    : m_extensionManager(extensionManager)
{
    m_cApi.api_version = LOGSCOPE_HOST_API_VERSION;
    m_cApi.context = this;
    m_cApi.register_extension = &PluginHostApi::registerExtensionThunk;
    m_cApi.register_report_contributor = &PluginHostApi::registerReportContributorThunk;
    m_cApi.register_format_parser = &PluginHostApi::registerFormatParserThunk;
    m_cApi.register_search_provider = &PluginHostApi::registerSearchProviderThunk;
    m_cApi.register_storage_backend = &PluginHostApi::registerStorageBackendThunk;
}

LogScopeHostApi PluginHostApi::cApi() noexcept
{
    return m_cApi;
}

const std::vector<LoadedPluginRecord>& PluginHostApi::loadedPlugins() const noexcept
{
    return m_loadedPlugins;
}

void PluginHostApi::setCurrentLibraryPath(std::string path)
{
    m_currentLibraryPath = std::move(path);
}

int PluginHostApi::registerExtensionThunk(void* context, const LogScopePluginInfo* info)
{
    if (context == nullptr || info == nullptr || info->id == nullptr)
    {
        return 1;
    }

    auto* host = static_cast<PluginHostApi*>(context);

    if (info->api_version > LOGSCOPE_HOST_API_VERSION)
    {
        SCOPE_LOG_ERROR("plugin", std::string("Plugin API version mismatch for ") + info->id);

        return 2;
    }

    extension::ExtensionDescriptor descriptor;
    descriptor.id = info->id;
    descriptor.version = info->version != nullptr ? info->version : "0.0.0";
    descriptor.description = info->description != nullptr ? info->description : "";
    descriptor.enabledByDefault = true;
    descriptor.dynamic = true;
    descriptor.apiVersion = info->api_version;
    descriptor.libraryPath = host->m_currentLibraryPath;

    LoadedPluginRecord record;
    record.id = descriptor.id;
    record.version = descriptor.version;
    record.description = descriptor.description;
    record.apiVersion = info->api_version;
    record.libraryPath = descriptor.libraryPath;

    host->m_extensionManager.registerDynamic(std::move(descriptor));

    host->m_loadedPlugins.push_back(std::move(record));

    return 0;
}

int PluginHostApi::registerReportContributorThunk(void* context, const char* contributorId,
                                                  LogScopeCreateReportContributorFn createFn)
{
    if (context == nullptr || contributorId == nullptr || createFn == nullptr)
    {
        return 1;
    }

    reporting::ReportSectionRegistry::instance().registerContributor(
        std::make_unique<CReportContributorAdapter>(contributorId, createFn));

    SCOPE_LOG_DEBUG("plugin", std::string("Registered report contributor: ") + contributorId);

    return 0;
}

int PluginHostApi::registerFormatParserThunk(void* context, const char* formatId,
                                              LogScopeCreateFormatParserFn createFn)
{
    if (context == nullptr || formatId == nullptr || createFn == nullptr)
    {
        return 1;
    }

    analysis::ParserRegistry::instance().registerParser(formatId, [createFn]() {
        LogScopeFormatParser* parser = createFn();

        return std::make_unique<CFormatParserAdapter>(*parser);
    });

    SCOPE_LOG_DEBUG("plugin", std::string("Registered format parser: ") + formatId);

    return 0;
}

int PluginHostApi::registerSearchProviderThunk(void* context, const char* providerId,
                                                LogScopeCreateSearchProviderFn createFn)
{
    if (context == nullptr || providerId == nullptr || createFn == nullptr)
    {
        return 1;
    }

    search::SearchProviderRegistry::instance().registerProvider(providerId, [providerId, createFn]() {
        LogScopeSearchProvider* provider = createFn();

        return std::make_unique<CSearchProviderAdapter>(providerId, *provider);
    });

    SCOPE_LOG_DEBUG("plugin", std::string("Registered search provider: ") + providerId);

    return 0;
}

int PluginHostApi::registerStorageBackendThunk(void* context, const char* backendId,
                                                 LogScopeCreateStorageBackendFn createFn)
{
    if (context == nullptr || backendId == nullptr || createFn == nullptr)
    {
        return 1;
    }

    storage::StorageBackendRegistry::instance().registerBackend(
        backendId,
        [createFn, backendId](const storage::StorageConfig& /*config*/, const storage::IndexFingerprint& fingerprint,
                   const foundation::Path& sourcePath, const analysis::LogFormat format)
            -> foundation::Result<storage::IndexStorePtr> {
            LogScopeStorageBackend* const backend = createFn();

            if (backend == nullptr || backend->vtable == nullptr || backend->vtable->create_store == nullptr)
            {
                return foundation::Result<storage::IndexStorePtr>(foundation::Error(
                    foundation::ErrorCode::Unknown, "Storage backend factory returned null."));
            }

            ScopedPluginStorageBackend scopedBackend(backend);

            void* storeOpaque = nullptr;

            if (backend->vtable->create_store(backend->instance, &storeOpaque) != 0 || storeOpaque == nullptr)
            {
                return foundation::Result<storage::IndexStorePtr>(foundation::Error(
                    foundation::ErrorCode::Unknown, "Storage backend failed to create store."));
            }

            storage::IndexMetadata metadata;
            metadata.fingerprint = fingerprint.value();
            metadata.sourcePath = sourcePath;
            metadata.format = format;

            LogScopeStorageStore* store = static_cast<LogScopeStorageStore*>(storeOpaque);

            return foundation::Result<storage::IndexStorePtr>(storage::IndexStorePtr(
                new CStorageStoreAdapter(*store, std::move(metadata), sourcePath, std::string(backendId))));
        });

    SCOPE_LOG_DEBUG("plugin", std::string("Registered storage backend: ") + backendId);

    return 0;
}

} // namespace scope::plugin
