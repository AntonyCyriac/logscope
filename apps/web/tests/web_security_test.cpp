/**
 * @file web_security_test.cpp
 * @brief Security scenario tests (M15 S1).
 */

#include <gtest/gtest.h>
#include <httplib.h>

#include <chrono>
#include <thread>

#include "middleware/api_key.hpp"
#include "web_config.hpp"
#include "web_server.hpp"

namespace
{

class WebSecurityTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        config = scope::web::WebConfig::defaults();
        config.bindPort = 0;
        server = std::make_unique<scope::web::WebServer>(config);
        ASSERT_TRUE(server->startInBackground());

        for (int attempt = 0; attempt < 50; ++attempt)
        {
            httplib::Client probe("127.0.0.1", server->port());
            probe.set_connection_timeout(0, 200000);

            if (const httplib::Result probeResult = probe.Get("/api/v1/health"))
            {
                return;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        FAIL() << "WebServer did not become ready";
    }

    void TearDown() override
    {
        if (server != nullptr)
        {
            server->stop();
        }
    }

    std::string createSession()
    {
        httplib::Client client("127.0.0.1", server->port());
        const httplib::Result result = client.Post("/api/v1/sessions/workspace");

        EXPECT_TRUE(result);
        EXPECT_EQ(200, result->status);

        const auto header = result->headers.find(scope::web::kSessionHeader);

        EXPECT_NE(header, result->headers.end());

        return header->second;
    }

    scope::web::WebConfig config;
    std::unique_ptr<scope::web::WebServer> server;
};

} // namespace

TEST(WebConfigTest, DefaultBindHostIsLoopback)
{
    const scope::web::WebConfig defaults = scope::web::WebConfig::defaults();

    EXPECT_EQ("127.0.0.1", defaults.bindHost);
}

TEST_F(WebSecurityTest, RejectsServerPathWhenDisabled)
{
    const std::string sessionId = createSession();
    httplib::Headers headers;
    headers.emplace(scope::web::kSessionHeader, sessionId);

    httplib::Client client("127.0.0.1", server->port());
    const httplib::Result result =
        client.Post("/api/v1/sources/open", headers, "{\"path\": \"samples/sample.log\"}", "application/json");

    ASSERT_TRUE(result);
    EXPECT_EQ(403, result->status);
    EXPECT_NE(std::string::npos, result->body.find("FORBIDDEN"));
}

TEST_F(WebSecurityTest, HealthAccessibleWithoutApiKeyWhenConfigured)
{
    server->stop();
    config.apiKey = "secret-key";
    server = std::make_unique<scope::web::WebServer>(config);
    ASSERT_TRUE(server->startInBackground());

    httplib::Client client("127.0.0.1", server->port());
    const httplib::Result health = client.Get("/api/v1/health");
    ASSERT_TRUE(health);
    EXPECT_EQ(200, health->status);

    const httplib::Result workspace = client.Post("/api/v1/sessions/workspace");
    ASSERT_TRUE(workspace);
    EXPECT_EQ(401, workspace->status);
}

TEST_F(WebSecurityTest, SessionIsolationDeniesUnknownSessionInvestigate)
{
    server->stop();
    config.allowServerPaths = true;
    config.allowedPathRoots = {scope::foundation::Path(std::string(LOGSCOPE_SOURCE_DIR) + "/samples")};
    server = std::make_unique<scope::web::WebServer>(config);
    ASSERT_TRUE(server->startInBackground());

    const std::string sessionA = createSession();
    httplib::Headers headersA;
    headersA.emplace(scope::web::kSessionHeader, sessionA);

    httplib::Client client("127.0.0.1", server->port());
    const std::string openBody =
        std::string("{\"path\": \"") + std::string(LOGSCOPE_SOURCE_DIR) + "/samples/sample.log\"}";
    ASSERT_TRUE(client.Post("/api/v1/sources/open", headersA, openBody, "application/json"));
    ASSERT_TRUE(client.Post("/api/v1/analyze", headersA, "{}", "application/json"));

    httplib::Headers headersB;
    headersB.emplace(scope::web::kSessionHeader, sessionA + "-other");
    const httplib::Result investigate =
        client.Post("/api/v1/investigate", headersB, "{\"search\": \"error\"}", "application/json");

    ASSERT_TRUE(investigate);
    EXPECT_EQ(400, investigate->status);
}

TEST_F(WebSecurityTest, SpaShellServedAtRoot)
{
    httplib::Client client("127.0.0.1", server->port());
    const httplib::Result result = client.Get("/");

    ASSERT_TRUE(result);
    EXPECT_EQ(200, result->status);
    EXPECT_NE(std::string::npos, result->body.find("LogScope Web"));
    EXPECT_NE(std::string::npos, result->body.find("app.js"));
}
