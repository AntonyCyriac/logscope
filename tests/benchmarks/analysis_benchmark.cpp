/**
 * @file analysis_benchmark.cpp
 * @brief Performance benchmarks for log line classification and analysis.
 */

#include <cstdio>
#include <fstream>
#include <string>

#include <benchmark/benchmark.h>

#include "analysis.hpp"
#include "log_line_classifier.hpp"
#include "source.hpp"

using scope::analysis::AnalysisEngine;
using scope::analysis::detectLogLevel;
using scope::foundation::Path;
using scope::source::SourceManager;

namespace
{

constexpr std::size_t kSyntheticLineCount = 100000U;

Path writeSyntheticLog(const std::size_t lineCount)
{
    const Path path("benchmark_synthetic.log");
    std::ofstream stream(path.string());

    for (std::size_t index = 0; index < lineCount; ++index)
    {
        switch (index % 3U)
        {
        case 0U:
            stream << "2024-01-01 INFO service started\n";
            break;
        case 1U:
            stream << "2024-01-01 WARN retry scheduled\n";
            break;
        default:
            stream << "2024-01-01 ERROR connection failed\n";
            break;
        }
    }

    return path;
}

} // namespace

static void BM_DetectLogLevel(benchmark::State& state)
{
    const std::string line = "2024-01-01 ERROR connection failed";

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(detectLogLevel(line));
    }
}

BENCHMARK(BM_DetectLogLevel);

static void BM_DetectLogLevelMixed(benchmark::State& state)
{
    const std::string infoLine = "2024-01-01 INFO service started";
    const std::string warnLine = "2024-01-01 WARN retry scheduled";
    const std::string errorLine = "2024-01-01 ERROR connection failed";
    std::size_t index = 0U;

    for (auto _ : state)
    {
        switch (index % 3U)
        {
        case 0U:
            benchmark::DoNotOptimize(detectLogLevel(infoLine));
            break;
        case 1U:
            benchmark::DoNotOptimize(detectLogLevel(warnLine));
            break;
        default:
            benchmark::DoNotOptimize(detectLogLevel(errorLine));
            break;
        }

        ++index;
    }
}

BENCHMARK(BM_DetectLogLevelMixed);

static void BM_AnalysisEngine(benchmark::State& state)
{
    const Path logPath = writeSyntheticLog(static_cast<std::size_t>(state.range(0)));
    SourceManager sourceManager;
    AnalysisEngine analysisEngine;

    for (auto _ : state)
    {
        auto datasetResult = sourceManager.open(logPath);

        if (!datasetResult.hasValue())
        {
            state.SkipWithError("Failed to open synthetic log");
            return;
        }

        scope::source::SourceDataset dataset = std::move(datasetResult.value());
        const auto analysisResult = analysisEngine.analyze(dataset);

        if (!analysisResult.hasValue())
        {
            state.SkipWithError("Analysis failed");
            return;
        }

        benchmark::DoNotOptimize(*analysisResult);
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
    std::remove(logPath.string().c_str());
}

BENCHMARK(BM_AnalysisEngine)->Arg(static_cast<int>(kSyntheticLineCount));

static void BM_SourceReadLoop(benchmark::State& state)
{
    const Path logPath = writeSyntheticLog(static_cast<std::size_t>(state.range(0)));
    SourceManager sourceManager;

    for (auto _ : state)
    {
        auto datasetResult = sourceManager.open(logPath);

        if (!datasetResult.hasValue())
        {
            state.SkipWithError("Failed to open synthetic log");
            return;
        }

        scope::source::SourceDataset dataset = std::move(datasetResult.value());
        std::string line;
        std::uint64_t lineCount = 0U;

        while (true)
        {
            const auto readResult = dataset.source().readLine(line);

            if (!readResult.hasValue() || !*readResult)
            {
                break;
            }

            ++lineCount;
        }

        benchmark::DoNotOptimize(lineCount);
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
    std::remove(logPath.string().c_str());
}

BENCHMARK(BM_SourceReadLoop)->Arg(static_cast<int>(kSyntheticLineCount));
