/**
 * @file analytics_benchmark.cpp
 * @brief Performance benchmarks for analytics engine.
 */

#include <vector>

#include <benchmark/benchmark.h>

#include "analysis_model.hpp"
#include "analytics_engine.hpp"
#include "foundation/path.hpp"
#include "foundation/timestamp.hpp"
#include "line_index.hpp"
#include "log_level_counts.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::DetectedLogLevel;
using scope::analysis::FieldSummary;
using scope::analysis::IndexedLine;
using scope::analysis::LineIndex;
using scope::analysis::LogLevelCounts;
using scope::analysis::makeLineIndex;
using scope::analytics::AnalyticsConfig;
using scope::analytics::AnalyticsEngine;
using scope::foundation::Path;
using scope::foundation::Timestamp;

namespace
{

AnalysisModel buildIndexedModel(const std::size_t lineCount)
{
    LineIndex index = makeLineIndex(lineCount);
    FieldSummary fieldSummary;

    for (std::size_t indexValue = 0U; indexValue < lineCount; ++indexValue)
    {
        IndexedLine line;
        line.lineNumber = static_cast<std::uint64_t>(indexValue + 1U);
        line.level = indexValue % 4U == 0U ? DetectedLogLevel::Error : DetectedLogLevel::Info;
        line.messageExcerpt = "Error code=" + std::to_string(indexValue % 7U);
        line.contentExcerpt = line.messageExcerpt;
        line.timestamp = Timestamp::fromUnixSeconds(static_cast<std::int64_t>(1000 + indexValue));
        fieldSummary.recordTimestamp(*line.timestamp);
        (void)index.tryAddLine(line);
    }

    LogLevelCounts counts;

    return AnalysisModel(Path("benchmark.log"), lineCount, counts, scope::analysis::LogFormat::PlainText,
                         std::nullopt, fieldSummary, std::move(index));
}

} // namespace

static void BM_AnalyticsEngine(benchmark::State& state)
{
    const AnalysisModel model = buildIndexedModel(10000U);
    const AnalyticsEngine engine;
    const AnalyticsConfig config;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(engine.analyze(model, config).indexedLineCount());
    }

    state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations()) * 10000LL);
}

BENCHMARK(BM_AnalyticsEngine);
