/**
 * @file provider_integration_test.cpp
 * @brief Integration tests for dynamic plugin providers (M12).
 */

#include <algorithm>
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
using scope::analysis::FormatParser;
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

void loadFullTestPlugin()
{
    ParserRegistry::instance().clear();
    SearchProviderRegistry::instance().clear();

    ExtensionManager manager = ExtensionManager::createWithBuiltIns();

    PluginConfig config;
    config.enabled = true;
    config.paths = {fullPluginDirectory()};

    ASSERT_TRUE(loadPluginsForManager(manager, config).hasValue());
}

void loadStorageTestPlugin()
{
    StorageBackendRegistry::instance().clear();

    ExtensionManager manager = ExtensionManager::createWithBuiltIns();

    PluginConfig config;
    config.enabled = true;
    config.paths = {storagePluginDirectory()};

    ASSERT_TRUE(loadPluginsForManager(manager, config).hasValue());
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
    ASSERT_TRUE(loadPluginsForManager(manager, config).hasValue());

    const auto describeResult = manager.describe("test.full");

    ASSERT_TRUE(describeResult.hasValue());
    EXPECT_TRUE(describeResult->dynamic);
}
