/**
 * @file info/capacity/max_size.cpp
 * @author Marcel Breyer
 * @date 2020-01-29
 *
 * @brief Test cases for the @ref mpicxx::info::max_size() static member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *CapacityTest*
 * | test case name | test case description                                       |
 * |:---------------|:------------------------------------------------------------|
 * | MaxSize        | get max size through an info object                         |
 * | MovedMaxSize   | get max size through an info object in the moved-from state |
 * | StaticMaxSize  | get max_size through a static function call                 |
 */

#include <limits>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(CapacityTest, MaxSize) {
    // create info object
    mpicxx::info info;

    // get the maximum possible number of [key, value]-pairs
    EXPECT_EQ(info.max_size(), std::numeric_limits<typename mpicxx::info::difference_type>::max());
}

TEST(CapacityTest, MovedMaxSize) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // call to max_size() from a moved-from info object is valid
    EXPECT_EQ(info.max_size(), std::numeric_limits<typename mpicxx::info::difference_type>::max());
}

TEST(CapacityTest, StaticMaxSize) {
    // get the maximum possible number of [key, value]-pairs
    EXPECT_EQ(mpicxx::info::max_size(), std::numeric_limits<typename mpicxx::info::difference_type>::max());
}