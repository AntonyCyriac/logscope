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

namespace
{

constexpr auto kSleepDuration = std::chrono::milliseconds(50);

} // namespace

TEST(StopwatchTest, StartsRunning)
{
    const Stopwatch stopwatch;

    EXPECT_TRUE(stopwatch.isRunning());
}

TEST(StopwatchTest, ElapsedIncreasesWhileRunning)
{
    Stopwatch stopwatch;

    const std::int64_t elapsedBeforeSleep = stopwatch.elapsed().totalNanoseconds();

    std::this_thread::sleep_for(kSleepDuration);

    EXPECT_GT(stopwatch.elapsed().totalNanoseconds(), elapsedBeforeSleep);
}

TEST(StopwatchTest, StopFreezesElapsed)
{
    Stopwatch stopwatch;

    std::this_thread::sleep_for(kSleepDuration);
    stopwatch.stop();

    const Duration first = stopwatch.elapsed();

    std::this_thread::sleep_for(kSleepDuration);

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

    std::this_thread::sleep_for(kSleepDuration);

    const std::int64_t elapsedBeforeReset = stopwatch.elapsed().totalNanoseconds();

    stopwatch.reset();

    EXPECT_LT(stopwatch.elapsed().totalNanoseconds(), elapsedBeforeReset);
    EXPECT_TRUE(stopwatch.isRunning());
}

TEST(StopwatchTest, ResetWhileStopped)
{
    Stopwatch stopwatch;

    std::this_thread::sleep_for(kSleepDuration);
    stopwatch.stop();
    stopwatch.reset();

    EXPECT_EQ(0, stopwatch.elapsed().totalNanoseconds());
    EXPECT_FALSE(stopwatch.isRunning());
}
