/**
 * @file upload_size_limit_test.cpp
 * @brief Upload size guard tests (M15 L1.8).
 */

#include <gtest/gtest.h>
#include <httplib.h>

#include <chrono>
#include <string>
#include <thread>

#include "middleware/api_key.hpp"
#include "web_config.hpp"
#include "web_server.hpp"

namespace
{

class UploadSizeLimitTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        config = scope::web::WebConfig::defaults();
        config.bindPort = 0;
        config.maxUploadBytes = 64U;
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

    scope::web::WebConfig config;
    std::unique_ptr<scope::web::WebServer> server;
};

} // namespace

TEST_F(UploadSizeLimitTest, RejectsOversizedUpload)
{
    httplib::Client client("127.0.0.1", server->port());
    ASSERT_TRUE(client.Post("/api/v1/sessions/workspace"));

    const std::string payload(128U, 'x');
    httplib::MultipartFormDataItems items = {
        {"file", payload, "big.log", "text/plain"},
    };

    const httplib::Result result = client.Post("/api/v1/sources/upload", items);

    ASSERT_TRUE(result);
    EXPECT_EQ(413, result->status);
    EXPECT_NE(std::string::npos, result->body.find("PAYLOAD_TOO_LARGE"));
}
