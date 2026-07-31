/**
 * @file api_key_middleware_test.cpp
 */

#include <gtest/gtest.h>
#include <httplib.h>

#include "middleware/api_key.hpp"
#include "rest_json.hpp"

TEST(ApiKeyMiddlewareTest, AllowsRequestWhenKeyNotConfigured)
{
    httplib::Request request;
    httplib::Response response;

    EXPECT_TRUE(scope::web::authorizeApiKey("", request, response));
}

TEST(ApiKeyMiddlewareTest, RejectsMissingKeyWhenConfigured)
{
    httplib::Request request;
    httplib::Response response;

    EXPECT_FALSE(scope::web::authorizeApiKey("secret", request, response));
    EXPECT_EQ(401, response.status);
    EXPECT_NE(std::string::npos, response.body.find("UNAUTHORIZED"));
}

TEST(ApiKeyMiddlewareTest, AcceptsMatchingKey)
{
    httplib::Request request;
    request.headers.emplace(scope::web::kApiKeyHeader, "secret");
    httplib::Response response;

    EXPECT_TRUE(scope::web::authorizeApiKey("secret", request, response));
}
