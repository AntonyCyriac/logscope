/**
 * @file tail_handler_test.cpp
 * @brief Unit tests for tail REST behavior (M15.3 / T1).
 */

#include <gtest/gtest.h>
#include <httplib.h>

#include <chrono>
#include <fstream>
#include <thread>

#include "middleware/api_key.hpp"
#include "web_config.hpp"
#include "web_server.hpp"

namespace
{

class TailHandlerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        tempLog = std::filesystem::temp_directory_path() / "logscope-tail-handler-test.log";
        std::ofstream stream(tempLog);
        stream << "line-one\n";

        config = scope::web::WebConfig::defaults();
        config.bindPort = 0;
        config.allowServerPaths = true;
        config.allowedPathRoots = {scope::foundation::Path(tempLog.parent_path().string())};
        server = std::make_unique<scope::web::WebServer>(config);
        ASSERT_TRUE(server->startInBackground());
        client = std::make_unique<httplib::Client>("127.0.0.1", server->port());

        const httplib::Result sessionResult = client->Post("/api/v1/sessions/workspace");
        ASSERT_TRUE(sessionResult);
        sessionId = sessionResult->get_header_value(scope::web::kSessionHeader);
        headers.emplace(scope::web::kSessionHeader, sessionId);

        const std::string openBody = "{\"path\": \"" + tempLog.string() + "\"}";
        ASSERT_TRUE(client->Post("/api/v1/sources/open", headers, openBody, "application/json"));
    }

    void TearDown() override
    {
        client.reset();
        server->stop();
        server.reset();
        std::filesystem::remove(tempLog);
    }

    httplib::Headers headers;
    std::string sessionId;
    std::filesystem::path tempLog;
    scope::web::WebConfig config;
    std::unique_ptr<scope::web::WebServer> server;
    std::unique_ptr<httplib::Client> client;
};

} // namespace

TEST_F(TailHandlerTest, PollWithoutStartReturns409)
{
    const httplib::Result pollResult = client->Get("/api/v1/tail/poll", headers);

    ASSERT_TRUE(pollResult);
    EXPECT_EQ(409, pollResult->status);
    EXPECT_NE(std::string::npos, pollResult->body.find("Tail is not active"));
}

TEST_F(TailHandlerTest, DoubleStartReturns409)
{
    ASSERT_TRUE(client->Post("/api/v1/tail/start", headers, "", "application/json"));
    const httplib::Result secondStart = client->Post("/api/v1/tail/start", headers, "", "application/json");

    ASSERT_TRUE(secondStart);
    EXPECT_EQ(409, secondStart->status);
}

TEST_F(TailHandlerTest, StartPollStopLifecycle)
{
    ASSERT_TRUE(client->Post("/api/v1/tail/start", headers, "", "application/json"));

    {
        std::ofstream stream(tempLog, std::ios::app);
        stream << "line-two\n";
    }

    const httplib::Result pollResult = client->Get("/api/v1/tail/poll", headers);
    ASSERT_TRUE(pollResult);
    EXPECT_EQ(200, pollResult->status);
    EXPECT_NE(std::string::npos, pollResult->body.find("line-two"));

    ASSERT_TRUE(client->Post("/api/v1/tail/stop", headers, "", "application/json"));
    const httplib::Result pollAfterStop = client->Get("/api/v1/tail/poll", headers);
    ASSERT_TRUE(pollAfterStop);
    EXPECT_EQ(409, pollAfterStop->status);
}
