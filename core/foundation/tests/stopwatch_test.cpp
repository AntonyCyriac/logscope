/**
 * @file stopwatch_test.cpp
 * @brief Unit tests for Stopwatch.
 */

#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::Duration;
using scope::foundation::Stopwatch;

TEST(StopwatchTest, StartsRunning)
{
    const Stopwatch stopwatch;

    EXPECT_TRUE(stopwatch.isRunning());
}

TEST(StopwatchTest, ElapsedIncreasesWhileRunning)
{
    Stopwatch stopwatch;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_GE(stopwatch.elapsed().totalNanoseconds(), 10000000LL);
}

TEST(StopwatchTest, StopFreezesElapsed)
{
    Stopwatch stopwatch;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    stopwatch.stop();

    const Duration first = stopwatch.elapsed();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_EQ(first, stopwatch.elapsed());
    EXPECT_FALSE(stopwatch.isRunning());
}

TEST(StopwatchTest, StartResumesTiming)
{
    Stopwatch stopwatch;

    stopwatch.stop();
    stopwatch.start();

    EXPECT_TRUE(stopwatch.isRunning());
    EXPECT_GE(stopwatch.elapsed().totalNanoseconds(), 0);
}

TEST(StopwatchTest, ResetClearsElapsed)
{
    Stopwatch stopwatch;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    stopwatch.reset();

    EXPECT_LT(stopwatch.elapsed().totalNanoseconds(), 10000000LL);
    EXPECT_TRUE(stopwatch.isRunning());
}

TEST(StopwatchTest, ResetWhileStopped)
{
    Stopwatch stopwatch;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    stopwatch.stop();
    stopwatch.reset();

    EXPECT_EQ(0, stopwatch.elapsed().totalNanoseconds());
    EXPECT_FALSE(stopwatch.isRunning());
}
