/**
 * @file application_service_test.cpp
 * @brief Unit tests for ApplicationService (M14).
 */

#include "application_service.hpp"

#include <gtest/gtest.h>

#include "foundation/path.hpp"
#include "investigation_criteria.hpp"
#include "time_range_filter.hpp"

using scope::application::ApplicationService;
using scope::foundation::Path;

TEST(ApplicationServiceTest, LoadEmptyConfigurationSucceeds)
{
    ApplicationService service;

    const auto result = service.loadConfiguration(Path{});

    ASSERT_TRUE(result);
    EXPECT_TRUE(*result);
}

TEST(ApplicationServiceTest, AnalyzeSampleLog)
{
    ApplicationService service;
    ASSERT_TRUE(service.loadConfiguration(Path{}));

    const Path samplePath("samples/sample.log");
    ASSERT_TRUE(service.openSource(samplePath));

    const auto modelResult = service.analyze(scope::analysis::AnalysisConfig::defaults());

    ASSERT_TRUE(modelResult);
    EXPECT_GT(modelResult->totalLines(), 0U);
}

TEST(ApplicationServiceTest, InvestigateMatchesErrors)
{
    ApplicationService service;
    ASSERT_TRUE(service.loadConfiguration(Path{}));

    ASSERT_TRUE(service.openSource(Path("samples/sample.log")));
    ASSERT_TRUE(service.analyze(scope::analysis::AnalysisConfig::defaults()));

    scope::investigation::InvestigationCriteria criteria;
    criteria.filterExpression = "level == ERROR";

    const auto result = service.investigate(criteria);

    ASSERT_TRUE(result);
    EXPECT_FALSE(result->matchingLines.empty());
}

TEST(ApplicationServiceTest, InvestigateTimeRangeFiltersLines)
{
    ApplicationService service;
    ASSERT_TRUE(service.loadConfiguration(Path{}));

    ASSERT_TRUE(service.openSource(Path("samples/sample.log")));
    ASSERT_TRUE(service.analyze(scope::analysis::AnalysisConfig::defaults()));

    const auto earliest = scope::foundation::Timestamp::parse("2026-07-11T10:00:06");
    const auto latest = scope::foundation::Timestamp::parse("2026-07-11T10:00:15");
    ASSERT_TRUE(earliest);
    ASSERT_TRUE(latest);

    scope::investigation::InvestigationCriteria criteria;
    criteria.timeRange =
        scope::investigation::TimeRangeFilter::any().withEarliest(*earliest).withLatest(*latest);

    const auto result = service.investigate(criteria);

    ASSERT_TRUE(result);
    EXPECT_FALSE(result->matchingLines.empty());
    EXPECT_LE(result->matchingLines.size(), 4U);
}

TEST(ApplicationServiceTest, ValidateSampleConfiguration)
{
    ApplicationService service;
    ASSERT_TRUE(service.loadConfiguration(Path("samples/logscope.properties")));

    const auto validateResult = service.validateConfiguration();

    ASSERT_TRUE(validateResult);
    EXPECT_TRUE(*validateResult);
}

TEST(ApplicationServiceTest, AgentInvestigateNoopAskErrors)
{
    ApplicationService service;
    ASSERT_TRUE(service.loadConfiguration(Path("samples/ai-noop.properties")));

    ASSERT_TRUE(service.openSource(Path("samples/sample.log")));
    ASSERT_TRUE(service.analyze(scope::analysis::AnalysisConfig::defaults()));

    scope::investigation::InvestigationCriteria criteria;
    const auto result = service.agentInvestigate(criteria, "errors", false, false);

    ASSERT_TRUE(result);
    EXPECT_EQ(result->investigation.matchingLines.size(), 4U);
}

TEST(ApplicationServiceTest, AgentInvestigateReusesExistingModel)
{
    ApplicationService service;
    ASSERT_TRUE(service.loadConfiguration(Path("samples/ai-noop.properties")));
    ASSERT_TRUE(service.openSource(Path("samples/sample.log")));
    ASSERT_TRUE(service.analyze(scope::analysis::AnalysisConfig::defaults()));

    scope::investigation::InvestigationCriteria criteria;
    const auto firstAsk = service.agentInvestigate(criteria, "errors", false, false);
    const auto secondAsk = service.agentInvestigate(criteria, "warnings", false, false);

    ASSERT_TRUE(firstAsk);
    ASSERT_TRUE(secondAsk);
    EXPECT_EQ(firstAsk->investigation.matchingLines.size(), 4U);
    EXPECT_FALSE(secondAsk->investigation.matchingLines.empty());
}
