/**
 * @file configuration_test.cpp
 * @brief Unit tests for Configuration.
 */

#include <gtest/gtest.h>

#include "runtime.hpp"

using scope::foundation::ErrorCode;
using scope::runtime::Configuration;

TEST(ConfigurationTest, SetAndGet)
{
    Configuration configuration;

    configuration.set("log.level", "info");

    auto result = configuration.get("log.level");

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ("info", *result);
}

TEST(ConfigurationTest, Has)
{
    Configuration configuration;

    configuration.set("key", "value");

    EXPECT_TRUE(configuration.has("key"));
    EXPECT_FALSE(configuration.has("missing"));
}

TEST(ConfigurationTest, MissingKey)
{
    Configuration configuration;

    auto result = configuration.get("missing");

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::InvalidArgument, result.error().code());
}
