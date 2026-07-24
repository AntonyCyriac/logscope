/**
 * @file query_planner_test.cpp
 */

#include <gtest/gtest.h>

#include "query_parser.hpp"
#include "query_planner.hpp"

using scope::query::parseFilterQuery;
using scope::storage::planQueryPushdown;

TEST(QueryPlannerTest, PlansLevelComparison)
{
    const auto parsed = parseFilterQuery("level == ERROR");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);

    ASSERT_TRUE(plan.has_value());
    EXPECT_NE(std::string::npos, plan->sqlWhere.find("level = 3"));
}

TEST(QueryPlannerTest, PlansContainsFunction)
{
    const auto parsed = parseFilterQuery(R"(contains(message, "timeout"))");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);

    ASSERT_TRUE(plan.has_value());
    EXPECT_NE(std::string::npos, plan->sqlWhere.find("LIKE"));
}

TEST(QueryPlannerTest, PlansBooleanComposition)
{
    const auto parsed = parseFilterQuery(R"(level == ERROR AND contains(message, "refused"))");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);

    ASSERT_TRUE(plan.has_value());
    EXPECT_NE(std::string::npos, plan->sqlWhere.find("AND"));
}

TEST(QueryPlannerTest, PlansLineComparison)
{
    const auto parsed = parseFilterQuery("line > 100");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);

    ASSERT_TRUE(plan.has_value());
    EXPECT_NE(std::string::npos, plan->sqlWhere.find("line_number > 100"));
}

TEST(QueryPlannerTest, PlansTimestampComparison)
{
    const auto parsed = parseFilterQuery(R"(time >= "2026-07-11T10:00:00")");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);

    ASSERT_TRUE(plan.has_value());
    EXPECT_NE(std::string::npos, plan->sqlWhere.find("timestamp_unix >="));
}

TEST(QueryPlannerTest, PlansCorrelationIdComparison)
{
    const auto parsed = parseFilterQuery(R"(correlationId == "req-42")");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);

    ASSERT_TRUE(plan.has_value());
    EXPECT_NE(std::string::npos, plan->sqlWhere.find("correlation_id = 'req-42'"));
}

TEST(QueryPlannerTest, PlansOrComposition)
{
    const auto parsed = parseFilterQuery(R"(level == ERROR OR level == WARN)");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);

    ASSERT_TRUE(plan.has_value());
    EXPECT_NE(std::string::npos, plan->sqlWhere.find(" OR "));
}

TEST(QueryPlannerTest, PlansNotComposition)
{
    const auto parsed = parseFilterQuery(R"(NOT level == INFO)");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);

    ASSERT_TRUE(plan.has_value());
    EXPECT_NE(std::string::npos, plan->sqlWhere.find("NOT ("));
}

TEST(QueryPlannerTest, PlansHasKeyFunction)
{
    const auto parsed = parseFilterQuery(R"(hasKey("service"))");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);

    ASSERT_TRUE(plan.has_value());
    EXPECT_NE(std::string::npos, plan->sqlWhere.find("top_level_keys_json"));
}

TEST(QueryPlannerTest, RejectsUnsupportedComparison)
{
    const auto parsed = parseFilterQuery(R"(service == "PCF")");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);

    EXPECT_FALSE(plan.has_value());
}
