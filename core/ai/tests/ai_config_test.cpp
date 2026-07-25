/**
 * @file ai_config_test.cpp
 */

#include <gtest/gtest.h>

#include "ai_config.hpp"
#include "runtime/configuration.hpp"

using scope::ai::kAiEnabledKey;
using scope::ai::kAiMaxContextLinesKey;
using scope::ai::kAiProviderKey;
using scope::ai::kProviderHttp;
using scope::ai::kProviderNoOp;
using scope::ai::resolveAiConfig;
using scope::ai::validateAiConfiguration;
using scope::runtime::Configuration;

TEST(AiConfigTest, DefaultsToDisabledNoopProvider)
{
    const Configuration configuration;
    const auto config = resolveAiConfig(configuration);

    EXPECT_FALSE(config.enabled);
    EXPECT_EQ(config.provider, kProviderNoOp);
    EXPECT_EQ(config.maxContextLines, 200U);
}

TEST(AiConfigTest, DisabledForcesNoopProvider)
{
    Configuration configuration;
    configuration.set(kAiEnabledKey, "false");
    configuration.set(kAiProviderKey, kProviderHttp);

    const auto config = resolveAiConfig(configuration);

    EXPECT_FALSE(config.enabled);
    EXPECT_EQ(config.provider, kProviderNoOp);
}

TEST(AiConfigTest, RejectsUnknownProvider)
{
    Configuration configuration;
    configuration.set(kAiProviderKey, "unknown");

    const auto result = validateAiConfiguration(configuration);

    EXPECT_FALSE(result);
}

TEST(AiConfigTest, RequiresEndpointAndModelForHttp)
{
    Configuration configuration;
    configuration.set(kAiEnabledKey, "true");
    configuration.set(kAiProviderKey, kProviderHttp);

    const auto result = validateAiConfiguration(configuration);

    EXPECT_FALSE(result);
}

TEST(AiConfigTest, AcceptsValidHttpConfiguration)
{
    Configuration configuration;
    configuration.set(kAiEnabledKey, "true");
    configuration.set(kAiProviderKey, kProviderHttp);
    configuration.set("ai.endpoint", "http://localhost:11434/v1");
    configuration.set("ai.model", "llama3");

    const auto result = validateAiConfiguration(configuration);

    ASSERT_TRUE(result);
}

TEST(AiConfigTest, RejectsZeroMaxContextLines)
{
    Configuration configuration;
    configuration.set(kAiMaxContextLinesKey, "0");

    const auto result = validateAiConfiguration(configuration);

    EXPECT_FALSE(result);
}
