/**
 * @file application_service.cpp
 * @brief ApplicationService implementation.
 */

#include "application_service.hpp"

#include "indexed_line_access.hpp"

#include "ai_config.hpp"
#include "analysis_config.hpp"
#include "foundation/filesystem.hpp"
#include "foundation/error.hpp"
#include "analytics_engine.hpp"
#include "analysis_engine.hpp"
#include "investigation_engine.hpp"
#include "plugin_config.hpp"
#include "plugin_runtime.hpp"
#include "query.hpp"
#include "reporting.hpp"
#include "runtime.hpp"
#include "search_config.hpp"
#include "session_store.hpp"
#include "source.hpp"
#include "tailing_file_log_source.hpp"
#include "storage.hpp"
#include "tailing_file_log_source.hpp"

namespace scope::application
{

foundation::Result<bool> ApplicationService::loadConfiguration(const foundation::Path& configFile)
{
    m_configFile = configFile;

    if (configFile.string().empty())
    {
        m_configurationManager.applyEnvironment();

        if (!scope::runtime::Diagnostics::instance().applyConfiguration(m_configurationManager.configuration()))
        {
            return foundation::Result<bool>(
                foundation::Error(foundation::ErrorCode::InvalidArgument, "Failed to apply configuration."));
        }

        return foundation::Result<bool>(true);
    }

    const auto loadResult = configuration::ConfigurationManager::loadFromFile(configFile);

    if (!loadResult)
    {
        return foundation::Result<bool>(loadResult.error());
    }

    m_configurationManager = std::move(loadResult.value());
    m_configurationManager.applyEnvironment();

    if (!scope::runtime::Diagnostics::instance().applyConfiguration(m_configurationManager.configuration()))
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Failed to apply configuration."));
    }

    return foundation::Result<bool>(true);
}

foundation::Result<bool> ApplicationService::validateConfiguration(
    const std::vector<std::string>& requiredKeys) const
{
    const auto validationResult = m_configurationManager.validate(requiredKeys);

    if (!validationResult)
    {
        return foundation::Result<bool>(validationResult.error());
    }

    const auto analysisValidation =
        scope::analysis::validateAnalysisConfiguration(m_configurationManager.configuration());

    if (!analysisValidation)
    {
        return foundation::Result<bool>(analysisValidation.error());
    }

    const auto searchValidation =
        scope::search::validateSearchConfiguration(m_configurationManager.configuration());

    if (!searchValidation)
    {
        return foundation::Result<bool>(searchValidation.error());
    }

    const auto queryValidation = scope::query::validateQueryConfiguration(m_configurationManager.configuration());

    if (!queryValidation)
    {
        return foundation::Result<bool>(queryValidation.error());
    }

    const auto storageValidation =
        scope::storage::validateStorageConfiguration(m_configurationManager.configuration());

    if (!storageValidation)
    {
        return foundation::Result<bool>(storageValidation.error());
    }

    const auto pluginValidation =
        scope::plugin::validatePluginConfiguration(m_configurationManager.configuration());

    if (!pluginValidation)
    {
        return foundation::Result<bool>(pluginValidation.error());
    }

    const auto aiValidation = scope::ai::validateAiConfiguration(m_configurationManager.configuration());

    if (!aiValidation)
    {
        return foundation::Result<bool>(aiValidation.error());
    }

    return foundation::Result<bool>(true);
}

configuration::ConfigurationManager& ApplicationService::configurationManager() noexcept
{
    return m_configurationManager;
}

const configuration::ConfigurationManager& ApplicationService::configurationManager() const noexcept
{
    return m_configurationManager;
}

foundation::Path ApplicationService::configFilePath() const noexcept
{
    return m_configFile;
}

foundation::Path ApplicationService::sourcePath() const noexcept
{
    return m_sourcePath;
}

bool ApplicationService::hasModel() const noexcept
{
    return m_model.has_value();
}

const analysis::AnalysisModel& ApplicationService::model() const
{
    return *m_model;
}

extension::ExtensionManager ApplicationService::createExtensionManager() const
{
    m_pluginStats = plugin::PluginLoadStats{};
    return plugin::createConfiguredExtensionManager(m_configurationManager.configuration(), &m_pluginStats);
}

foundation::Result<source::SourceDataset> ApplicationService::openSource(const foundation::Path& path,
                                                                         const source::OpenOptions options)
{
    m_sourcePath = path;
    m_openOptions = options;
    m_model.reset();
    m_dataset.reset();
    stopTail();

    scope::source::SourceManager sourceManager;

    return sourceManager.open(path, options);
}

