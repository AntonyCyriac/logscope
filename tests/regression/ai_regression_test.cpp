/**
 * @file ai_regression_test.cpp
 * @brief Regression guards for M13 AI failure isolation.
 */

#include <gtest/gtest.h>

#include <cstdlib>

#include "analysis.hpp"
#include "investigation.hpp"
#include "line_index.hpp"

#include "ai_config.hpp"
#include "ai_investigation_assistant.hpp"
#include "nl_query_translator.hpp"
#include "noop_ai_provider.hpp"

using scope::ai::AiConfig;
using scope::ai::AiInvestigationAssistant;
using scope::ai::NoOpAiProvider;
using scope::ai::NlQueryTranslator;
using scope::analysis::AnalysisModel;
using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LineIndex;
using scope::analysis::LogLevelCounts;
using scope::analysis::makeLineIndex;
using scope::foundation::Path;
using scope::investigation::InvestigationCriteria;
using scope::investigation::InvestigationEngine;
using scope::investigation::InvestigationResult;
using scope::investigation::InvestigationView;

namespace
{

class ApiKeyEnvironment final
{
  public:
    explicit ApiKeyEnvironment(const char* value)
    {
#if defined(_WIN32)
        _putenv_s("LOGSCOPE_AI_API_KEY", value);
#else
        setenv("LOGSCOPE_AI_API_KEY", value, 1);
#endif
    }

    ~ApiKeyEnvironment()
    {
#if defined(_WIN32)
        _putenv_s("LOGSCOPE_AI_API_KEY", "");
#else
        unsetenv("LOGSCOPE_AI_API_KEY");
#endif
    }
};

InvestigationResult investigateSingleErrorLine()
{
    LineIndex lineIndex = makeLineIndex();

    IndexedLine line;
    line.lineNumber = 3U;
    line.level = DetectedLogLevel::Error;
    line.messageExcerpt = "timeout";
    line.contentExcerpt = "ERROR timeout";
    EXPECT_TRUE(lineIndex.tryAddLine(line));

    LogLevelCounts levelCounts;
    levelCounts.recordError();

    const AnalysisModel model(Path("app.log"), 1U, levelCounts, scope::analysis::LogFormat::PlainText,
                              std::nullopt, std::nullopt, std::move(lineIndex));

    InvestigationEngine engine;
    InvestigationCriteria criteria;
    criteria.contentSearch = "timeout";

    return engine.investigate(model, criteria);
}

} // namespace

TEST(AiRegressionTest, InvalidNaturalLanguageDoesNotProduceFilter)
{
    const NoOpAiProvider provider;
    const NlQueryTranslator translator(provider);

    const auto expression = translator.translateToFilterExpression("everything unusual");

    EXPECT_FALSE(expression);
}

TEST(AiRegressionTest, SummarizeFailureDoesNotMutateInvestigationResult)
{
    ApiKeyEnvironment apiKey("test-key");

    const InvestigationResult result = investigateSingleErrorLine();
    EXPECT_EQ(1U, result.matchingLines.size());

    AiConfig config;
    config.enabled = true;
    config.provider = "http";
    config.endpoint = "http://127.0.0.1:1/v1";
    config.model = "test-model";

    const AiInvestigationAssistant assistant(config);

    const InvestigationEngine engine;
    const AnalysisModel model(Path("app.log"), 1U);
    const InvestigationView view = engine.inspect(model);

    const auto summary = assistant.summarizeInvestigation(view, result);

    EXPECT_FALSE(summary);
    EXPECT_EQ(1U, result.matchingLines.size());
}
