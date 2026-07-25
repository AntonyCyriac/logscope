/**
 * @file ai_provider_test.cpp
 */

#include <gtest/gtest.h>

#include "ai_config.hpp"
#include "ai_provider.hpp"

using scope::ai::AiConfig;
using scope::ai::createAiProvider;
using scope::ai::kProviderHttp;
using scope::ai::kProviderNoOp;

TEST(AiProviderTest, CreateNoopByDefault)
{
    const AiConfig config;
    const auto provider = createAiProvider(config);

    ASSERT_NE(provider, nullptr);
    EXPECT_EQ(provider->id(), kProviderNoOp);
}

TEST(AiProviderTest, CreateHttpWhenEnabled)
{
    AiConfig config;
    config.enabled = true;
    config.provider = kProviderHttp;
    config.endpoint = "http://localhost:11434/v1";
    config.model = "llama3";

    const auto provider = createAiProvider(config);

    ASSERT_NE(provider, nullptr);
    EXPECT_EQ(provider->id(), kProviderHttp);
}

TEST(AiProviderTest, HttpProviderRequiresApiKey)
{
    AiConfig config;
    config.enabled = true;
    config.provider = kProviderHttp;
    config.endpoint = "http://localhost:11434/v1";
    config.model = "llama3";

    const auto provider = createAiProvider(config);

    ASSERT_NE(provider, nullptr);

    const auto result = provider->translateNlToFilter("errors");

    EXPECT_FALSE(result);
}
