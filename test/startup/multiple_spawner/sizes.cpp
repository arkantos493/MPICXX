/**
 * @file test/startup/multiple_spawner/sizes.cpp
 * @author Marcel Breyer
 * @date 2020-06-02
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::size() const and @ref mpicxx::multiple_spawner::total_maxprocs() const member
 *        function provided by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name   | test case description          |
 * |:-----------------|:-------------------------------|
 * | GetSize          | set a new root process         |
 * | GetTotalMaxprocs | set a new illegal root process |
 */

#include <initializer_list>
#include <utility>

#include <gtest/gtest.h>

#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, GetSize) {
    // create new multiple_spawner objects
    mpicxx::multiple_spawner ms1({ { "foo", 1 }, { "bar", 1 } });
    mpicxx::multiple_spawner ms2({ { "foo", 1 } });

    // check sizes
    EXPECT_EQ(ms1.size(), 2);
    EXPECT_EQ(ms2.size(), 1);
}

TEST(MultipleSpawnerTest, GetTotalMaxprocs) {
    // create new multiple_spawner objects
    mpicxx::multiple_spawner ms1({ { "foo", 1 }, { "bar", 1 } });
    mpicxx::multiple_spawner ms2({ { "foo", 1 } });
    mpicxx::multiple_spawner ms3({ { "foo", 2 } });

    // check number of total maxprocs
    EXPECT_EQ(ms1.total_maxprocs(), 2);
    EXPECT_EQ(ms2.total_maxprocs(), 1);
    EXPECT_EQ(ms3.total_maxprocs(), 2);
}