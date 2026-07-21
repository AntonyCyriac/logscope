/**
 * @file line_index_test.cpp
 * @brief Unit tests for LineIndex.
 */

#include <gtest/gtest.h>

#include "line_index.hpp"

using scope::analysis::IndexedLine;
using scope::analysis::LineIndex;
using scope::analysis::maxIndexedLines;

TEST(LineIndexTest, StoresLinesUntilCapacity)
{
    LineIndex index;

    for (std::size_t lineNumber = 1U; lineNumber <= maxIndexedLines; ++lineNumber)
    {
        IndexedLine line;
        line.lineNumber = lineNumber;
        EXPECT_TRUE(index.tryAddLine(line));
    }

    IndexedLine overflowLine;
    overflowLine.lineNumber = maxIndexedLines + 1U;
    EXPECT_FALSE(index.tryAddLine(overflowLine));
    EXPECT_EQ(maxIndexedLines, index.indexedLineCount());
    EXPECT_EQ(1U, index.truncatedLineCount());
}
