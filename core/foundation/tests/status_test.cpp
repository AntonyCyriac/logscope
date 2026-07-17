#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::Status;

TEST(StatusTest, Success)
{
    EXPECT_EQ(Status::Success, Status::Success);
}

TEST(StatusTest, Failure)
{
    EXPECT_EQ(Status::Failure, Status::Failure);
}
