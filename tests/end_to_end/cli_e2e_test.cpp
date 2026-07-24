/**
 * @file cli_e2e_test.cpp
 * @brief End-to-end tests for the LogScope CLI executable.
 */

#include <fstream>

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

    EXPECT_NE(std::string::npos, output.find("\"summary\""));
    EXPECT_NE(std::string::npos, output.find("\"totalLines\": 8"));
    EXPECT_NE(std::string::npos, output.find("\"errorLines\": 4"));
    EXPECT_NE(std::string::npos, output.find("\"warningLines\": 1"));
    EXPECT_NE(std::string::npos, output.find("\"infoLines\": 3"));
    EXPECT_NE(std::string::npos, output.find("\"sourceMetadata\""));
}

TEST(CliE2eTest, AnalyzeSubcommandProducesCsvReport)
{
    const std::string output =
        runLogscope("analyze --format csv " + scope::test_support::quoteArgument(sourcePath("samples/sample.log")));

    EXPECT_NE(std::string::npos, output.find("section,key,value"));
    EXPECT_NE(std::string::npos, output.find("summary,totalLines,8"));
    EXPECT_NE(std::string::npos, output.find("levelBreakdown,errorLines,4"));
}

TEST(CliE2eTest, AnalyzeSubcommandProducesMarkdownReport)
{
    const std::string output = runLogscope("analyze --format markdown --sections summary,levels " +
                                           scope::test_support::quoteArgument(sourcePath("samples/sample.log")));

    EXPECT_NE(std::string::npos, output.find("# LogScope Report"));
    EXPECT_NE(std::string::npos, output.find("## Summary"));
    EXPECT_NE(std::string::npos, output.find("## Level Breakdown"));
    EXPECT_EQ(std::string::npos, output.find("## Source Metadata"));
}

TEST(CliE2eTest, ConfigValidateSucceedsForSampleConfiguration)
{
    const std::string output = runLogscope("config validate --config " +
                                           scope::test_support::quoteArgument(sourcePath("samples/logscope.properties")) +
                                           " --require log.level");

    EXPECT_NE(std::string::npos, output.find("Configuration is valid."));
}

TEST(CliE2eTest, ExtensionsListShowsBuiltIns)
{
    const std::string output = runLogscope("extensions list");

    EXPECT_NE(std::string::npos, output.find("analysis.log-levels"));
    EXPECT_NE(std::string::npos, output.find("source.files"));
    EXPECT_NE(std::string::npos, output.find("reporting.multi-format"));
}

TEST(CliE2eTest, ExtensionsDescribeShowsMetadata)
{
    const std::string output = runLogscope("extensions describe analysis.log-levels");

    EXPECT_NE(std::string::npos, output.find("ID          : analysis.log-levels"));
    EXPECT_NE(std::string::npos, output.find("Description :"));
    EXPECT_NE(std::string::npos, output.find("ERROR"));
}

TEST(CliE2eTest, SessionSaveAndLoadRoundTrip)
{
    const std::string sessionFile = "cli_e2e_session.logscope-session";

    std::remove(sessionFile.c_str());

    const std::string saveOutput = runLogscope("session save " + sessionFile + " --min-errors 1 " +
                                               scope::test_support::quoteArgument(sourcePath("samples/sample.log")));

    EXPECT_NE(std::string::npos, saveOutput.find("Session saved to " + sessionFile));

    const std::string loadOutput = runLogscope("session load " + sessionFile);

    EXPECT_NE(std::string::npos, loadOutput.find("Matches     : yes"));
    EXPECT_NE(std::string::npos, loadOutput.find("Total log lines : 8"));
    EXPECT_NE(std::string::npos, loadOutput.find("Error lines     : 4"));

    std::remove(sessionFile.c_str());
}

