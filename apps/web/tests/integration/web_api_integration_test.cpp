/**
 * @file web_api_integration_test.cpp
 * @brief REST integration tests against real fixtures (M15.1).
 */

#include <gtest/gtest.h>
#include <httplib.h>

#include "foundation/path.hpp"
#include "middleware/api_key.hpp"
#include "web_config.hpp"
#include "web_server.hpp"

namespace
{

std::string sourcePath(const std::string& relativePath)
{
    return std::string(LOGSCOPE_SOURCE_DIR) + "/" + relativePath;
}

class WebApiIntegrationTest : public ::testing::Test
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

    httplib::Headers sessionHeaders(const std::string& sessionId) const
    {
        httplib::Headers headers;
        headers.emplace(scope::web::kSessionHeader, sessionId);

        return headers;
    }

    scope::web::WebConfig config;
    std::unique_ptr<scope::web::WebServer> server;
    std::unique_ptr<httplib::Client> client;
};

} // namespace

TEST_F(WebApiIntegrationTest, OpenAnalyzeInvestigateSampleLog)
{
    const std::string sessionId = createSession();
    const httplib::Headers headers = sessionHeaders(sessionId);

    const std::string openBody = "{\"path\": \"" + sourcePath("samples/sample.log") + "\"}";
    const httplib::Result openResult = client->Post("/api/v1/sources/open", headers, openBody, "application/json");
    ASSERT_TRUE(openResult);
    EXPECT_EQ(200, openResult->status);

    const httplib::Result analyzeResult = client->Post("/api/v1/analyze", headers, "{}", "application/json");
    ASSERT_TRUE(analyzeResult);
    EXPECT_EQ(200, analyzeResult->status);
    EXPECT_NE(std::string::npos, analyzeResult->body.find("\"totalLines\": 8"));

    const std::string investigateBody = "{\"search\": \"error\"}";
    const httplib::Result investigateResult =
        client->Post("/api/v1/investigate", headers, investigateBody, "application/json");
    ASSERT_TRUE(investigateResult);
    EXPECT_EQ(200, investigateResult->status);
    EXPECT_NE(std::string::npos, investigateResult->body.find("\"matchingLineCount\""));
}

TEST_F(WebApiIntegrationTest, RejectsTraversalPathWhenOpeningSource)
{
    const std::string sessionId = createSession();
    const httplib::Headers headers = sessionHeaders(sessionId);
    const httplib::Result result =
        client->Post("/api/v1/sources/open", headers, "{\"path\": \"../../../etc/passwd\"}", "application/json");

    ASSERT_TRUE(result);
    EXPECT_EQ(400, result->status);
}

TEST_F(WebApiIntegrationTest, ListsExtensions)
{
    const std::string sessionId = createSession();
    const httplib::Headers headers = sessionHeaders(sessionId);
    const httplib::Result result = client->Get("/api/v1/extensions", headers);

    ASSERT_TRUE(result);
    EXPECT_EQ(200, result->status);
    EXPECT_NE(std::string::npos, result->body.find("\"id\""));
}
