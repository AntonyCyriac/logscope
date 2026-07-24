/**
 * @file message_signature_test.cpp
 */

#include <gtest/gtest.h>

#include "message_signature.hpp"

TEST(MessageSignatureTest, NormalizesDigitsToPlaceholder)
{
    EXPECT_EQ("Error code=N", scope::analytics::normalizeClusterSignature("Error code=500"));
    EXPECT_EQ("Error code=N", scope::analytics::normalizeClusterSignature("Error code=503"));
}

TEST(MessageSignatureTest, TrimsWhitespace)
{
    EXPECT_EQ("Connection refused", scope::analytics::normalizeClusterSignature("  Connection refused  "));
}
