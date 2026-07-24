/**
 * @file query_evaluator_test.cpp
 */

#include <gtest/gtest.h>

#include "line_index.hpp"
#include "query_evaluator.hpp"
#include "query_parser.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::query::QueryEvaluator;
using scope::query::parseFilterQuery;
using scope::foundation::Timestamp;

namespace
{

IndexedLine makeLine(const DetectedLogLevel level, const std::string& message)
{
    IndexedLine line;
    line.lineNumber = 1U;
    line.level = level;
    line.messageExcerpt = message;
    line.contentExcerpt = message;
    line.timestamp = Timestamp::fromUnixSeconds(1'700'000'000);

    return line;
}

} // namespace

TEST(QueryEvaluatorTest, MatchesLevelComparison)
{
    const auto parsed = parseFilterQuery("level == ERROR");
    ASSERT_TRUE(parsed);

    const QueryEvaluator evaluator(*parsed);
    const IndexedLine errorLine = makeLine(DetectedLogLevel::Error, "Connection refused");
    const IndexedLine infoLine = makeLine(DetectedLogLevel::Info, "Started");

    EXPECT_TRUE(evaluator.matches(errorLine));
    EXPECT_FALSE(evaluator.matches(infoLine));
}

TEST(QueryEvaluatorTest, MatchesContainsFunction)
{
    const auto parsed = parseFilterQuery(R"(contains(message, "timeout"))");
    ASSERT_TRUE(parsed);

    const QueryEvaluator evaluator(*parsed);

    EXPECT_TRUE(evaluator.matches(makeLine(DetectedLogLevel::Error, "Request timeout")));
    EXPECT_FALSE(evaluator.matches(makeLine(DetectedLogLevel::Error, "Connection refused")));
}

TEST(QueryEvaluatorTest, MatchesLineNumberComparison)
{
    IndexedLine line;
    line.lineNumber = 42U;
    line.level = DetectedLogLevel::Info;

    const auto parsed = parseFilterQuery("line == 42");
    ASSERT_TRUE(parsed);

    const QueryEvaluator evaluator(*parsed);

    EXPECT_TRUE(evaluator.matches(line));
}

TEST(QueryEvaluatorTest, MatchesHasKeyFunction)
{
    IndexedLine line;
    line.topLevelKeys = {"correlationId", "level"};

    const auto parsed = parseFilterQuery(R"(hasKey("correlationId"))");
    ASSERT_TRUE(parsed);

    const QueryEvaluator evaluator(*parsed);

    EXPECT_TRUE(evaluator.matches(line));
}

TEST(QueryEvaluatorTest, MatchesCombinedExpression)
{
    const auto parsed = parseFilterQuery(R"(level == ERROR AND contains(message, "refused"))");
    ASSERT_TRUE(parsed);

    const QueryEvaluator evaluator(*parsed);

    EXPECT_TRUE(evaluator.matches(makeLine(DetectedLogLevel::Error, "Connection refused")));
    EXPECT_FALSE(evaluator.matches(makeLine(DetectedLogLevel::Info, "Connection refused")));
    EXPECT_FALSE(evaluator.matches(makeLine(DetectedLogLevel::Error, "Request timeout")));
}

TEST(QueryEvaluatorTest, MatchesContentField)
{
    IndexedLine line;
    line.contentExcerpt = "full payload with timeout";

    const auto parsed = parseFilterQuery(R"(contains(content, "timeout"))");
    ASSERT_TRUE(parsed);

    const QueryEvaluator evaluator(*parsed);

    EXPECT_TRUE(evaluator.matches(line));
}

TEST(QueryEvaluatorTest, MatchesCorrelationIdField)
{
    IndexedLine line;
    line.correlationId = "abc-123";

    const auto parsed = parseFilterQuery(R"(correlationId == "abc-123")");
    ASSERT_TRUE(parsed);

    const QueryEvaluator evaluator(*parsed);

    EXPECT_TRUE(evaluator.matches(line));
    EXPECT_FALSE(evaluator.matches(makeLine(DetectedLogLevel::Info, "other")));
}

TEST(QueryEvaluatorTest, RejectsMissingTimestampComparison)
{
    IndexedLine line;
    line.level = DetectedLogLevel::Error;

    const auto parsed = parseFilterQuery(R"(time >= "2026-07-11 10:00:00")");
    ASSERT_TRUE(parsed);

    const QueryEvaluator evaluator(*parsed);

    EXPECT_FALSE(evaluator.matches(line));
}

TEST(QueryEvaluatorTest, MatchesNotExpression)
{
    const auto parsed = parseFilterQuery("NOT level == INFO");
    ASSERT_TRUE(parsed);

    const QueryEvaluator evaluator(*parsed);

    EXPECT_TRUE(evaluator.matches(makeLine(DetectedLogLevel::Error, "failed")));
    EXPECT_FALSE(evaluator.matches(makeLine(DetectedLogLevel::Info, "started")));
}

TEST(QueryEvaluatorTest, MatchesOrExpression)
{
    const auto parsed = parseFilterQuery("level == ERROR OR level == WARNING");
    ASSERT_TRUE(parsed);

    const QueryEvaluator evaluator(*parsed);

    EXPECT_TRUE(evaluator.matches(makeLine(DetectedLogLevel::Error, "failed")));
    EXPECT_TRUE(evaluator.matches(makeLine(DetectedLogLevel::Warn, "slow")));
    EXPECT_FALSE(evaluator.matches(makeLine(DetectedLogLevel::Info, "ok")));
}
