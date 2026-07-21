/**
 * @file search_benchmark.cpp
 * @brief Performance benchmarks for search queries.
 */

#include <vector>

#include <benchmark/benchmark.h>

#include "line_index.hpp"
#include "search_engine.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::makeLineIndex;
using scope::search::SearchEngine;
using scope::search::SearchQuery;

namespace
{

std::vector<IndexedLine> buildIndexedLines(const std::size_t lineCount)
{
    std::vector<IndexedLine> lines;
    lines.reserve(lineCount);

    for (std::size_t index = 0U; index < lineCount; ++index)
    {
        IndexedLine line;
        line.lineNumber = static_cast<std::uint64_t>(index + 1U);
        line.level = index % 3U == 0U ? DetectedLogLevel::Error : DetectedLogLevel::Info;
        line.contentExcerpt = "2026-07-11 line " + std::to_string(index) + " message payload";
        line.messageExcerpt = line.contentExcerpt;
        lines.push_back(std::move(line));
    }

    return lines;
}

} // namespace

static void BM_SearchEngineTextQuery(benchmark::State& state)
{
    const std::vector<IndexedLine> lines = buildIndexedLines(10000U);
    const SearchQuery query = SearchQuery::term("payload");
    const SearchEngine engine;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(engine.search(lines, query));
    }

    state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations()) *
                            static_cast<std::int64_t>(lines.size()));
}

BENCHMARK(BM_SearchEngineTextQuery);
