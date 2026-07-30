/**
 * @file tailing_file_log_source_test.cpp
 * @brief Tests for live tail file source (M14).
 */

#include <fstream>

#include <gtest/gtest.h>

#include "foundation/path.hpp"
#include "tailing_file_log_source.hpp"

using scope::foundation::Path;
using scope::source::TailingFileLogSource;

TEST(TailingFileLogSourceTest, OpenAtEndReadsAppendedLine)
{
    const Path path("logscope_tail_test.log");
    std::ofstream writer(path.string());
    writer << "line1\n";
    writer.flush();

    const auto tailResult = TailingFileLogSource::openAtEnd(path);

    ASSERT_TRUE(tailResult);

    writer << "line2\n";
    writer.flush();

    std::string line;
    const auto readResult = tailResult->get()->readLine(line);

    ASSERT_TRUE(readResult);
    EXPECT_TRUE(*readResult);
    EXPECT_EQ(line, "line2");

    std::remove(path.string().c_str());
}
