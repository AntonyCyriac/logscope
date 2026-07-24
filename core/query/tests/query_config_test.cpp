/**
 * @file query_config_test.cpp
 */

#include <gtest/gtest.h>

#include "query_config.hpp"
#include "runtime/configuration.hpp"

using scope::query::savedFilterKeyPrefix;
using scope::query::validateQueryConfiguration;
using scope::runtime::Configuration;

TEST(QueryConfigTest, ValidatesSavedFilterKeys)
{
    Configuration configuration;
    configuration.set(std::string(savedFilterKeyPrefix) + "errors", "level == ERROR");

    const auto result = validateQueryConfiguration(configuration);

    ASSERT_TRUE(result);
}

TEST(QueryConfigTest, RejectsInvalidSavedFilter)
{
    Configuration configuration;
    configuration.set(std::string(savedFilterKeyPrefix) + "broken", "level ==");

    const auto result = validateQueryConfiguration(configuration);

    EXPECT_FALSE(result);
}
