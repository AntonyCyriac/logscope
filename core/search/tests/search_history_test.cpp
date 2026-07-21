/**
 * @file search_history_test.cpp
 * @brief Unit tests for SearchHistory.
 */

#include <gtest/gtest.h>

#include "search_history.hpp"

using scope::search::SearchHistory;
using scope::search::maxSearchHistoryEntries;

TEST(SearchHistoryTest, SerializesAndDeserializesEntries)
{
    SearchHistory history;
    history.add("error");
    history.add("timeout");

    const SearchHistory restored = SearchHistory::deserialize(history.serialize());

    ASSERT_EQ(2U, restored.entries().size());
    EXPECT_EQ("error", restored.entries()[0]);
    EXPECT_EQ("timeout", restored.entries()[1]);
}

TEST(SearchHistoryTest, CapsHistorySize)
{
    SearchHistory history;

    for (std::size_t index = 0U; index < maxSearchHistoryEntries + 5U; ++index)
    {
        history.add("query-" + std::to_string(index));
    }

    EXPECT_EQ(maxSearchHistoryEntries, history.entries().size());
    EXPECT_EQ("query-5", history.entries().front());
}
