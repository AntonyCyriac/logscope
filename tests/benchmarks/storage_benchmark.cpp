/**
 * @file storage_benchmark.cpp
 * @brief Performance benchmarks for SQLite index storage and query pushdown.
 */

#include <benchmark/benchmark.h>

#include <filesystem>
#include <fstream>

#include "index_fingerprint.hpp"
#include "line_index.hpp"
#include "line_index.hpp"
#include "log_line_classifier.hpp"
#include "query_evaluator.hpp"
#include "query_parser.hpp"
#include "query_planner.hpp"
#include "sqlite_index_store.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LogFormat;
using scope::foundation::Path;
using scope::query::QueryEvaluator;
using scope::query::parseFilterQuery;
using scope::storage::IndexFingerprint;
using scope::storage::IndexMetadata;
using scope::storage::SqliteIndexStore;
using scope::storage::planQueryPushdown;

namespace
{

IndexedLine makeLine(const std::uint64_t lineNumber, const DetectedLogLevel level, const std::string& message)
{
    IndexedLine line;
    line.lineNumber = lineNumber;
    line.level = level;
    line.messageExcerpt = message;
    line.contentExcerpt = message;

    return line;
}

Path writeBenchmarkSource(const std::string& name)
{
    const Path sourcePath(name);
    std::ofstream output(sourcePath.string());
    output << "benchmark source\n";
    output.close();

    return sourcePath;
}

Path createBenchmarkIndex(const std::size_t lineCount)
{
    const Path databasePath("storage_benchmark_index.db");
    const Path sourcePath = writeBenchmarkSource("storage_benchmark_source.log");
    const auto fingerprint = IndexFingerprint::compute(sourcePath);

    if (!fingerprint)
    {
        return databasePath;
    }

    IndexMetadata metadata;
    metadata.fingerprint = fingerprint->value();
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    const auto created = SqliteIndexStore::create(databasePath, metadata);

    if (!created)
    {
        return databasePath;
    }

    for (std::size_t indexValue = 0U; indexValue < lineCount; ++indexValue)
    {
        const DetectedLogLevel level = indexValue % 4U == 0U ? DetectedLogLevel::Error : DetectedLogLevel::Info;
        const std::string message = "Error code=" + std::to_string(indexValue % 7U);
        (void)(*created)->appendLine(makeLine(static_cast<std::uint64_t>(indexValue + 1U), level, message), message);
    }

    (void)(*created)->finalize(static_cast<std::uint64_t>(lineCount));

    return databasePath;
}

} // namespace

static void BM_IndexStoreAppend(benchmark::State& state)
{
    const Path databasePath("storage_benchmark_append.db");
    const Path sourcePath = writeBenchmarkSource("storage_benchmark_append_source.log");
    const auto fingerprint = IndexFingerprint::compute(sourcePath);

    if (!fingerprint)
    {
        state.SkipWithError("Failed to compute fingerprint");

        return;
    }

    IndexMetadata metadata;
    metadata.fingerprint = fingerprint->value();
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    for (auto _ : state)
    {
        std::error_code error;
        std::filesystem::remove(databasePath.string(), error);

        const auto created = SqliteIndexStore::create(databasePath, metadata);

        if (!created)
        {
            state.SkipWithError("Failed to create index store");

            return;
        }

        for (std::int64_t indexValue = 0; indexValue < state.range(0); ++indexValue)
        {
            const std::string message = "line " + std::to_string(indexValue);
            (void)(*created)->appendLine(
                makeLine(static_cast<std::uint64_t>(indexValue + 1), DetectedLogLevel::Info, message), message);
        }

        (void)(*created)->finalize(static_cast<std::uint64_t>(state.range(0)));
        benchmark::DoNotOptimize(created);
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}

static void BM_QueryPushdown(benchmark::State& state)
{
    constexpr std::size_t lineCount = 10000U;
    const Path databasePath = createBenchmarkIndex(lineCount);
    const auto opened = SqliteIndexStore::open(databasePath);
    const auto parsed = parseFilterQuery(R"(level == ERROR AND contains(message, "code=3"))");
    const auto plan = parsed ? planQueryPushdown(*parsed) : std::nullopt;

    if (!opened || !parsed || !plan)
    {
        state.SkipWithError("Failed to prepare benchmark input");

        return;
    }

    const QueryEvaluator evaluator(*parsed);

    for (auto _ : state)
    {
        const auto pushed = (*opened)->fetchLinesWhere(plan->sqlWhere);
        benchmark::DoNotOptimize(pushed);
    }
}

static void BM_QueryEvaluatorScan(benchmark::State& state)
{
    constexpr std::size_t lineCount = 10000U;
    const Path databasePath = createBenchmarkIndex(lineCount);
    const auto opened = SqliteIndexStore::open(databasePath);
    const auto parsed = parseFilterQuery(R"(level == ERROR AND contains(message, "code=3"))");

    if (!opened || !parsed)
    {
        state.SkipWithError("Failed to prepare benchmark input");

        return;
    }

    const auto lines = (*opened)->fetchAllLines();

    if (!lines)
    {
        state.SkipWithError("Failed to fetch lines");

        return;
    }

    const QueryEvaluator evaluator(*parsed);

    for (auto _ : state)
    {
        std::size_t matches = 0U;

        for (const IndexedLine& line : *lines)
        {
            if (evaluator.matches(line))
            {
                ++matches;
            }
        }

        benchmark::DoNotOptimize(matches);
    }
}

BENCHMARK(BM_IndexStoreAppend)->Arg(1000)->Arg(100000);
BENCHMARK(BM_QueryPushdown);
BENCHMARK(BM_QueryEvaluatorScan);
