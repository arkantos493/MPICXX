/**
 * @file test/startup/multiple_spawner/maxprocs/iterator_range.cpp
 * @author Marcel Breyer
 * @date 2020-06-02
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_maxprocs(InputIt, InputIt) member function provided
 *        by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                               | test case description                                                    |
 * |:---------------------------------------------|:-------------------------------------------------------------------------|
 * | SetMaxprocsViaIteratorRange                  | set number of processes from an iterator range                           |
 * | SetMaxprocsViaInvalidIteratorRange           | illegal iterator range (death test)                                      |
 * | SetMaxprocsViaIteratorRangeInvalidSize       | iterator range with illegal size (death test)                            |
 * | SetMaxprocsViaIteratorRangeInvalidValue      | try to set new number of processes with illegal value (death test)       |
 * | SetMaxprocsViaIteratorRangeInvalidTotalValue | try to set new number of processes with illegal total value (death test) |
 */

#include <cstddef>
#include <initializer_list>
#include <limits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, SetMaxprocsViaIteratorRange) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set number of processes
    std::vector<int> vec = { 1, 1 };
    ms.set_maxprocs(vec.begin(), vec.end());

    // check if the values were set correctly
    ASSERT_EQ(ms.command().size(), 2);
    for (std::size_t i = 0; i < 2; ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(ms.maxprocs_at(i), vec[i]);
    }
}

TEST(MultipleSpawnerDeathTest, SetMaxprocsViaInvalidIteratorRange) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // set new number of processes with illegal iterator range
    std::vector<int> vec = { 1, 1 };
    ASSERT_DEATH( ms.set_maxprocs(vec.end(), vec.begin()) , "");
}

TEST(MultipleSpawnerDeathTest, SetMaxprocsViaIteratorRangeInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new number of processes with different size
    std::vector<int> vec = { 1, 1, 1 };
    ASSERT_DEATH( ms.set_maxprocs(vec.begin(), vec.begin() + 1) , "");
    ASSERT_DEATH( ms.set_maxprocs(vec.begin(), vec.end()) , "");
}

TEST(MultipleSpawnerDeathTest, SetMaxprocsViaIteratorRangeInvalidValue) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new number of processes with illegal value
    std::vector<int> vec = { 1, 0, 1, std::numeric_limits<int>::max() };
    ASSERT_DEATH( ms.set_maxprocs(vec.begin(), vec.begin() + 2), "");
    ASSERT_DEATH( ms.set_maxprocs(vec.begin() + 2, vec.end()), "");
}

TEST(MultipleSpawnerDeathTest, SetMaxprocsViaIteratorRangeInvalidTotalValue) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new number of processes with illegal total value
    std::vector<int> vec = { 2, 2 };
    ASSERT_DEATH( ms.set_maxprocs(vec.begin(), vec.end()), "");
}