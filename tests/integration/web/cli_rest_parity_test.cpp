/**
 * @file cli_rest_parity_test.cpp
 * @brief Parity tests: REST responses match CLI --format json (M15.1).
 */

#include <gtest/gtest.h>
#include <httplib.h>

#include <optional>
#include <string>

#include "foundation/path.hpp"
#include "middleware/api_key.hpp"
#include "process.hpp"
#include "web_config.hpp"
#include "web_server.hpp"

namespace
{

std::string logscopeExecutable()
{
    return LOGSCOPE_EXECUTABLE;
}

std::string sourcePath(const std::string& relativePath)
{
    return std::string(LOGSCOPE_SOURCE_DIR) + "/" + relativePath;
}

std::string runLogscope(const std::string& arguments)
{
#if defined(_WIN32)
    const std::string command = "cmd /c " +
                                scope::test_support::quoteArgument(scope::test_support::quoteArgument(logscopeExecutable()) +
                                                                   " " + arguments + " 2>&1");
#else
    const std::string command =
        scope::test_support::quoteArgument(logscopeExecutable()) + " " + arguments + " 2>&1";
#endif

    return scope::test_support::captureCommandOutput(command);
}

std::optional<std::size_t> extractJsonSizeT(const std::string& body, const std::string& key)
{
    const std::string quoted = '"' + key + '"';
    const std::size_t position = body.find(quoted);

    if (position == std::string::npos)
    {
        return std::nullopt;
    }

    const std::size_t colon = body.find(':', position);

    if (colon == std::string::npos)
    {
        return std::nullopt;
    }

    std::size_t index = colon + 1U;

    while (index < body.size() && (body[index] == ' ' || body[index] == '\n'))
    {
        ++index;
    }

    std::size_t value = 0U;

    while (index < body.size() && body[index] >= '0' && body[index] <= '9')
    {
        value = value * 10U + static_cast<std::size_t>(body[index] - '0');
        ++index;
    }

    return value;
}

class CliRestParityTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        config = scope::web::WebConfig::defaults();
        config.bindPort = 0;
        config.allowServerPaths = true;
        config.allowedPathRoots = {scope::foundation::Path(sourcePath("samples"))};
        server = std::make_unique<scope::web::WebServer>(config);
        ASSERT_TRUE(server->startInBackground());
        client = std::make_unique<httplib::Client>("127.0.0.1", server->port());
    }

    void TearDown() override
    {
        client.reset();
        server->stop();
        server.reset();
    }

    std::string createSession()
    {
        const httplib::Result result = client->Post("/api/v1/sessions/workspace");
        EXPECT_TRUE(result);
        EXPECT_EQ(200, result->status);

        const auto header = result->headers.find(scope::web::kSessionHeader);
        EXPECT_NE(header, result->headers.end());

        return header->second;
    }

    scope::web::WebConfig config;
    std::unique_ptr<scope::web::WebServer> server;
    std::unique_ptr<httplib::Client> client;
};

} // namespace

TEST_F(CliRestParityTest, AnalyzeTotalLinesMatchSampleLog)
{
    const std::string cliOutput =
        runLogscope("analyze --format json " + scope::test_support::quoteArgument(sourcePath("samples/sample.log")));
    const std::optional<std::size_t> cliTotalLines = extractJsonSizeT(cliOutput, "totalLines");
    ASSERT_TRUE(cliTotalLines.has_value());

    const std::string sessionId = createSession();
    httplib::Headers headers;
    headers.emplace(scope::web::kSessionHeader, sessionId);

    const std::string openBody = "{\"path\": \"" + sourcePath("samples/sample.log") + "\"}";
    ASSERT_TRUE(client->Post("/api/v1/sources/open", headers, openBody, "application/json"));
    const httplib::Result analyzeResult = client->Post("/api/v1/analyze", headers, "{}", "application/json");
    ASSERT_TRUE(analyzeResult);
    const std::optional<std::size_t> restTotalLines = extractJsonSizeT(analyzeResult->body, "totalLines");
    ASSERT_TRUE(restTotalLines.has_value());

    EXPECT_EQ(*cliTotalLines, *restTotalLines);
}

