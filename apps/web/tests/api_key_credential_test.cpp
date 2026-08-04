/**
 * @file api_key_credential_test.cpp
 */

#include <gtest/gtest.h>

#include "middleware/api_key_credential.hpp"

TEST(ApiKeyCredentialTest, HashRoundTripVerifiesPresentedKey)
{
    const auto stored = scope::web::ApiKeyCredential::hashPlaintextForStorage("integration-secret");

    ASSERT_TRUE(stored.hasValue()) << stored.error().message();

    const auto credential = scope::web::ApiKeyCredential::fromStoredHash(*stored);

    ASSERT_TRUE(credential.hasValue()) << credential.error().message();
    EXPECT_TRUE(credential->verify("integration-secret"));
    EXPECT_FALSE(credential->verify("wrong-secret"));
}

TEST(ApiKeyCredentialTest, PlaintextCredentialStillWorks)
{
    const scope::web::ApiKeyCredential credential = scope::web::ApiKeyCredential::fromPlaintext("legacy-key");

    EXPECT_TRUE(credential.verify("legacy-key"));
    EXPECT_FALSE(credential.verify("other-key"));
    EXPECT_FALSE(credential.isPlaintextInConfig());
}

TEST(ApiKeyCredentialTest, PlaintextInConfigFlag)
{
    const scope::web::ApiKeyCredential credential = scope::web::ApiKeyCredential::fromPlaintextInConfig("legacy-key");

    EXPECT_TRUE(credential.isPlaintextInConfig());
}

TEST(ApiKeyCredentialTest, RejectsMalformedHash)
{
    const auto credential = scope::web::ApiKeyCredential::fromStoredHash("not-a-hash");

    EXPECT_FALSE(credential.hasValue());
}
