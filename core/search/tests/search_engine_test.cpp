/**
 * @file search_engine_test.cpp
 * @brief Unit tests for SearchEngine.
 */

#include <gtest/gtest.h>

#include "line_index.hpp"
#include "search_engine.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::makeLineIndex;
using scope::search::CaseSensitivity;
using scope::search::SearchEngine;
using scope::search::SearchMode;
using scope::search::SearchQuery;

namespace
{

IndexedLine makeLine(const std::string& content, const std::string& message = {})
{
    IndexedLine line;
    line.lineNumber = 1U;
    line.level = DetectedLogLevel::Error;
    line.contentExcerpt = content;
    line.messageExcerpt = message.empty() ? content : message;

    return line;
}

} // namespace

TEST(SearchEngineTest, MatchesIndexedLinesByText)
{
    auto index = makeLineIndex();
    index.tryAddLine(makeLine("ERROR connection refused"));
    index.tryAddLine(makeLine("INFO service started"));

    const SearchEngine engine;
    const auto matches = engine.search(index, SearchQuery::term("refused"));

    ASSERT_EQ(1U, matches.size());
    EXPECT_EQ("ERROR connection refused", matches[0].contentExcerpt);
}

TEST(SearchEngineTest, EvaluatesBooleanAndQuery)
{
    auto index = makeLineIndex();
    index.tryAddLine(makeLine("ERROR connection refused"));
    index.tryAddLine(makeLine("ERROR timeout waiting"));
    index.tryAddLine(makeLine("INFO service started"));

    const SearchQuery query = SearchQuery::andQuery(SearchQuery::term("error"), SearchQuery::term("timeout"));
    const SearchEngine engine;
    const auto matches = engine.search(index, query);

    ASSERT_EQ(1U, matches.size());
    EXPECT_NE(std::string::npos, matches[0].contentExcerpt.find("timeout"));
}

TEST(SearchEngineTest, MatchesRegexPatterns)
{
    auto index = makeLineIndex();
    index.tryAddLine(makeLine("ERROR code=500"));
    index.tryAddLine(makeLine("INFO ok"));

    const SearchEngine engine;
    const auto matches =
        engine.search(index, SearchQuery::term("code=\\d+", SearchMode::Regex, CaseSensitivity::Sensitive));

    ASSERT_EQ(1U, matches.size());
}