TEST_F(CliRestParityTest, InvestigateMatchingLineCountMatchesSampleLog)
{
    const std::string cliOutput = runLogscope("investigate --format json --search error " +
                                              scope::test_support::quoteArgument(sourcePath("samples/sample.log")));
    const std::optional<std::size_t> cliMatchCount = extractJsonSizeT(cliOutput, "matchingLineCount");
    ASSERT_TRUE(cliMatchCount.has_value());

    const std::string sessionId = createSession();
    httplib::Headers headers;
    headers.emplace(scope::web::kSessionHeader, sessionId);

    const std::string openBody = "{\"path\": \"" + sourcePath("samples/sample.log") + "\"}";
    ASSERT_TRUE(client->Post("/api/v1/sources/open", headers, openBody, "application/json"));
    ASSERT_TRUE(client->Post("/api/v1/analyze", headers, "{}", "application/json"));

    const httplib::Result investigateResult =
        client->Post("/api/v1/investigate", headers, "{\"search\": \"error\"}", "application/json");
    ASSERT_TRUE(investigateResult);
    const std::optional<std::size_t> restMatchCount = extractJsonSizeT(investigateResult->body, "matchingLineCount");
    ASSERT_TRUE(restMatchCount.has_value());

    EXPECT_EQ(*cliMatchCount, *restMatchCount);
}

TEST_F(CliRestParityTest, AnalyticsBucketCountsMatchSampleLog)
{
    const std::string cliOutput = runLogscope("analytics --format json " +
                                              scope::test_support::quoteArgument(sourcePath("samples/sample.log")));
    const std::optional<std::size_t> cliClusterCount = extractJsonSizeT(cliOutput, "clusterCount");
    ASSERT_TRUE(cliClusterCount.has_value());

    const std::string sessionId = createSession();
    httplib::Headers headers;
    headers.emplace(scope::web::kSessionHeader, sessionId);

    const std::string openBody = "{\"path\": \"" + sourcePath("samples/sample.log") + "\"}";
    ASSERT_TRUE(client->Post("/api/v1/sources/open", headers, openBody, "application/json"));
    ASSERT_TRUE(client->Post("/api/v1/analyze", headers, "{}", "application/json"));

    const httplib::Result analyticsResult = client->Post("/api/v1/analytics", headers, "{}", "application/json");
    ASSERT_TRUE(analyticsResult);
    const std::optional<std::size_t> restClusterCount = extractJsonSizeT(analyticsResult->body, "clusterCount");
    ASSERT_TRUE(restClusterCount.has_value());

    EXPECT_EQ(*cliClusterCount, *restClusterCount);
}

TEST_F(CliRestParityTest, AgentInvestigateAskErrorsMatchesCli)
{
    const std::string cliOutput =
        runLogscope("agent investigate --config " + scope::test_support::quoteArgument(sourcePath("samples/ai-noop.properties")) +
                    " --format json --ask errors " +
                    scope::test_support::quoteArgument(sourcePath("samples/sample.log")));
    const std::optional<std::size_t> cliMatchCount = extractJsonSizeT(cliOutput, "matchingLineCount");
    ASSERT_TRUE(cliMatchCount.has_value());

    const std::string sessionId = createSession();
    httplib::Headers headers;
    headers.emplace(scope::web::kSessionHeader, sessionId);

    ASSERT_TRUE(client->Post("/api/v1/config/load", headers,
                             "{\"path\": \"" + sourcePath("samples/ai-noop.properties") + "\"}",
                             "application/json"));

    const std::string openBody = "{\"path\": \"" + sourcePath("samples/sample.log") + "\"}";
    ASSERT_TRUE(client->Post("/api/v1/sources/open", headers, openBody, "application/json"));
    ASSERT_TRUE(client->Post("/api/v1/analyze", headers, "{}", "application/json"));

    const httplib::Result agentResult =
        client->Post("/api/v1/agent/investigate", headers, "{\"ask\": \"errors\"}", "application/json");
    ASSERT_TRUE(agentResult);
    const std::optional<std::size_t> restMatchCount = extractJsonSizeT(agentResult->body, "matchingLineCount");
    ASSERT_TRUE(restMatchCount.has_value());

    EXPECT_EQ(*cliMatchCount, *restMatchCount);
}
