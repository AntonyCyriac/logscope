/**
 * @file ai_investigation_assistant_test.cpp
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "investigation.hpp"
#include "line_index.hpp"

#include "ai_investigation_assistant.hpp"
#include "analytics_engine.hpp"

using scope::ai::AiConfig;
using scope::ai::AiInvestigationAssistant;
using scope::ai::kProviderNoOp;
using scope::analysis::AnalysisModel;
using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LineIndex;
using scope::analysis::LogLevelCounts;
using scope::analysis::makeLineIndex;
using scope::analytics::AnalyticsEngine;
using scope::foundation::Path;
using scope::investigation::InvestigationCriteria;
using scope::investigation::InvestigationEngine;
using scope::investigation::InvestigationResult;
using scope::investigation::InvestigationView;

TEST(AiInvestigationAssistantTest, ResolvesNoopProvider)
{
    const AiConfig config;
    const AiInvestigationAssistant assistant(config);

    EXPECT_EQ(assistant.provider().id(), kProviderNoOp);
    EXPECT_FALSE(assistant.config().enabled);
}

TEST(AiInvestigationAssistantTest, TranslatesNaturalLanguageQuery)
{
    const AiConfig config;
    const AiInvestigationAssistant assistant(config);

    const auto expression = assistant.translateNaturalLanguageQuery("errors");

    ASSERT_TRUE(expression);
    EXPECT_EQ(*expression, "level == ERROR");

    const auto query = assistant.translateNaturalLanguageFilter("warnings");

    ASSERT_TRUE(query);
}

TEST(AiInvestigationAssistantTest, RejectsSummaryWhenDisabled)
{
    AiConfig config;
    config.enabled = false;

    const AiInvestigationAssistant assistant(config);
    const InvestigationEngine engine;
    const AnalysisModel model(Path("sample.log"), 1U);
    const InvestigationView view = engine.inspect(model);

    const InvestigationResult result;

    const auto summary = assistant.summarizeInvestigation(view, result);

    EXPECT_FALSE(summary);
}

TEST(AiInvestigationAssistantTest, SummarizesInvestigationWhenEnabled)
{
    AiConfig config;
    config.enabled = true;

    LineIndex lineIndex = makeLineIndex();

    IndexedLine line;
    line.lineNumber = 7U;
    line.level = DetectedLogLevel::Error;
    line.messageExcerpt = "timeout";
    line.contentExcerpt = "ERROR timeout";
    ASSERT_TRUE(lineIndex.tryAddLine(line));

    LogLevelCounts levelCounts;
    levelCounts.recordError();

    const AnalysisModel model(Path("app.log"), 1U, levelCounts, scope::analysis::LogFormat::PlainText, std::nullopt,
                              std::nullopt, std::move(lineIndex));

    InvestigationEngine engine;
    InvestigationCriteria criteria;
    criteria.contentSearch = "timeout";

    const InvestigationView view = engine.inspect(model);
    const auto result = engine.investigate(model, criteria);

    const AiInvestigationAssistant assistant(config);
    const auto summary = assistant.summarizeInvestigation(view, result);

    ASSERT_TRUE(summary);
    EXPECT_NE(std::string::npos, summary->summary.find("1 matching line"));
    ASSERT_EQ(1U, summary->evidence.size());
    EXPECT_EQ(7U, summary->evidence[0].lineNumber);
}

TEST(AiInvestigationAssistantTest, RejectsAnomalyHintsWhenDisabled)
{
    AiConfig config;
    config.enabled = false;

    const AiInvestigationAssistant assistant(config);
    const AnalyticsEngine engine;
    const AnalysisModel model(Path("sample.log"), 1U);
    const auto analytics = engine.analyze(model);

    const auto hints = assistant.suggestAnomalyHints(analytics);

    EXPECT_FALSE(hints);
}

TEST(AiInvestigationAssistantTest, SuggestsAnomalyHintsWhenEnabled)
{
    AiConfig config;
    config.enabled = true;

    LineIndex index = makeLineIndex();

    IndexedLine line;
    line.lineNumber = 1U;
    line.level = DetectedLogLevel::Error;
    line.messageExcerpt = "Connection refused";
    ASSERT_TRUE(index.tryAddLine(line));

    line.lineNumber = 2U;
    ASSERT_TRUE(index.tryAddLine(line));

    LogLevelCounts counts;
    counts.recordError();
    counts.recordError();

    const AnalysisModel model(Path("sample.log"), 2U, counts, scope::analysis::LogFormat::PlainText, std::nullopt,
                              std::nullopt, std::move(index));

    const AnalyticsEngine engine;
    const auto analytics = engine.analyze(model);

    const AiInvestigationAssistant assistant(config);
    const auto hints = assistant.suggestAnomalyHints(analytics);

    ASSERT_TRUE(hints);
    ASSERT_FALSE(hints->empty());
    EXPECT_NE(std::string::npos, hints->front().message.find("Repeated error cluster"));
}
