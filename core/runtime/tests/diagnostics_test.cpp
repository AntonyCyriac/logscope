/**
 * @file diagnostics_test.cpp
 * @brief Unit tests for Diagnostics.
 */

#include <sstream>

#include <gtest/gtest.h>

#include "runtime.hpp"

using scope::foundation::ErrorCode;
using scope::runtime::Configuration;
using scope::runtime::Diagnostics;
using scope::runtime::LogLevel;
using scope::runtime::parseLogLevel;

namespace
{

class DiagnosticsTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        m_outputStream.str("");
        m_outputStream.clear();

        Diagnostics& diagnostics = Diagnostics::instance();
        diagnostics.setMinLevel(LogLevel::Info);
        diagnostics.setTimestampsEnabled(true);
        diagnostics.setOutputStream(&m_outputStream);
    }

    void TearDown() override
    {
        Diagnostics::instance().setOutputStream(nullptr);
        Diagnostics::instance().setMinLevel(LogLevel::Info);
        Diagnostics::instance().setTimestampsEnabled(true);
    }

    [[nodiscard]] std::string output() const
    {
        return m_outputStream.str();
    }

  private:
    std::ostringstream m_outputStream;
};

} // namespace

TEST_F(DiagnosticsTest, Instance)
{
    Diagnostics& first = Diagnostics::instance();
    Diagnostics& second = Diagnostics::instance();

    EXPECT_EQ(&first, &second);
}

TEST_F(DiagnosticsTest, LogDoesNotThrow)
{
    EXPECT_NO_THROW(Diagnostics::instance().log(LogLevel::Info, "test message"));
}

TEST_F(DiagnosticsTest, MinLevelFiltersDebug)
{
    Diagnostics::instance().log(LogLevel::Debug, "cli", "hidden message");

    EXPECT_TRUE(output().empty());
}

TEST_F(DiagnosticsTest, MinLevelAllowsError)
{
    Diagnostics::instance().log(LogLevel::Error, "cli", "visible error");

    EXPECT_NE(output().find("[ERROR]"), std::string::npos);
    EXPECT_NE(output().find("[cli]"), std::string::npos);
    EXPECT_NE(output().find("visible error"), std::string::npos);
}

TEST_F(DiagnosticsTest, CategoryInOutput)
{
    Diagnostics::instance().log(LogLevel::Info, "cli", "starting analysis");

    EXPECT_NE(output().find("[INFO]"), std::string::npos);
    EXPECT_NE(output().find("[cli]"), std::string::npos);
    EXPECT_NE(output().find("starting analysis"), std::string::npos);
}

TEST_F(DiagnosticsTest, TimestampInOutput)
{
    Diagnostics::instance().setTimestampsEnabled(true);
    Diagnostics::instance().log(LogLevel::Info, "cli", "timestamped message");

    const std::string line = output();

    EXPECT_NE(line.find("T"), std::string::npos);
    EXPECT_NE(line.find("[INFO]"), std::string::npos);
    EXPECT_LT(line.find('T'), line.find("[INFO]"));
}

TEST_F(DiagnosticsTest, TimestampCanBeDisabled)
{
    Diagnostics::instance().setTimestampsEnabled(false);
    Diagnostics::instance().log(LogLevel::Info, "cli", "plain message");

    EXPECT_EQ(output().find("[INFO]"), 0U);
}

TEST_F(DiagnosticsTest, ApplyConfigurationTimestamps)
{
    Configuration configuration;
    configuration.set("log.timestamps", "false");

    EXPECT_TRUE(Diagnostics::instance().applyConfiguration(configuration));
    EXPECT_FALSE(Diagnostics::instance().timestampsEnabled());
}

TEST_F(DiagnosticsTest, ApplyConfigurationInvalidTimestamps)
{
    Configuration configuration;
    configuration.set("log.timestamps", "maybe");

    EXPECT_FALSE(Diagnostics::instance().applyConfiguration(configuration));
    EXPECT_TRUE(Diagnostics::instance().timestampsEnabled());
}

TEST(ParseLogLevelTest, ValidValues)
{
    const auto debug = parseLogLevel("debug");
    const auto info = parseLogLevel("INFO");
    const auto warn = parseLogLevel("Warn");
    const auto warning = parseLogLevel("warning");
    const auto error = parseLogLevel("error");

    ASSERT_TRUE(debug);
    ASSERT_TRUE(info);
    ASSERT_TRUE(warn);
    ASSERT_TRUE(warning);
    ASSERT_TRUE(error);

    EXPECT_EQ(*debug, LogLevel::Debug);
    EXPECT_EQ(*info, LogLevel::Info);
    EXPECT_EQ(*warn, LogLevel::Warning);
    EXPECT_EQ(*warning, LogLevel::Warning);
    EXPECT_EQ(*error, LogLevel::Error);
}

TEST(ParseLogLevelTest, InvalidValue)
{
    const auto result = parseLogLevel("trace");

    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code(), ErrorCode::InvalidArgument);
}

TEST_F(DiagnosticsTest, ApplyConfiguration)
{
    Configuration configuration;
    configuration.set("log.level", "debug");

    EXPECT_TRUE(Diagnostics::instance().applyConfiguration(configuration));
    EXPECT_EQ(Diagnostics::instance().minLevel(), LogLevel::Debug);

    Diagnostics::instance().log(LogLevel::Debug, "cli", "debug enabled");

    EXPECT_NE(output().find("debug enabled"), std::string::npos);
}

TEST_F(DiagnosticsTest, ApplyConfigurationInvalid)
{
    Configuration configuration;
    configuration.set("log.level", "trace");

    EXPECT_FALSE(Diagnostics::instance().applyConfiguration(configuration));
    EXPECT_EQ(Diagnostics::instance().minLevel(), LogLevel::Info);
}
