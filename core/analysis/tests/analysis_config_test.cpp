/**
 * @file analysis_config_test.cpp
 * @brief Unit tests for analysis configuration resolution.
 */

#include <gtest/gtest.h>

#include "analysis_config.hpp"
#include "runtime/configuration.hpp"

using scope::analysis::AnalysisConfig;
using scope::analysis::LogFormat;
using scope::analysis::maxIndexedLines;
using scope::analysis::resolveAnalysisConfig;
using scope::analysis::resolveFormatProfile;
using scope::analysis::validateAnalysisConfiguration;
using scope::runtime::Configuration;

TEST(AnalysisConfigTest, ResolvesGenericJsonProfile)
{
    const auto profile = resolveFormatProfile("generic-json");

    ASSERT_TRUE(profile.has_value());
    EXPECT_EQ(LogFormat::JsonLines, profile->defaults.formatHint);
}

TEST(AnalysisConfigTest, AppliesConfigurationKeys)
{
    Configuration configuration;
    configuration.set("source.format", "plain");
    configuration.set("source.json.timestamp_field", "@ts");
    configuration.set("investigation.max_indexed_lines", "500");

    const AnalysisConfig config = resolveAnalysisConfig(configuration, AnalysisConfig::defaults());

    EXPECT_EQ(LogFormat::PlainText, config.formatHint);
    EXPECT_EQ("@ts", config.jsonFieldMapping.timestampField);
    EXPECT_EQ(500U, config.maxIndexedLines);
}

TEST(AnalysisConfigTest, ValidatesUnknownProfile)
{
    Configuration configuration;
    configuration.set("profile", "unknown-profile");

    const auto result = validateAnalysisConfiguration(configuration);

    EXPECT_FALSE(result.hasValue());
}

TEST(AnalysisConfigTest, CliOverrideWinsOverConfiguration)
{
    Configuration configuration;
    configuration.set("source.format", "plain");

    AnalysisConfig cliOverrides;
    cliOverrides.formatHint = LogFormat::JsonLines;

    const AnalysisConfig config = resolveAnalysisConfig(configuration, cliOverrides);

    EXPECT_EQ(LogFormat::JsonLines, config.formatHint);
}

TEST(AnalysisConfigTest, DefaultsPreserveIndexedLineCap)
{
    const AnalysisConfig config = AnalysisConfig::defaults();

    EXPECT_EQ(maxIndexedLines, config.maxIndexedLines);
}
