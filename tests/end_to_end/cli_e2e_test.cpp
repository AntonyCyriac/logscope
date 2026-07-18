/**
 * @file cli_e2e_test.cpp
 * @brief End-to-end tests for the LogScope CLI executable.
 */

#include <gtest/gtest.h>

#include <string>

#include "process.hpp"

namespace
{

std::string logscopeExecutable()
{
    return LOGSCOPE_EXECUTABLE;
}

std::string sourcePath(const std::string& relativePath)
{
    return std::string(LOGSCOPE_SOURCE_DIR) + "/" + relativePath;
}

std::string runLogscope(const std::string& arguments)
{
#if defined(_WIN32)
    const std::string command = "cmd /c " +
                                scope::test_support::quoteArgument(scope::test_support::quoteArgument(logscopeExecutable()) +
                                                                   " " + arguments + " 2>&1");
#else
    const std::string command =
        scope::test_support::quoteArgument(logscopeExecutable()) + " " + arguments + " 2>&1";
#endif

    return scope::test_support::captureCommandOutput(command);
}

} // namespace

TEST(CliE2eTest, LegacyAnalyzeProducesTextReport)
{
    const std::string output = runLogscope(scope::test_support::quoteArgument(sourcePath("samples/sample.log")));

    EXPECT_NE(std::string::npos, output.find("========== LOGSCOPE REPORT =========="));
    EXPECT_NE(std::string::npos, output.find("Total log lines : 8"));
    EXPECT_NE(std::string::npos, output.find("Error lines     : 4"));
}

TEST(CliE2eTest, AnalyzeSubcommandProducesJsonReport)
{
    const std::string output =
        runLogscope("analyze --format json " + scope::test_support::quoteArgument(sourcePath("samples/sample.log")));

    EXPECT_NE(std::string::npos, output.find("\"totalLines\": 8"));
    EXPECT_NE(std::string::npos, output.find("\"errorLines\": 4"));
    EXPECT_NE(std::string::npos, output.find("\"warningLines\": 1"));
    EXPECT_NE(std::string::npos, output.find("\"infoLines\": 3"));
    EXPECT_NE(std::string::npos, output.find("\"source\""));
}

TEST(CliE2eTest, ConfigValidateSucceedsForSampleConfiguration)
{
    const std::string output = runLogscope("config validate --config " +
                                           scope::test_support::quoteArgument(sourcePath("samples/logscope.properties")) +
                                           " --require log.level");

    EXPECT_NE(std::string::npos, output.find("Configuration is valid."));
}

TEST(CliE2eTest, HelpDisplaysUsage)
{
    const std::string output = runLogscope("--help");

    EXPECT_NE(std::string::npos, output.find("Commands:"));
    EXPECT_NE(std::string::npos, output.find("analyze"));
    EXPECT_NE(std::string::npos, output.find("config validate"));
}
