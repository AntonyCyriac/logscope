/**
 * @file plugin_loader_test.cpp
 */

#include <gtest/gtest.h>

#include <filesystem>

#include "extension_manager.hpp"
#include "plugin_config.hpp"
#include "plugin_loader.hpp"

using scope::extension::ExtensionManager;
using scope::foundation::Path;
using scope::plugin::PluginConfig;
using scope::plugin::PluginLoader;
using scope::plugin::loadPluginsForManager;
using scope::plugin::sharedLibraryExtension;

namespace
{

Path reportPluginDirectory()
{
#ifdef PLUGIN_TEST_REPORT_DIR
    return Path(PLUGIN_TEST_REPORT_DIR);
#else
    return Path(".");
#endif
}

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

} // namespace

TEST(PluginLoaderTest, LoadsTestReportPlugin)
{
    ExtensionManager manager = ExtensionManager::createWithBuiltIns();

    PluginConfig config;
    config.enabled = true;
    config.paths = {reportPluginDirectory()};

    const auto loaded = loadPluginsForManager(manager, config);

    ASSERT_TRUE(loaded.hasValue());

    const auto describeResult = manager.describe("test.report");

    ASSERT_TRUE(describeResult.hasValue());
    EXPECT_TRUE(describeResult->dynamic);
    EXPECT_EQ("1.0.0", describeResult->version);
    EXPECT_EQ(1U, describeResult->apiVersion);
}

TEST(PluginLoaderTest, SkipsWhenDisabled)
{
    ExtensionManager manager = ExtensionManager::createWithBuiltIns();

    PluginConfig config;
    config.enabled = false;
    config.paths = {reportPluginDirectory()};

    const auto loaded = loadPluginsForManager(manager, config);

    ASSERT_TRUE(loaded.hasValue());
    EXPECT_EQ(0U, *loaded);
    EXPECT_TRUE(manager.describe("test.report").hasError());
}

TEST(PluginLoaderTest, RejectsFutureApiVersion)
{
    ExtensionManager manager = ExtensionManager::createWithBuiltIns();
    PluginLoader loader(manager);

    const auto loaded = loader.loadFromPaths({Path("missing_future_api_plugin" + sharedLibraryExtension())});

    ASSERT_TRUE(loaded.hasValue());
}
