/**
 * @file query_benchmark.cpp
 * @brief Performance benchmark for QueryEvaluator.
 */

#include <benchmark/benchmark.h>

#include "analysis_model.hpp"
#include "line_index.hpp"
#include "query_evaluator.hpp"
#include "query_parser.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LineIndex;
using scope::analysis::LogLevelCounts;
using scope::analysis::makeLineIndex;
using scope::query::QueryEvaluator;
using scope::query::parseFilterQuery;
using scope::foundation::Path;
using scope::foundation::Timestamp;

namespace
{

AnalysisModel buildIndexedModel(const std::size_t lineCount)
{
    LineIndex index = makeLineIndex();

    for (std::size_t indexValue = 0U; indexValue < lineCount; ++indexValue)
    {
        IndexedLine line;
        line.lineNumber = static_cast<std::uint64_t>(indexValue + 1U);
        line.level = indexValue % 4U == 0U ? DetectedLogLevel::Error : DetectedLogLevel::Info;
        line.messageExcerpt = "Error code=" + std::to_string(indexValue % 7U);
        line.contentExcerpt = line.messageExcerpt;
        line.timestamp = Timestamp::fromUnixSeconds(static_cast<std::int64_t>(1'700'000'000 + indexValue));
        (void)index.tryAddLine(line);
    }

    LogLevelCounts counts;

    return AnalysisModel(Path("benchmark.log"), lineCount, counts, scope::analysis::LogFormat::PlainText,
                         std::nullopt, std::nullopt, std::move(index));
}

} // namespace

static void BM_QueryEvaluator(benchmark::State& state)
{
    const AnalysisModel model = buildIndexedModel(10000U);
    const auto parsed = parseFilterQuery(R"(level == ERROR AND contains(message, "code=3"))");
    benchmark::DoNotOptimize(parsed);

    if (!parsed || !model.lineIndex().has_value())
    {
        state.SkipWithError("Failed to prepare benchmark input");

        return;
    }

    const QueryEvaluator evaluator(*parsed);

    for (auto _ : state)
    {
        std::size_t matches = 0U;

        for (const IndexedLine& line : model.lineIndex()->lines())
        {
            if (evaluator.matches(line))
            {
                ++matches;
            }
        }

        benchmark::DoNotOptimize(matches);
    }
}

BENCHMARK(BM_QueryEvaluator);
