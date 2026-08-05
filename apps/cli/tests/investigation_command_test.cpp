/**
 * @file investigation_command_test.cpp
 * @brief Unit tests for investigation CLI commands (Story 2 / v2.4.0).
 */

#include <filesystem>
#include <fstream>
#include <sstream>

#include <gtest/gtest.h>

#include "gtest_temp_path.hpp"
#include "investigation_command.hpp"
#include "workspace.hpp"

using scope::cli::InvestigationAddOptions;
using scope::cli::InvestigationCreateOptions;
using scope::cli::InvestigationOpenOptions;
using scope::cli::InvestigationShowOptions;
using scope::cli::runInvestigationAddCommand;
using scope::cli::runInvestigationCreateCommand;
using scope::cli::runInvestigationOpenCommand;
using scope::cli::runInvestigationShowCommand;
using scope::foundation::Path;
using scope::workspace::Investigation;

namespace
{

void removeDirectoryTree(const Path& directory)
{
    std::error_code errorCode;
    std::filesystem::remove_all(directory.string(), errorCode);
}

std::string trimTrailingNewline(std::string value)
{
    while (!value.empty() && (value.back() == '\n' || value.back() == '\r'))
    {
        value.pop_back();
    }

    return value;
}

std::string createInvestigation(const Path& rootDirectory)
{
    InvestigationCreateOptions options;
    options.name = "cli-test";
    options.rootDirectory = rootDirectory;

    std::ostringstream output;
    std::ostringstream errorOutput;

    EXPECT_EQ(0, runInvestigationCreateCommand(options, output, errorOutput));

    return trimTrailingNewline(output.str());
}

Path writeSourceFile(const std::string& suffix, const std::string& contents)
{
    const Path path(logscope::gtest::uniqueTestPath(suffix));
    std::ofstream stream(path.string());

    EXPECT_TRUE(stream);
    stream << contents;

    return path;
}

class InvestigationCommandTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        m_root = Path(logscope::gtest::uniqueTestPath("_investigation_root"));
        std::error_code errorCode;
        std::filesystem::create_directories(m_root.string(), errorCode);
        m_investigationId = createInvestigation(m_root);
    }

    void TearDown() override
    {
        removeDirectoryTree(m_root);
    }

    Path m_root;
    std::string m_investigationId;
};

} // namespace

TEST_F(InvestigationCommandTest, AddInfersCoreTypeFromExtension)
{
    const Path source = writeSourceFile("_dump.core", "CORE");

    InvestigationAddOptions options;
    options.investigationId = m_investigationId;
    options.logFile = source;
    options.rootDirectory = m_root;

    std::ostringstream output;
    std::ostringstream errorOutput;

    ASSERT_EQ(0, runInvestigationAddCommand(options, output, errorOutput));

    const auto investigation = Investigation::open(Path(m_root.string() + "/" + m_investigationId));

    ASSERT_TRUE(investigation);
    ASSERT_EQ(1U, investigation->manifest().artifacts.size());
    EXPECT_EQ("core", investigation->manifest().artifacts.front().type);

    std::filesystem::remove(source.string());
}

TEST_F(InvestigationCommandTest, AddInfersPstackTypeFromExtension)
{
    const Path source = writeSourceFile("_threads.pstack", "#0 main ()\n");

    InvestigationAddOptions options;
    options.investigationId = m_investigationId;
    options.logFile = source;
    options.rootDirectory = m_root;

    std::ostringstream output;
    std::ostringstream errorOutput;

    ASSERT_EQ(0, runInvestigationAddCommand(options, output, errorOutput));

    const auto investigation = Investigation::open(Path(m_root.string() + "/" + m_investigationId));

    ASSERT_TRUE(investigation);
    ASSERT_EQ(1U, investigation->manifest().artifacts.size());
    EXPECT_EQ("pstack", investigation->manifest().artifacts.front().type);

    std::filesystem::remove(source.string());
}

TEST_F(InvestigationCommandTest, AddDefaultsToLogType)
{
    const Path source = writeSourceFile("_app.log", "2026-08-05 ERROR failed\n");

    InvestigationAddOptions options;
    options.investigationId = m_investigationId;
    options.logFile = source;
    options.rootDirectory = m_root;

    std::ostringstream output;
    std::ostringstream errorOutput;

    ASSERT_EQ(0, runInvestigationAddCommand(options, output, errorOutput));

    const auto investigation = Investigation::open(Path(m_root.string() + "/" + m_investigationId));

    ASSERT_TRUE(investigation);
    ASSERT_EQ(1U, investigation->manifest().artifacts.size());
    EXPECT_EQ("log", investigation->manifest().artifacts.front().type);
    EXPECT_EQ(investigation->manifest().artifacts.front().id, investigation->manifest().primaryArtifactId);

    std::filesystem::remove(source.string());
}