foundation::Result<analysis::AnalysisModel> ApplicationService::analyze(const analysis::AnalysisConfig& config,
                                                                        analysis::AnalysisStats* statsOut)
{
    if (m_sourcePath.string().empty())
    {
        return foundation::Result<analysis::AnalysisModel>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "No log source opened."));
    }

    const extension::ExtensionManager extensionManager = createExtensionManager();
    (void)extensionManager;

    if (!m_dataset.has_value())
    {
        auto openAgain = scope::source::SourceManager{}.open(m_sourcePath, m_openOptions);

        if (!openAgain)
        {
            return foundation::Result<analysis::AnalysisModel>(openAgain.error());
        }

        m_dataset.emplace(std::move(openAgain.value()));
    }

    scope::source::SourceDataset& dataset = *m_dataset;
    const auto modelResult = scope::analysis::AnalysisEngine{}.analyze(dataset, config, statsOut);

    if (!modelResult)
    {
        return modelResult;
    }

    m_model = *modelResult;

    return modelResult;
}

foundation::Result<investigation::InvestigationResult>
ApplicationService::investigate(const investigation::InvestigationCriteria& criteria)
{
    if (!m_model.has_value())
    {
        return foundation::Result<investigation::InvestigationResult>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Analyze a source before investigating."));
    }

    investigation::InvestigationCriteria resolved = criteria;
    investigation::applyInvestigationConfiguration(resolved, m_configurationManager.configuration());

    scope::investigation::InvestigationEngine engine;

    if (!resolved.isActive())
    {
        investigation::InvestigationResult result;
        result.correlations = engine.findCorrelations(*m_model);
        result.indexedLineCount = analysis::indexedLineCountForModel(*m_model);
        result.truncatedLineCount = analysis::truncatedLineCountForModel(*m_model);

        return foundation::Result<investigation::InvestigationResult>(std::move(result));
    }

    const auto queryResult = resolved.resolvedSearchQuery();

    if (!queryResult)
    {
        return foundation::Result<investigation::InvestigationResult>(queryResult.error());
    }

    const auto filterResult = resolved.resolvedFilterQuery();

    if (!filterResult)
    {
        return foundation::Result<investigation::InvestigationResult>(filterResult.error());
    }

    return foundation::Result<investigation::InvestigationResult>(engine.investigate(*m_model, resolved));
}

foundation::Result<analytics::AnalyticsResult> ApplicationService::runAnalytics(
    const analytics::AnalyticsConfig& config)
{
    if (!m_model.has_value())
    {
        return foundation::Result<analytics::AnalyticsResult>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Analyze a source before running analytics."));
    }

    return foundation::Result<analytics::AnalyticsResult>(
        analytics::AnalyticsEngine{}.analyze(*m_model, config));
}

reporting::Report ApplicationService::generateReport(const reporting::ReportOptions& options) const
{
    return reporting::ReportGenerator{}.generate(model(), options);
}

foundation::Result<bool> ApplicationService::saveSession(const SessionSaveRequest& request)
{
    if (!m_model.has_value())
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "No analysis model to save."));
    }

    search::SearchHistory searchHistory;

    if (request.contentCriteria.isActive())
    {
        const auto queryResult = request.contentCriteria.resolvedSearchQuery();

        if (queryResult && queryResult->isActive())
        {
            searchHistory.add(queryResult->toString());
        }
    }

    const workspace::InvestigationSession session = workspace::InvestigationSession::fromAnalysis(
        *m_model,
        request.lineFilter,
        request.levelFilter,
        request.searchQuery,
        request.contentCriteria,
        searchHistory,
        request.reportOptions,
        request.configFile);

    const workspace::SessionStore store;

    return store.save(session, request.sessionFile);
}

foundation::Result<workspace::InvestigationSession> ApplicationService::loadSession(
    const foundation::Path& sessionFile)
{
    const workspace::SessionStore store;

    return store.load(sessionFile);
}

foundation::Result<std::vector<foundation::Path>> ApplicationService::listSessions(
    const foundation::Path& directory) const
{
    const workspace::SessionStore store;

    return store.list(directory);
}

std::vector<extension::ExtensionInfo> ApplicationService::listExtensions() const
{
    return createExtensionManager().listExtensions();
}

foundation::Result<extension::ExtensionInfo> ApplicationService::describeExtension(
    const std::string& extensionId) const
{
    const extension::ExtensionManager manager = createExtensionManager();

    for (const extension::ExtensionInfo& info : manager.listExtensions())
    {
        if (info.id == extensionId)
        {
            return foundation::Result<extension::ExtensionInfo>(info);
        }
    }

    return foundation::Result<extension::ExtensionInfo>(
        foundation::Error(foundation::ErrorCode::FileNotFound, "Extension not found: " + extensionId));
}

