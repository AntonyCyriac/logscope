/**
 * @file application_service_test.cpp
 * @brief Unit tests for ApplicationService (M14).
 */

#include "application_service.hpp"

#include <gtest/gtest.h>

#include "foundation/path.hpp"

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
