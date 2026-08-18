/**
 * @file provider_integration_test.cpp
 * @brief Integration tests for dynamic plugin providers (M12).
 */

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "analysis_engine.hpp"
#include "format_parser.hpp"
#include "extension_manager.hpp"
#include "investigation_engine.hpp"
#include "parser_registry.hpp"
#include "plugin_config.hpp"
#include "plugin_loader.hpp"
#include "report_section_renderer.hpp"
#include "search_provider_registry.hpp"
#include "source_manager.hpp"
#include "storage_backend_registry.hpp"

using scope::analysis::AnalysisConfig;
using scope::analysis::AnalysisEngine;
using scope::analysis::ParserRegistry;
using scope::extension::ExtensionManager;
using scope::foundation::Path;
using scope::investigation::InvestigationCriteria;
using scope::investigation::InvestigationEngine;
using scope::plugin::PluginConfig;
using scope::plugin::loadPluginsForManager;
using scope::reporting::ReportSectionRegistry;
using scope::search::SearchProviderRegistry;
using scope::search::SearchQuery;
using scope::source::SourceManager;
using scope::storage::StorageBackendRegistry;

namespace
{

Path fullPluginDirectory()
{
#ifdef PLUGIN_TEST_FULL_DIR
    return Path(PLUGIN_TEST_FULL_DIR);
#else
    return Path(".");
#endif
}

Path storagePluginDirectory()
{
#ifdef PLUGIN_TEST_STORAGE_DIR
    return Path(PLUGIN_TEST_STORAGE_DIR);
#else
    return Path(".");
#endif
}

Path failingStoragePluginDirectory()
{
#ifdef PLUGIN_TEST_FAILING_STORAGE_DIR
    return Path(PLUGIN_TEST_FAILING_STORAGE_DIR);
#else
    return Path(".");
#endif
}

void loadFullTestPlugin()
{
    ParserRegistry::instance().clear();
    SearchProviderRegistry::instance().clear();
    ReportSectionRegistry::instance().clearPluginContributors();

    ExtensionManager manager = ExtensionManager::createWithBuiltIns();

    PluginConfig config;
    config.enabled = true;
    config.paths = {fullPluginDirectory()};

    const auto loaded = loadPluginsForManager(manager, config);

    ASSERT_TRUE(loaded.hasValue());
    ASSERT_GT(*loaded, 0U);
}

void loadStorageTestPlugin()
{
    StorageBackendRegistry::instance().clear();

    ExtensionManager manager = ExtensionManager::createWithBuiltIns();

    PluginConfig config;
    config.enabled = true;
    config.paths = {storagePluginDirectory()};

    const auto loaded = loadPluginsForManager(manager, config);

    ASSERT_TRUE(loaded.hasValue());
    ASSERT_GT(*loaded, 0U);
}

void loadFailingStorageTestPlugin()
{
    StorageBackendRegistry::instance().clear();

    ExtensionManager manager = ExtensionManager::createWithBuiltIns();

    PluginConfig config;
    config.enabled = true;
    config.paths = {failingStoragePluginDirectory()};

    const auto loaded = loadPluginsForManager(manager, config);

    ASSERT_TRUE(loaded.hasValue());
    ASSERT_GT(*loaded, 0U);
}

} // namespace

TEST(PluginProviderIntegrationTest, RegistersParserProvider)
{
    loadFullTestPlugin();

    const std::vector<std::string> formatIds = ParserRegistry::instance().registeredFormatIds();

    EXPECT_TRUE(std::find(formatIds.begin(), formatIds.end(), "pipe-delimited") != formatIds.end());
}

TEST(PluginProviderIntegrationTest, RegistersSearchProvider)
{
    loadFullTestPlugin();

    const std::vector<std::string> providerIds = SearchProviderRegistry::instance().registeredProviderIds();

    EXPECT_TRUE(std::find(providerIds.begin(), providerIds.end(), "test.full.search") != providerIds.end());
}

TEST(PluginProviderIntegrationTest, RegistersStorageBackend)
{
    loadStorageTestPlugin();

    EXPECT_TRUE(StorageBackendRegistry::instance().findFactory("memory"));
}

TEST(PluginProviderIntegrationTest, RegistersReportContributor)
{
    loadFullTestPlugin();

    ExtensionManager manager = ExtensionManager::createWithBuiltIns();
    PluginConfig config;
    config.enabled = true;
    config.paths = {fullPluginDirectory()};
    const auto loaded = loadPluginsForManager(manager, config);

    ASSERT_TRUE(loaded.hasValue());
    ASSERT_GT(*loaded, 0U);

    const auto describeResult = manager.describe("test.full");

    ASSERT_TRUE(describeResult.hasValue());
    EXPECT_TRUE(describeResult->dynamic);
}

TEST(PluginProviderIntegrationTest, PropagatesPluginStorageWriteFailures)
{
    loadFailingStorageTestPlugin();

    const auto* testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
    const Path sourcePath = Path((std::filesystem::temp_directory_path() /
                                  ("logscope_plugin_fail_" + std::string(testInfo->name()) + ".log"))
                                     .string());

    {
        std::ofstream output(sourcePath.string());
        output << "alpha\nbeta\n";
    }

    scope::source::SourceManager sourceManager;
    auto datasetResult = sourceManager.open(sourcePath);

    ASSERT_TRUE(datasetResult.hasValue());

    AnalysisConfig config = AnalysisConfig::defaults();
    config.storage.persistIndex = true;
    config.storage.backend = "plugin:failing";
    config.storage.indexDirectory = Path(sourcePath.string() + ".ws");

    AnalysisEngine engine;
    const auto modelResult = engine.analyze(*datasetResult, config);

    EXPECT_FALSE(modelResult.hasValue());
    EXPECT_NE(modelResult.error().message().find("append_line failed"), std::string::npos);

    std::error_code error;
    std::filesystem::remove(sourcePath.string(), error);
    std::filesystem::remove_all(config.storage.indexDirectory.string(), error);
}
