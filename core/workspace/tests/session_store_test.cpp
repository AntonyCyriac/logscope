/**
 * @file session_store_test.cpp
 * @brief Unit tests for SessionStore.
 */

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "workspace.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::LogLevelCounts;
using scope::foundation::Path;
using scope::investigation::InvestigationCriteria;
using scope::investigation::LineCountFilter;
using scope::investigation::LogLevelFilter;
using scope::reporting::ReportFormat;
using scope::reporting::ReportOptions;
using scope::workspace::InvestigationSession;
using scope::workspace::SessionStore;

namespace
{

InvestigationSession createSampleSession()
{
    const LogLevelCounts levelCounts = LogLevelCounts::fromCounts(3U, 1U, 4U, 0U, 0U);
    const AnalysisModel model(Path("sample.log"), 8U, levelCounts);

    ReportOptions reportOptions;
    reportOptions.format = ReportFormat::Json;

    return InvestigationSession::fromAnalysis(
        model, LineCountFilter::nonEmpty(), LogLevelFilter::any().withMinimumErrors(1U), "sample",
        InvestigationCriteria{}, reportOptions, Path("logscope.properties"));
}

} // namespace

TEST(SessionStoreTest, SavesAndLoadsSession)
{
    const Path sessionFile("session_store_test.logscope-session");
    const SessionStore store;

    const auto saveResult = store.save(createSampleSession(), sessionFile);

    ASSERT_TRUE(saveResult.hasValue());
    EXPECT_TRUE(*saveResult);

    const auto loadResult = store.load(sessionFile);

    ASSERT_TRUE(loadResult.hasValue());
    EXPECT_EQ("sample.log", loadResult->sourcePath().string());
    EXPECT_EQ(8U, loadResult->analysisModel().totalLines());
    EXPECT_EQ(4U, loadResult->analysisModel().levelCounts().errorLines());
    EXPECT_EQ(1U, loadResult->levelFilter().minimumErrors());
    EXPECT_EQ(ReportFormat::Json, loadResult->reportOptions().format);
    EXPECT_EQ("sample", loadResult->searchQuery());

    std::remove(sessionFile.string().c_str());
}

TEST(SessionStoreTest, RoundTripsContentInvestigationFilters)
{
    const Path sessionFile("session_store_content_test.logscope-session");
    const SessionStore store;

    LogLevelCounts levelCounts = LogLevelCounts::fromCounts(1U, 0U, 1U, 0U, 0U);
    const AnalysisModel model(Path("sample.log"), 2U, levelCounts);

    ReportOptions reportOptions;
    InvestigationCriteria criteria;
    criteria.contentSearch = "refused";
    criteria.field = criteria.field.withLevel(scope::analysis::DetectedLogLevel::Error);

    const InvestigationSession session = InvestigationSession::fromAnalysis(
        model, LineCountFilter::any(), LogLevelFilter::any(), "", criteria, reportOptions, Path());

    ASSERT_TRUE(store.save(session, sessionFile).hasValue());

    const auto loadResult = store.load(sessionFile);

    ASSERT_TRUE(loadResult.hasValue());
    EXPECT_EQ("refused", loadResult->contentCriteria().contentSearch);
    ASSERT_TRUE(loadResult->contentCriteria().field.level().has_value());
    EXPECT_EQ(scope::analysis::DetectedLogLevel::Error, *loadResult->contentCriteria().field.level());

    std::remove(sessionFile.string().c_str());
}

TEST(SessionStoreTest, ListsSessionFilesInDirectory)
{
    const Path directoryPath("session_store_dir");

    std::filesystem::create_directory(directoryPath.string());

    const Path sessionFile = directoryPath.append("test.logscope-session");
    const Path otherFile = directoryPath.append("notes.txt");

    SessionStore store;
    ASSERT_TRUE(store.save(createSampleSession(), sessionFile).hasValue());

    {
        std::ofstream otherStream(otherFile.string());
        otherStream << "ignore";
    }

    const auto listResult = store.list(directoryPath);

    ASSERT_TRUE(listResult.hasValue());
    ASSERT_EQ(1U, listResult->size());
    EXPECT_EQ(sessionFile.string(), (*listResult)[0].string());

    std::remove(sessionFile.string().c_str());
    std::remove(otherFile.string().c_str());
    std::filesystem::remove(directoryPath.string());
}
