/**
 * @file test/chrono/clock.cpp
 * @author Marcel Breyer
 * @date 2020-02-18
 *
 * @brief Test cases for the @ref mpicxx::clock class.
 * @details Testsuite: *ClockTest*
 * | test case name | test case description                      |
 * |:---------------|:-------------------------------------------|
 * | Now            | check the static `now()` function          |
 * | Resolution     | check the static `resolution()` function   |
 * | Synchronized   | check the static `synchronized()` function |
 */

#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include <mpicxx/chrono/clock.hpp>


TEST(ClockTest, Now) {
    // get current wall-clock time
    auto start = mpicxx::clock::now();

    // wait for 1s
    std::chrono::milliseconds timespan(1000);
    std::this_thread::sleep_for(timespan);

    // get current wall-clock time
    auto end = mpicxx::clock::now();

    // the duration in ms should be at least 1000
    EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(), 1000);
}

TEST(ClockTest, Resolution) {
    EXPECT_LE(mpicxx::clock::resolution(), 1.0);
}

TEST(ClockTest, Synchronized) {
    EXPECT_FALSE(mpicxx::clock::synchronized());
}