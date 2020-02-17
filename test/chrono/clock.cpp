/**
 * @file test/chrono/clock.cpp
 * @author Marcel Breyer
 * @date 2020-02-17
 *
 * @brief Test cases for the @ref mpicxx::clock class.
 * @details Testsuite: *ClockTest*
 * | test case name               | test case description                   |
 * |:-----------------------------|:----------------------------------------|
 * |                              |                                         |
 */

#include <gtest/gtest.h>

#include <mpicxx/chrono/clock.hpp>

#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
TEST(ClockTest, ClockTest) {

    std::cout << "Tick Rate: " << mpicxx::clock::resolution() << std::endl;

    auto start = mpicxx::clock::now();
    std::chrono::milliseconds timespan(5050); // or whatever
    std::this_thread::sleep_for(timespan);
    auto end = mpicxx::clock::now();

    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
}