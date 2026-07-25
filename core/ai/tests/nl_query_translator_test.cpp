/**
 * @file nl_query_translator_test.cpp
 */

#include <gtest/gtest.h>

#include "line_index.hpp"
#include "log_line_classifier.hpp"
#include "nl_query_translator.hpp"
#include "noop_ai_provider.hpp"
#include "query_evaluator.hpp"

using scope::ai::NlQueryTranslator;
using scope::ai::NoOpAiProvider;
using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::query::QueryEvaluator;

namespace
{

IndexedLine makeLine(DetectedLogLevel level, const std::string& message)
{
    IndexedLine line;
    line.level = level;
    line.messageExcerpt = message;

    return line;
}

} // namespace

TEST(NlQueryTranslatorTest, TranslatesAndValidatesNoopHeuristic)
{
    const NoOpAiProvider provider;
    const NlQueryTranslator translator(provider);

    const auto expression = translator.translateToFilterExpression("show errors");

    ASSERT_TRUE(expression);
    EXPECT_EQ(*expression, "level == ERROR");

    const auto query = translator.translateToFilterQuery("show errors");

    ASSERT_TRUE(query);

    const QueryEvaluator evaluator(*query);
    const IndexedLine errorLine = makeLine(DetectedLogLevel::Error, "connection failed");
    const IndexedLine infoLine = makeLine(DetectedLogLevel::Info, "started");

    EXPECT_TRUE(evaluator.matches(errorLine));
    EXPECT_FALSE(evaluator.matches(infoLine));
}

TEST(NlQueryTranslatorTest, RejectsEmptyNaturalLanguageQuery)
{
    const NoOpAiProvider provider;
    const NlQueryTranslator translator(provider);

    const auto result = translator.translateToFilterExpression("");

    EXPECT_FALSE(result);
}

TEST(NlQueryTranslatorTest, RejectsUnmappedNaturalLanguage)
{
    const NoOpAiProvider provider;
    const NlQueryTranslator translator(provider);

    const auto result = translator.translateToFilterExpression("everything unusual");

    EXPECT_FALSE(result);
}

TEST(NlQueryTranslatorTest, RejectsInvalidProviderOutput)
{
    struct InvalidDslProvider final : public scope::ai::AiProvider
    {
        std::string id() const override
        {
            return "test.invalid";
        }

        scope::foundation::Result<std::string> translateNlToFilter(std::string_view) const override
        {
            return scope::foundation::Result<std::string>("level ==");
        }

        scope::foundation::Result<scope::ai::AiSummary> summarize(
            const scope::ai::AiInvestigationContext&) const override
        {
            return scope::foundation::Result<scope::ai::AiSummary>(scope::ai::AiSummary{});
        }

        scope::foundation::Result<std::vector<scope::ai::AiAnomalyHint>> suggestAnomalies(
            const scope::ai::AiAnalyticsContext&) const override
        {
            return scope::foundation::Result<std::vector<scope::ai::AiAnomalyHint>>(
                std::vector<scope::ai::AiAnomalyHint>{});
        }
    };

    const InvalidDslProvider provider;
    const NlQueryTranslator translator(provider);

    const auto result = translator.translateToFilterExpression("anything");

    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().code(), scope::foundation::ErrorCode::ParseError);
}