TEST_F(InvestigationCommandTest, AddExplicitPstackTypeAndRole)
{
    const Path source = writeSourceFile("_trace.txt", "#0 main ()\n");

    InvestigationAddOptions options;
    options.investigationId = m_investigationId;
    options.logFile = source;
    options.artifactType = "pstack";
    options.role = "application";
    options.displayName = "app-threads";
    options.rootDirectory = m_root;

    std::ostringstream output;
    std::ostringstream errorOutput;

    ASSERT_EQ(0, runInvestigationAddCommand(options, output, errorOutput));

    const auto investigation = Investigation::open(Path(m_root.string() + "/" + m_investigationId));

    ASSERT_TRUE(investigation);
    ASSERT_EQ(1U, investigation->manifest().artifacts.size());
    EXPECT_EQ("pstack", investigation->manifest().artifacts.front().type);
    EXPECT_EQ("app-threads", investigation->manifest().artifacts.front().name);
    EXPECT_EQ("application", investigation->manifest().artifacts.front().metadata.at("role"));

    std::filesystem::remove(source.string());
}

TEST_F(InvestigationCommandTest, RejectsUnsupportedArtifactType)
{
    const Path source = writeSourceFile("_note.txt", "text");

    InvestigationAddOptions options;
    options.investigationId = m_investigationId;
    options.logFile = source;
    options.artifactType = "note";
    options.rootDirectory = m_root;

    std::ostringstream output;
    std::ostringstream errorOutput;

    EXPECT_EQ(1, runInvestigationAddCommand(options, output, errorOutput));
    EXPECT_NE(std::string::npos, errorOutput.str().find("Unsupported artifact type"));

    std::filesystem::remove(source.string());
}

TEST_F(InvestigationCommandTest, OpenEntryAndSpecificLogArtifactPaths)
{
    const Path appLog = writeSourceFile("_app.log", "2026-08-05 ERROR app\n");
    const Path syslog = writeSourceFile("_syslog.log", "2026-08-05 ERROR kernel\n");

    InvestigationAddOptions addApp;
    addApp.investigationId = m_investigationId;
    addApp.logFile = appLog;
    addApp.displayName = "app.log";
    addApp.role = "application";
    addApp.rootDirectory = m_root;

    std::ostringstream addOutput;
    std::ostringstream addError;

    ASSERT_EQ(0, runInvestigationAddCommand(addApp, addOutput, addError));

    addOutput.str("");
    addOutput.clear();
    addError.str("");
    addError.clear();

    InvestigationAddOptions addSyslog;
    addSyslog.investigationId = m_investigationId;
    addSyslog.logFile = syslog;
    addSyslog.displayName = "syslog";
    addSyslog.role = "system";
    addSyslog.rootDirectory = m_root;

    ASSERT_EQ(0, runInvestigationAddCommand(addSyslog, addOutput, addError));
    const std::string syslogArtifactId = trimTrailingNewline(addOutput.str());

    InvestigationOpenOptions openEntry;
    openEntry.investigationId = m_investigationId;
    openEntry.rootDirectory = m_root;

    std::ostringstream openOutput;
    std::ostringstream openError;

    ASSERT_EQ(0, runInvestigationOpenCommand(openEntry, openOutput, openError));
    EXPECT_NE(std::string::npos, openOutput.str().find("log\tapp.log"));

    openOutput.str("");
    openOutput.clear();
    openError.str("");
    openError.clear();

    InvestigationOpenOptions openSyslog;
    openSyslog.investigationId = m_investigationId;
    openSyslog.artifactId = syslogArtifactId;
    openSyslog.rootDirectory = m_root;

    ASSERT_EQ(0, runInvestigationOpenCommand(openSyslog, openOutput, openError));
    EXPECT_NE(std::string::npos, openOutput.str().find("log\tsyslog"));

    std::filesystem::remove(appLog.string());
    std::filesystem::remove(syslog.string());
}

TEST_F(InvestigationCommandTest, ShowMarksEntryAndRole)
{
    const Path appLog = writeSourceFile("_app.log", "2026-08-05 ERROR app\n");

    InvestigationAddOptions addApp;
    addApp.investigationId = m_investigationId;
    addApp.logFile = appLog;
    addApp.displayName = "app.log";
    addApp.role = "application";
    addApp.rootDirectory = m_root;

    std::ostringstream addOutput;
    std::ostringstream addError;

    ASSERT_EQ(0, runInvestigationAddCommand(addApp, addOutput, addError));

    InvestigationShowOptions showOptions;
    showOptions.investigationId = m_investigationId;
    showOptions.rootDirectory = m_root;

    std::ostringstream showOutput;
    std::ostringstream showError;

    ASSERT_EQ(0, runInvestigationShowCommand(showOptions, showOutput, showError));
    EXPECT_NE(std::string::npos, showOutput.str().find("[entry]"));
    EXPECT_NE(std::string::npos, showOutput.str().find("role=application"));
    EXPECT_NE(std::string::npos, showOutput.str().find("(log)"));

    std::filesystem::remove(appLog.string());
}
