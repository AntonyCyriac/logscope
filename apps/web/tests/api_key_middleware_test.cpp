/**
 * @file api_key_middleware_test.cpp
 */

#include <gtest/gtest.h>
#include <httplib.h>

#include "middleware/api_key.hpp"
#include "middleware/api_key_credential.hpp"
#include "rest_json.hpp"

TEST(ApiKeyMiddlewareTest, AllowsRequestWhenKeyNotConfigured)
{
    httplib::Request request;
    httplib::Response response;

    EXPECT_TRUE(scope::web::authorizeApiKey(scope::web::ApiKeyCredential::disabled(), request, response));
}

TEST(ApiKeyMiddlewareTest, RejectsMissingKeyWhenConfigured)
{
    httplib::Request request;
    httplib::Response response;

    EXPECT_FALSE(scope::web::authorizeApiKey(scope::web::ApiKeyCredential::fromPlaintext("secret"), request, response));
    EXPECT_EQ(401, response.status);
    EXPECT_NE(std::string::npos, response.body.find("UNAUTHORIZED"));
}

TEST(ApiKeyMiddlewareTest, AcceptsMatchingKey)
{
    httplib::Request request;
    request.headers.emplace(scope::web::kApiKeyHeader, "secret");
    httplib::Response response;

    EXPECT_TRUE(scope::web::authorizeApiKey(scope::web::ApiKeyCredential::fromPlaintext("secret"), request, response));
}

TEST(ApiKeyMiddlewareTest, AcceptsMatchingHashedKey)
{
    const auto stored = scope::web::ApiKeyCredential::hashPlaintextForStorage("secret");

    ASSERT_TRUE(stored.hasValue()) << stored.error().message();

    const auto credential = scope::web::ApiKeyCredential::fromStoredHash(*stored);

    ASSERT_TRUE(credential.hasValue()) << credential.error().message();

    httplib::Request request;
    request.headers.emplace(scope::web::kApiKeyHeader, "secret");
    httplib::Response response;

    EXPECT_TRUE(scope::web::authorizeApiKey(*credential, request, response));
}
