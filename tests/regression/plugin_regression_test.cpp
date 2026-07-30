/**
 * @file plugin_regression_test.cpp
 * @brief Regression guards for M12 plugin failure isolation.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "extension_manager.hpp"
#include "plugin_loader.hpp"
#include "source.hpp"

using scope::analysis::AnalysisConfig;
using scope::analysis::AnalysisEngine;
using scope::foundation::Path;
using scope::plugin::loadPluginsForManager;
using scope::source::SourceManager;

TEST(PluginRegressionTest, AnalyzeContinuesWhenPluginSearchPathMissing)
{
    scope::extension::ExtensionManager manager;

    scope::plugin::PluginConfig pluginConfig;
    pluginConfig.enabled = true;
    pluginConfig.paths = {Path("/path/does/not/exist/for_logscope_plugins")};

    const auto loadResult = loadPluginsForManager(manager, pluginConfig);

    ASSERT_TRUE(loadResult);
    EXPECT_EQ(0U, *loadResult);

    SourceManager sourceManager;
    auto dataset = sourceManager.open(Path("samples/sample.log"));

    ASSERT_TRUE(dataset);

    const auto model = AnalysisEngine{}.analyze(*dataset, AnalysisConfig::defaults());

    ASSERT_TRUE(model);
    EXPECT_GT(model->totalLines(), 0U);
}
