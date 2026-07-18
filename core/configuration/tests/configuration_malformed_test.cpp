/**
 * @file configuration_malformed_test.cpp
 * @brief Malformed-input tests for ConfigurationManager.
 */

#include <fstream>

#include <gtest/gtest.h>

#include "configuration_manager.hpp"

using scope::configuration::ConfigurationManager;
using scope::foundation::Path;

namespace
{

void writeConfigFile(const Path& path, const std::string& content)
{
    std::ofstream stream(path.string());
    stream << content;
}

} // namespace

TEST(ConfigurationMalformedTest, RejectsMissingEqualsSign)
{
    const Path configFile("configuration_malformed_missing_equals.properties");
    writeConfigFile(configFile, "invalid line without separator\n");

    const auto result = ConfigurationManager::loadFromFile(configFile);

    EXPECT_FALSE(result.hasValue());
    std::remove(configFile.string().c_str());
}

TEST(ConfigurationMalformedTest, RejectsEmptyKey)
{
    const Path configFile("configuration_malformed_empty_key.properties");
    writeConfigFile(configFile, "=value\n");

    const auto result = ConfigurationManager::loadFromFile(configFile);

    EXPECT_FALSE(result.hasValue());
    std::remove(configFile.string().c_str());
}

TEST(ConfigurationMalformedTest, IgnoresCommentsAndBlankLines)
{
    const Path configFile("configuration_malformed_comments.properties");
    writeConfigFile(configFile,
                     "# comment\n"
                     "\n"
                     "log.level=info\n");

    const auto result = ConfigurationManager::loadFromFile(configFile);

    ASSERT_TRUE(result.hasValue());
    const auto level = result->configuration().get("log.level");
    ASSERT_TRUE(level.hasValue());
    EXPECT_EQ("info", *level);
    std::remove(configFile.string().c_str());
}

TEST(ConfigurationMalformedTest, RejectsNonexistentFile)
{
    const Path configFile("configuration_malformed_missing.properties");

    const auto result = ConfigurationManager::loadFromFile(configFile);

    EXPECT_FALSE(result.hasValue());
}