foundation::Result<AgentInvestigateResult> ApplicationService::agentInvestigate(
    const investigation::InvestigationCriteria& criteria,
    const std::string& askQuery,
    bool summarize,
    bool hints,
    analysis::AnalysisStats* statsOut)
{
    if (m_sourcePath.string().empty())
    {
        return foundation::Result<AgentInvestigateResult>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "No log source opened."));
    }

    const extension::ExtensionManager extensionManager = createExtensionManager();
    (void)extensionManager;

    const ai::AiConfig aiConfig = ai::resolveAiConfig(m_configurationManager.configuration());
    const ai::AiInvestigationAssistant assistant(aiConfig);

    if (!hasModel())
    {
        const auto analysisConfig = analysis::resolveAnalysisConfig(m_configurationManager.configuration(),
                                                                  analysis::AnalysisConfig::defaults());
        const auto modelResult = analyze(analysisConfig, statsOut);

        if (!modelResult)
        {
            return foundation::Result<AgentInvestigateResult>(modelResult.error());
        }
    }

    investigation::InvestigationCriteria resolvedCriteria = criteria;
    investigation::applyInvestigationConfiguration(resolvedCriteria, m_configurationManager.configuration());

    if (!askQuery.empty())
    {
        const auto filterResult = assistant.translateNaturalLanguageQuery(askQuery);

        if (!filterResult)
        {
            return foundation::Result<AgentInvestigateResult>(filterResult.error());
        }

        resolvedCriteria.filterExpression = *filterResult;
    }

    AgentInvestigateResult agentResult;

    const auto investigationResult = investigate(resolvedCriteria);

    if (!investigationResult)
    {
        return foundation::Result<AgentInvestigateResult>(investigationResult.error());
    }

    agentResult.investigation = *investigationResult;

    scope::investigation::InvestigationEngine investigationEngine;
    const scope::investigation::InvestigationView view = investigationEngine.inspect(*m_model);

    if (summarize && aiConfig.enabled)
    {
        const auto summary = assistant.summarizeInvestigation(view, agentResult.investigation);

        if (summary)
        {
            agentResult.summary = *summary;
        }
        else
        {
            agentResult.aiErrors.push_back(summary.error().message());
        }
    }

    if (hints && aiConfig.enabled)
    {
        const auto analyticsResult = scope::analytics::AnalyticsEngine{}.analyze(*m_model);
        const auto hintResult = assistant.suggestAnomalyHints(analyticsResult);

        if (hintResult)
        {
            agentResult.hints = *hintResult;
        }
        else
        {
            agentResult.aiErrors.push_back(hintResult.error().message());
        }
    }

    return foundation::Result<AgentInvestigateResult>(std::move(agentResult));
}

foundation::Result<bool> ApplicationService::startTail()
{
    if (m_sourcePath.string().empty())
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Open a file before tailing."));
    }

    const auto isFile = foundation::FileSystem::isFile(m_sourcePath);

    if (!isFile || !*isFile)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Tail is supported for single files only."));
    }

    stopTail();

    auto tailResult = source::TailingFileLogSource::openAtEnd(m_sourcePath);

    if (!tailResult)
    {
        return foundation::Result<bool>(tailResult.error());
    }

    m_tailSource = std::move(tailResult.value());

    return foundation::Result<bool>(true);
}

void ApplicationService::stopTail()
{
    if (m_tailSource != nullptr)
    {
        auto* tailing = dynamic_cast<source::TailingFileLogSource*>(m_tailSource.get());

        if (tailing != nullptr)
        {
            tailing->stopFollow();
        }

        m_tailSource.reset();
    }
}

bool ApplicationService::isTailing() const noexcept
{
    if (m_tailSource == nullptr)
    {
        return false;
    }

    const auto* tailing = dynamic_cast<const source::TailingFileLogSource*>(m_tailSource.get());

    return tailing != nullptr && tailing->isFollowing();
}

foundation::Result<std::vector<std::string>> ApplicationService::pollTailLines()
{
    if (m_tailSource == nullptr)
    {
        return foundation::Result<std::vector<std::string>>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Tail is not active."));
    }

    std::vector<std::string> lines;
    std::string line;

    auto* tailing = dynamic_cast<source::TailingFileLogSource*>(m_tailSource.get());

    for (int i = 0; i < 64; ++i)
    {
        const foundation::Result<bool> readResult =
            tailing != nullptr ? tailing->pollLine(line) : m_tailSource->readLine(line);

        if (!readResult)
        {
            return foundation::Result<std::vector<std::string>>(readResult.error());
        }

        if (!*readResult)
        {
            break;
        }

        lines.push_back(line);
    }

    return foundation::Result<std::vector<std::string>>(std::move(lines));
}

const plugin::PluginLoadStats& ApplicationService::lastPluginStats() const noexcept
{
    return m_pluginStats;
}

void ApplicationService::adoptModel(const analysis::AnalysisModel& model, const foundation::Path& sourcePath)
{
    m_model = model;
    m_sourcePath = sourcePath;
}

} // namespace scope::application
