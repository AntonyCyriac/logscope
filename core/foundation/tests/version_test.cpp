/**
 * @file version_test.cpp
 * @brief Unit tests for Version.
 */

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::Version;

TEST(VersionTest, DefaultConstruction)
{
    const Version version;

    EXPECT_EQ(0U, version.major());
    EXPECT_EQ(0U, version.minor());
    EXPECT_EQ(0U, version.patch());
}

TEST(VersionTest, ParameterizedConstruction)
{
    const Version version(1, 2, 3);

    EXPECT_EQ(1U, version.major());
    EXPECT_EQ(2U, version.minor());
    EXPECT_EQ(3U, version.patch());
}

TEST(VersionTest, ToString)
{
    const Version version(10, 20, 30);

    EXPECT_EQ("10.20.30", version.toString());
}

TEST(VersionTest, Equality)
{
    const Version lhs(1, 2, 3);
    const Version rhs(1, 2, 3);

    EXPECT_TRUE(lhs == rhs);
    EXPECT_FALSE(lhs != rhs);
}

TEST(VersionTest, Inequality)
{
    const Version lhs(1, 2, 3);
    const Version rhs(1, 2, 4);

    EXPECT_FALSE(lhs == rhs);
    EXPECT_TRUE(lhs != rhs);
}

TEST(VersionTest, LessThan)
{
    EXPECT_TRUE(Version(1, 0, 0) < Version(2, 0, 0));

    EXPECT_TRUE(Version(1, 2, 0) < Version(1, 3, 0));

    EXPECT_TRUE(Version(1, 2, 3) < Version(1, 2, 4));
}

TEST(VersionTest, LessThanOrEqual)
{
    EXPECT_TRUE(Version(1, 2, 3) <= Version(1, 2, 3));

    EXPECT_TRUE(Version(1, 2, 3) <= Version(1, 2, 4));
}

TEST(VersionTest, GreaterThan)
{
    EXPECT_TRUE(Version(2, 0, 0) > Version(1, 9, 9));

    EXPECT_TRUE(Version(1, 3, 0) > Version(1, 2, 9));

    EXPECT_TRUE(Version(1, 2, 4) > Version(1, 2, 3));
}

TEST(VersionTest, GreaterThanOrEqual)
{
    EXPECT_TRUE(Version(1, 2, 3) >= Version(1, 2, 3));

    EXPECT_TRUE(Version(2, 0, 0) >= Version(1, 9, 9));
}

TEST(VersionTest, ZeroVersion)
{
    const Version version;

    EXPECT_EQ("0.0.0", version.toString());
}

TEST(VersionTest, LargeVersion)
{
    const Version version(999, 888, 777);

    EXPECT_EQ("999.888.777", version.toString());
}
