/**
 * @file health_handler_test.cpp
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

class HealthHandlerTest : public ::testing::Test
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

        FAIL() << "WebServer did not become ready on port " << server->port();
    }

    void TearDown() override
    {
        if (server != nullptr)
        {
            server->stop();
        }
    }

    scope::web::WebConfig config;
    std::unique_ptr<scope::web::WebServer> server;
};

} // namespace

TEST_F(HealthHandlerTest, ReturnsVersionAndSessionCount)
{
    httplib::Client client("127.0.0.1", server->port());
    client.set_connection_timeout(2, 0);
    const httplib::Result result = client.Get("/api/v1/health");

    ASSERT_TRUE(result);
    EXPECT_EQ(200, result->status);
    EXPECT_NE(std::string::npos, result->body.find("\"version\": \"2."));
    EXPECT_NE(std::string::npos, result->body.find("\"sessionCount\": 0"));
    EXPECT_NE(std::string::npos, result->body.find("\"uptimeSeconds\":"));
}

TEST_F(HealthHandlerTest, WorkspaceCreateReturnsSessionHeader)
{
    httplib::Client client("127.0.0.1", server->port());
    client.set_connection_timeout(2, 0);
    const httplib::Result result = client.Post("/api/v1/sessions/workspace");

    ASSERT_TRUE(result);
    EXPECT_EQ(200, result->status);
    EXPECT_NE(result->headers.find(scope::web::kSessionHeader), result->headers.end());
    EXPECT_NE(std::string::npos, result->body.find("\"sessionId\""));
}