TEST(CliE2eTest, AnalyzeSubcommandProducesHtmlReportToFile)
{
    const std::string outputFile = "cli_e2e_report.html";

    std::remove(outputFile.c_str());

    const std::string output =
        runLogscope("analyze --format html --sections executive,summary,charts --output " + outputFile + " " +
                    scope::test_support::quoteArgument(sourcePath("samples/sample.log")));

    std::ifstream stream(outputFile);
    ASSERT_TRUE(stream.good());

    std::string html((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

    std::remove(outputFile.c_str());

    EXPECT_NE(std::string::npos, html.find("<!DOCTYPE html>"));
    EXPECT_NE(std::string::npos, html.find("Executive Summary"));
    EXPECT_NE(std::string::npos, html.find("<svg"));
}

TEST(CliE2eTest, AnalyzeExecutiveAndErrorSections)
{
    const std::string output = runLogscope("analyze --sections executive,errors " +
                                           scope::test_support::quoteArgument(sourcePath("samples/sample.log")));

    EXPECT_NE(std::string::npos, output.find("Executive Summary"));
    EXPECT_NE(std::string::npos, output.find("Error Summary"));
    EXPECT_NE(std::string::npos, output.find("Health verdict"));
}

TEST(CliE2eTest, HelpDisplaysUsage)
{
    const std::string output = runLogscope("--help");

    EXPECT_NE(std::string::npos, output.find("Commands:"));
    EXPECT_NE(std::string::npos, output.find("analyze"));
    EXPECT_NE(std::string::npos, output.find("analytics"));
    EXPECT_NE(std::string::npos, output.find("query"));
    EXPECT_NE(std::string::npos, output.find("config validate"));
    EXPECT_NE(std::string::npos, output.find("extensions list"));
    EXPECT_NE(std::string::npos, output.find("session save"));
}

TEST(CliE2eTest, AnalyzeDirectoryProducesCombinedReport)
{
    const std::string output =
        runLogscope("analyze " + scope::test_support::quoteArgument(sourcePath("samples")));

    EXPECT_NE(std::string::npos, output.find("Total log lines : 8"));
    EXPECT_NE(std::string::npos, output.find("Error lines     : 4"));
}

TEST(CliE2eTest, AnalyzeStdinProducesReport)
{
    const std::string inputFile = "cli_e2e_stdin_input.log";

    {
        std::ofstream stream(inputFile);
        stream << "2026-07-11 10:00:01 INFO stdin line\n";
        stream << "2026-07-11 10:00:02 ERROR stdin error\n";
    }

#if defined(_WIN32)
    const std::string command =
        "cmd /c " + scope::test_support::quoteArgument(scope::test_support::quoteArgument(logscopeExecutable()) +
                                                         " analyze - < " + inputFile);
#else
    const std::string command =
        scope::test_support::quoteArgument(logscopeExecutable()) + " analyze - < " + inputFile;
#endif

    const std::string output = scope::test_support::captureCommandOutput(command);

    std::remove(inputFile.c_str());

    EXPECT_NE(std::string::npos, output.find("Total log lines : 2"));
    EXPECT_NE(std::string::npos, output.find("Error lines     : 1"));
}

TEST(CliE2eTest, InvestigateQueryFindsMatches)
{
    const std::string output = runLogscope("investigate --query \"error AND failed\" " +
                                           scope::test_support::quoteArgument(sourcePath("samples/sample.log")));

    EXPECT_NE(std::string::npos, output.find("========== INVESTIGATION RESULT =========="));
    EXPECT_NE(std::string::npos, output.find("Matching lines"));
}

TEST(CliE2eTest, AnalyticsCommandProducesSummary)
{
    const std::string output = runLogscope("analytics " +
                                           scope::test_support::quoteArgument(sourcePath("samples/sample.log")));

    EXPECT_NE(std::string::npos, output.find("Analytics summary"));
    EXPECT_NE(std::string::npos, output.find("Trend verdict"));
    EXPECT_NE(std::string::npos, output.find("Error clusters"));
}

TEST(CliE2eTest, InvestigateFilterFindsErrorLines)
{
    const std::string output = runLogscope("investigate --filter \"level == ERROR\" " +
                                           scope::test_support::quoteArgument(sourcePath("samples/sample.log")));

    EXPECT_NE(std::string::npos, output.find("========== INVESTIGATION RESULT =========="));
    EXPECT_NE(std::string::npos, output.find("Matching lines"));
}

TEST(CliE2eTest, QueryCommandRunsFilterExpression)
{
    const std::string output = runLogscope("query --filter \"level == ERROR\" " +
                                           scope::test_support::quoteArgument(sourcePath("samples/sample.log")));

    EXPECT_NE(std::string::npos, output.find("========== INVESTIGATION RESULT =========="));
    EXPECT_NE(std::string::npos, output.find("Matching lines"));
}

TEST(CliE2eTest, InvestigateCombinesTextQueryAndFilter)
{
    const std::string output =
        runLogscope("investigate --query \"error\" --filter \"contains(message, \\\"Connection\\\")\" " +
                    scope::test_support::quoteArgument(sourcePath("samples/sample.log")));

    EXPECT_NE(std::string::npos, output.find("========== INVESTIGATION RESULT =========="));
}

TEST(CliE2eTest, AnalyzeAnalyticsSections)
{
    const std::string output = runLogscope("analyze --sections analytics,clusters " +
                                           scope::test_support::quoteArgument(sourcePath("samples/sample.log")));

    EXPECT_NE(std::string::npos, output.find("Analytics"));
    EXPECT_NE(std::string::npos, output.find("Error Clusters"));
}
