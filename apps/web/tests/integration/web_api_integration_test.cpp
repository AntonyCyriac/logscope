/**
 * @file web_api_integration_test.cpp
 * @brief REST integration tests against real fixtures (M15.1).
 */

#include <gtest/gtest.h>
#include <httplib.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

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
        workspaceRoot = std::filesystem::temp_directory_path() / "logscope-web-api-integration";
        std::filesystem::remove_all(workspaceRoot);
        std::filesystem::create_directories(workspaceRoot);

        config = scope::web::WebConfig::defaults();
        config.bindPort = 0;
        config.allowServerPaths = true;
        config.allowedPathRoots = {scope::foundation::Path(sourcePath("samples"))};
        config.workspaceDir = scope::foundation::Path(workspaceRoot.string());
        server = std::make_unique<scope::web::WebServer>(config);
        ASSERT_TRUE(server->startInBackground());
        client = std::make_unique<httplib::Client>("127.0.0.1", server->port());
        client->set_connection_timeout(5, 0);
        client->set_read_timeout(120, 0);
    }

    [[nodiscard]] bool pollAnalyzeJobUntilComplete(const std::string& sessionId, const std::string& jobId,
                                                   const int maxAttempts = 200)
    {
        const httplib::Headers headers = sessionHeaders(sessionId);

        for (int attempt = 0; attempt < maxAttempts; ++attempt)
        {
            const httplib::Result pollResult = client->Get("/api/v1/jobs/" + jobId, headers);

            if (!pollResult || pollResult->status != 200)
            {
                return false;
            }

            if (pollResult->body.find("\"status\": \"completed\"") != std::string::npos)
            {
                return pollResult->body.find("\"totalLines\"") != std::string::npos;
            }

            if (pollResult->body.find("\"status\": \"failed\"") != std::string::npos)
            {
                return false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        return false;
    }

    void restartServer(const scope::web::WebConfig& newConfig)
    {
        client.reset();
        server->stop();
        server.reset();
        config = newConfig;
        server = std::make_unique<scope::web::WebServer>(config);
        ASSERT_TRUE(server->startInBackground());
        client = std::make_unique<httplib::Client>("127.0.0.1", server->port());
        client->set_connection_timeout(5, 0);
        client->set_read_timeout(120, 0);
    }

    void TearDown() override
    {
        client.reset();
        server->stop();
        server.reset();
        std::filesystem::remove_all(workspaceRoot);
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
    std::filesystem::path workspaceRoot;
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

TEST_F(WebApiIntegrationTest, SessionSaveLoadRoundTrip)
{
    const std::string sessionId = createSession();
    const httplib::Headers headers = sessionHeaders(sessionId);

    const std::string openBody = "{\"path\": \"" + sourcePath("samples/sample.log") + "\"}";
    ASSERT_TRUE(client->Post("/api/v1/sources/open", headers, openBody, "application/json"));
    ASSERT_TRUE(client->Post("/api/v1/analyze", headers, "{}", "application/json"));

    const httplib::Result exportBefore =
        client->Post("/api/v1/export", headers, "{\"format\": \"json\"}", "application/json");
    ASSERT_TRUE(exportBefore);
    EXPECT_EQ(200, exportBefore->status);

    const std::filesystem::path sessionFile =
        std::filesystem::temp_directory_path() / "logscope-web-session-test.session";
    std::filesystem::remove(sessionFile);

    const std::string saveBody = "{\"sessionFile\": \"" + sessionFile.generic_string() + "\"}";
    const httplib::Result saveResult = client->Post("/api/v1/sessions/save", headers, saveBody, "application/json");
    ASSERT_TRUE(saveResult);
    EXPECT_EQ(200, saveResult->status);
    EXPECT_TRUE(std::filesystem::exists(sessionFile));

    const std::string newSessionId = createSession();
    const httplib::Headers newHeaders = sessionHeaders(newSessionId);
    const std::string loadBody = "{\"path\": \"" + sessionFile.generic_string() + "\"}";
    const httplib::Result loadResult = client->Post("/api/v1/sessions/load", newHeaders, loadBody, "application/json");
    ASSERT_TRUE(loadResult);
    EXPECT_EQ(200, loadResult->status);

    const httplib::Result exportAfter =
        client->Post("/api/v1/export", newHeaders, "{\"format\": \"json\"}", "application/json");
    ASSERT_TRUE(exportAfter);
    EXPECT_EQ(200, exportAfter->status);
    EXPECT_NE(std::string::npos, exportAfter->body.find("\"totalLines\": 8"));
    EXPECT_NE(std::string::npos, exportAfter->body.find("\"errorLines\": 4"));
    EXPECT_NE(std::string::npos, exportBefore->body.find("\"totalLines\": 8"));
    EXPECT_NE(std::string::npos, exportBefore->body.find("\"errorLines\": 4"));

    std::filesystem::remove(sessionFile);
}

TEST_F(WebApiIntegrationTest, ExportHtmlAndPdf)
{
    const std::string sessionId = createSession();
    const httplib::Headers headers = sessionHeaders(sessionId);

    const std::string openBody = "{\"path\": \"" + sourcePath("samples/sample.log") + "\"}";
    ASSERT_TRUE(client->Post("/api/v1/sources/open", headers, openBody, "application/json"));
    ASSERT_TRUE(client->Post("/api/v1/analyze", headers, "{}", "application/json"));

    const httplib::Result htmlResult =
        client->Post("/api/v1/export", headers, "{\"format\": \"html\"}", "application/json");
    ASSERT_TRUE(htmlResult);
    EXPECT_EQ(200, htmlResult->status);
    EXPECT_NE(std::string::npos, htmlResult->body.find("<html"));

    const httplib::Result pdfResult =
        client->Post("/api/v1/export", headers, "{\"format\": \"pdf\"}", "application/json");
    ASSERT_TRUE(pdfResult);
    EXPECT_EQ(200, pdfResult->status);
    EXPECT_GE(pdfResult->body.size(), 4U);
    EXPECT_EQ('%', pdfResult->body[0]);
    EXPECT_EQ('P', pdfResult->body[1]);
    EXPECT_EQ('D', pdfResult->body[2]);
    EXPECT_EQ('F', pdfResult->body[3]);
}

TEST_F(WebApiIntegrationTest, AgentInvestigateAskErrors)
{
    const std::string sessionId = createSession();
    const httplib::Headers headers = sessionHeaders(sessionId);

    ASSERT_TRUE(client->Post("/api/v1/config/load", headers,
                             "{\"path\": \"" + sourcePath("samples/ai-noop.properties") + "\"}",
                             "application/json"));

    const std::string openBody = "{\"path\": \"" + sourcePath("samples/sample.log") + "\"}";
    ASSERT_TRUE(client->Post("/api/v1/sources/open", headers, openBody, "application/json"));
    ASSERT_TRUE(client->Post("/api/v1/analyze", headers, "{}", "application/json"));

    const httplib::Result result =
        client->Post("/api/v1/agent/investigate", headers, "{\"ask\": \"errors\"}", "application/json");
    ASSERT_TRUE(result);
    EXPECT_EQ(200, result->status);
    EXPECT_NE(std::string::npos, result->body.find("\"matchingLineCount\": 4"));
}

TEST_F(WebApiIntegrationTest, LargeAppAnalyzeSmoke)
{
    scope::web::WebConfig asyncConfig = config;
    asyncConfig.asyncAnalyzeThresholdBytes = 1U;
    restartServer(asyncConfig);

    const std::string sessionId = createSession();
    const httplib::Headers headers = sessionHeaders(sessionId);

    const std::string openBody = "{\"path\": \"" + sourcePath("samples/large-app.log") + "\"}";
    ASSERT_TRUE(client->Post("/api/v1/sources/open", headers, openBody, "application/json"));

    const auto start = std::chrono::steady_clock::now();
    const httplib::Result analyzeResult =
        client->Post("/api/v1/analyze", headers, "{}", "application/json");
    const auto elapsed = std::chrono::steady_clock::now() - start;

    ASSERT_TRUE(analyzeResult);
    EXPECT_EQ(202, analyzeResult->status);
    EXPECT_LT(elapsed, std::chrono::seconds(10));
    EXPECT_NE(std::string::npos, analyzeResult->body.find("\"jobId\""));

    const std::size_t jobPos = analyzeResult->body.find("\"jobId\": \"");
    ASSERT_NE(std::string::npos, jobPos);
    const std::size_t jobStart = jobPos + 10U;
    const std::size_t jobEnd = analyzeResult->body.find('"', jobStart);
    const std::string jobId = analyzeResult->body.substr(jobStart, jobEnd - jobStart);

    EXPECT_TRUE(pollAnalyzeJobUntilComplete(sessionId, jobId, 600));
}

TEST_F(WebApiIntegrationTest, SharedWorkspaceCreateOpenSaveFlow)
{
    const std::string sessionId = createSession();
    const httplib::Headers headers = sessionHeaders(sessionId);

    const std::string openBody = "{\"path\": \"" + sourcePath("samples/sample.log") + "\"}";
    ASSERT_TRUE(client->Post("/api/v1/sources/open", headers, openBody, "application/json"));
    ASSERT_TRUE(client->Post("/api/v1/analyze", headers, "{}", "application/json"));

    const httplib::Result createResult = client->Post("/api/v1/workspaces", headers,
                                                      "{\"name\": \"shared-incident\", \"captureSession\": true}",
                                                      "application/json");
    ASSERT_TRUE(createResult);
    EXPECT_EQ(200, createResult->status);
    EXPECT_NE(std::string::npos, createResult->body.find("\"name\": \"shared-incident\""));

    const httplib::Result listResult = client->Get("/api/v1/workspaces", headers);
    ASSERT_TRUE(listResult);
    EXPECT_EQ(200, listResult->status);
    EXPECT_NE(std::string::npos, listResult->body.find("shared-incident"));

    const std::string newSessionId = createSession();
    const httplib::Headers newHeaders = sessionHeaders(newSessionId);

    const std::size_t idPos = createResult->body.find("\"id\": \"");
    ASSERT_NE(std::string::npos, idPos);
    const std::size_t idStart = idPos + 7U;
    const std::size_t idEnd = createResult->body.find('"', idStart);
    const std::string workspaceId = createResult->body.substr(idStart, idEnd - idStart);

    const httplib::Result openWorkspace =
        client->Post("/api/v1/workspaces/" + workspaceId + "/open", newHeaders, "", "application/json");
    ASSERT_TRUE(openWorkspace);
    EXPECT_EQ(200, openWorkspace->status);
    EXPECT_NE(std::string::npos, openWorkspace->body.find("\"opened\": true"));

    const httplib::Result investigateResult =
        client->Post("/api/v1/investigate", newHeaders, "{\"search\": \"error\"}", "application/json");
    ASSERT_TRUE(investigateResult);
    EXPECT_EQ(200, investigateResult->status);

    const std::string saveBody = "{\"workspaceId\": \"" + workspaceId + "\"}";
    const httplib::Result saveResult = client->Post("/api/v1/sessions/save", headers, saveBody, "application/json");
    ASSERT_TRUE(saveResult);
    EXPECT_EQ(200, saveResult->status);
}

TEST_F(WebApiIntegrationTest, AsyncAnalyzeReturnsAcceptedAndCompletes)
{
    scope::web::WebConfig asyncConfig = config;
    asyncConfig.asyncAnalyzeThresholdBytes = 1U;
    restartServer(asyncConfig);

    const std::string sessionId = createSession();
    const httplib::Headers headers = sessionHeaders(sessionId);

    const std::string openBody = "{\"path\": \"" + sourcePath("samples/sample.log") + "\"}";
    ASSERT_TRUE(client->Post("/api/v1/sources/open", headers, openBody, "application/json"));

    const httplib::Result analyzeResult = client->Post("/api/v1/analyze", headers, "{}", "application/json");
    ASSERT_TRUE(analyzeResult);
    EXPECT_EQ(202, analyzeResult->status);
    EXPECT_NE(std::string::npos, analyzeResult->body.find("\"jobId\""));

    const std::size_t jobPos = analyzeResult->body.find("\"jobId\": \"");
    ASSERT_NE(std::string::npos, jobPos);
    const std::size_t jobStart = jobPos + 10U;
    const std::size_t jobEnd = analyzeResult->body.find('"', jobStart);
    const std::string jobId = analyzeResult->body.substr(jobStart, jobEnd - jobStart);

    EXPECT_TRUE(pollAnalyzeJobUntilComplete(sessionId, jobId));
}

namespace
{

class WebApiKeyIntegrationTest : public WebApiIntegrationTest
{
  protected:
    void SetUp() override
    {
        config = scope::web::WebConfig::defaults();
        config.bindPort = 0;
        config.apiKey = "integration-secret";
        server = std::make_unique<scope::web::WebServer>(config);
        ASSERT_TRUE(server->startInBackground());
        client = std::make_unique<httplib::Client>("127.0.0.1", server->port());
        client->set_connection_timeout(2, 0);
    }
};

} // namespace

TEST_F(WebApiKeyIntegrationTest, ApiKeyRequiredOnMutatingRoutes)
{
    const httplib::Result missingKey = client->Post("/api/v1/sessions/workspace");
    ASSERT_TRUE(missingKey);
    EXPECT_EQ(401, missingKey->status);

    httplib::Headers headers;
    headers.emplace(scope::web::kApiKeyHeader, "integration-secret");
    const httplib::Result withKey = client->Post("/api/v1/sessions/workspace", headers);
    ASSERT_TRUE(withKey);
    EXPECT_EQ(200, withKey->status);
}
