/**
 * @file http_ai_provider_test.cpp
 */

#include <gtest/gtest.h>

#include <cstdlib>

#include "ai_config.hpp"
#include "http_ai_client.hpp"
#include "http_ai_provider.hpp"
#include "http_ai_mock_server.hpp"

using scope::ai::AiAnalyticsContext;
using scope::ai::AiConfig;
using scope::ai::AiInvestigationContext;
using scope::ai::HttpAiClient;
using scope::ai::HttpAiProvider;
using scope::ai::test::HttpAiMockServer;

namespace
{

class ApiKeyEnvironment final
{
  public:
    explicit ApiKeyEnvironment(const char* value)
    {
#if defined(_WIN32)
        _putenv_s("LOGSCOPE_AI_API_KEY", value);
#else
        setenv("LOGSCOPE_AI_API_KEY", value, 1);
#endif
    }

    ~ApiKeyEnvironment()
    {
#if defined(_WIN32)
        _putenv_s("LOGSCOPE_AI_API_KEY", "");
#else
        unsetenv("LOGSCOPE_AI_API_KEY");
#endif
    }
};

std::string chatCompletionResponse(const std::string& content)
{
    return R"({"choices":[{"message":{"role":"assistant","content":")" + content + R"("}}]})";
}

AiConfig makeHttpConfig(const std::string& endpoint)
{
    AiConfig config;
    config.enabled = true;
    config.provider = "http";
    config.endpoint = endpoint;
    config.model = "test-model";

    return config;
}

} // namespace

TEST(HttpAiProviderTest, TranslatesNaturalLanguageThroughMockServer)
{
    ApiKeyEnvironment apiKey("test-key");
    HttpAiMockServer server;
    server.setResponse(chatCompletionResponse("level == ERROR"));
    server.start();

    const HttpAiProvider provider(makeHttpConfig(server.endpoint()));
    const auto result = provider.translateNlToFilter("show errors");

    server.stop();

    ASSERT_TRUE(result);
    EXPECT_EQ("level == ERROR", *result);

    const std::string requestBody = server.lastRequestBody();
    server.stop();

    EXPECT_NE(std::string::npos, requestBody.find("\"model\":\"test-model\""));
    EXPECT_NE(std::string::npos, requestBody.find("show errors"));
}

TEST(HttpAiProviderTest, SummarizeUsesMockServerResponse)
{
    ApiKeyEnvironment apiKey("test-key");
    HttpAiMockServer server;
    server.setResponse(chatCompletionResponse("Two connection errors detected."));
    server.start();

    AiInvestigationContext context;
    context.matchCount = 2U;
    context.sourceSummary = "source=sample.log";

    const HttpAiProvider provider(makeHttpConfig(server.endpoint()));
    const auto result = provider.summarize(context);

    server.stop();

    ASSERT_TRUE(result);
    EXPECT_EQ("Two connection errors detected.", result->summary);
}

TEST(HttpAiProviderTest, SuggestAnomaliesParsesHintLines)
{
    ApiKeyEnvironment apiKey("test-key");
    HttpAiMockServer server;
    server.setResponse(chatCompletionResponse("Spike in ERROR rate\nRepeated connection reset"));
    server.start();

    AiAnalyticsContext context;
    context.hasSpike = true;

    const HttpAiProvider provider(makeHttpConfig(server.endpoint()));
    const auto result = provider.suggestAnomalies(context);

    server.stop();

    ASSERT_TRUE(result);
    ASSERT_EQ(2U, result->size());
    EXPECT_EQ("Spike in ERROR rate", result->at(0).message);
}

TEST(HttpAiClientTest, SupportsLocalV1EndpointShape)
{
    ApiKeyEnvironment apiKey("test-key");
    HttpAiMockServer server;
    server.setResponse(chatCompletionResponse("level == WARN"));
    server.start();

    const HttpAiClient client(makeHttpConfig(server.endpoint()));
    const auto result = client.chatCompletion("system", "warnings");

    server.stop();

    ASSERT_TRUE(result);
    EXPECT_EQ("level == WARN", *result);
}

TEST(HttpAiClientTest, SurfacesReadTimeout)
{
    ApiKeyEnvironment apiKey("test-key");
    HttpAiMockServer server;
    server.setResponse(chatCompletionResponse("slow"));
    server.setDelay(std::chrono::seconds(3));
    server.start();

    HttpAiClient::Settings settings;
    settings.connectTimeoutSeconds = 1;
    settings.readTimeoutSeconds = 1;

    const HttpAiClient client(makeHttpConfig(server.endpoint()), settings);
    const auto result = client.chatCompletion("system", "timeout");

    server.stop();

    EXPECT_FALSE(result);
    EXPECT_NE(std::string::npos, result.error().message().find("HTTP AI request failed"));
}
