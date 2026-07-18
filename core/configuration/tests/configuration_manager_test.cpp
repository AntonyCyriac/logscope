/**
 * @file configuration_manager_test.cpp
 * @brief Unit tests for ConfigurationManager.
 */

#include <cstdlib>
#include <fstream>

#if defined(_WIN32)
#include <stdlib.h>
#endif

#include <gtest/gtest.h>

#include "configuration.hpp"

using scope::configuration::ConfigurationManager;
using scope::configuration::environmentVariableToConfigKey;
using scope::foundation::ErrorCode;
using scope::foundation::Path;

namespace
{

class ConfigurationManagerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        m_configFile = Path("configuration_manager_test.properties");

        std::ofstream stream(m_configFile.string());

        stream << "# LogScope configuration\n";
        stream << "log.level=debug\n";
        stream << "log.timestamps=false\n";
        stream << "\n";
        stream << "app.name=logscope\n";
    }

    void TearDown() override
    {
        std::remove(m_configFile.string().c_str());
    }

    Path m_configFile;
};

} // namespace

TEST(EnvironmentMappingTest, ConvertsScopePrefix)
{
    EXPECT_EQ("log.level", environmentVariableToConfigKey("SCOPE_LOG_LEVEL"));
    EXPECT_EQ("log.timestamps", environmentVariableToConfigKey("SCOPE_LOG_TIMESTAMPS"));
}

TEST(EnvironmentMappingTest, RejectsNonScopePrefix)
{
    EXPECT_TRUE(environmentVariableToConfigKey("LOG_LEVEL").empty());
    EXPECT_TRUE(environmentVariableToConfigKey("PATH").empty());
}

TEST_F(ConfigurationManagerTest, LoadFromFile)
{
    auto result = ConfigurationManager::loadFromFile(m_configFile);

    ASSERT_TRUE(result.hasValue());

    const auto& configuration = result->configuration();

    ASSERT_TRUE(configuration.has("log.level"));
    EXPECT_EQ("debug", *configuration.get("log.level"));

    ASSERT_TRUE(configuration.has("log.timestamps"));
    EXPECT_EQ("false", *configuration.get("log.timestamps"));

    ASSERT_TRUE(configuration.has("app.name"));
    EXPECT_EQ("logscope", *configuration.get("app.name"));
}

TEST_F(ConfigurationManagerTest, LoadMissingFile)
{
    auto result = ConfigurationManager::loadFromFile(Path("missing_config.properties"));

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::FileNotFound, result.error().code());
}

TEST_F(ConfigurationManagerTest, LoadInvalidLine)
{
    const Path invalidFile("configuration_manager_invalid_test.properties");

    {
        std::ofstream stream(invalidFile.string(), std::ios::trunc);

        stream << "invalid-line-without-separator\n";
    }

    auto result = ConfigurationManager::loadFromFile(invalidFile);

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::ParseError, result.error().code());

    std::remove(invalidFile.string().c_str());
}

TEST_F(ConfigurationManagerTest, ValidateRequiredKeys)
{
    auto loadResult = ConfigurationManager::loadFromFile(m_configFile);

    ASSERT_TRUE(loadResult.hasValue());

    auto validResult = loadResult->validate({"log.level", "app.name"});

    ASSERT_TRUE(validResult.hasValue());
    EXPECT_TRUE(*validResult);
}

TEST_F(ConfigurationManagerTest, ValidateMissingKeys)
{
    auto loadResult = ConfigurationManager::loadFromFile(m_configFile);

    ASSERT_TRUE(loadResult.hasValue());

    auto validResult = loadResult->validate({"log.level", "missing.key"});

    ASSERT_TRUE(validResult.hasError());
    EXPECT_EQ(ErrorCode::InvalidArgument, validResult.error().code());
}

TEST_F(ConfigurationManagerTest, ApplyEnvironmentOverride)
{
#if defined(_WIN32)
    _putenv_s("SCOPE_LOG_LEVEL", "error");
#else
    setenv("SCOPE_LOG_LEVEL", "error", 1);
#endif

    ConfigurationManager manager;

    manager.applyEnvironment();

    ASSERT_TRUE(manager.configuration().has("log.level"));
    EXPECT_EQ("error", *manager.configuration().get("log.level"));

#if defined(_WIN32)
    _putenv_s("SCOPE_LOG_LEVEL", "");
#else
    unsetenv("SCOPE_LOG_LEVEL");
#endif
}
