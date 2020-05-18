/**
 * @file test/startup/multiple_spawner/constructor/iterator_range_constructor.cpp
 * @author Marcel Breyer
 * @date 2020-05-18
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::multiple_spawner(InputIt, InputIt) member function provided by the
 * @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                                 | test case description                                                                         |
 * |:-----------------------------------------------|:----------------------------------------------------------------------------------------------|
 * | ConstructFromIteratorRange                     | construct a multiple_spawner object from an iterator range                                    |
 * | ConstructFromInvalidIteratorRange              | illegal iterator range (death test)                                                           |
 * | ConstructFromIteratorRangeInvalidName          | try to construct a multiple_spawner object with an illegal executable name (death test)       |
 * | ConstructFromIteratorRangeInvalidMaxprocs      | try to construct a multiple_spawner object with an illegal maxprocs number (death test)       |
 * | ConstructFromIteratorRangeInvalidTotalMaxprocs | try to construct a multiple_spawner object with an illegal total mayprocs number (death test) |
 */

#include <limits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <mpicxx/info/runtime_info.hpp>
#include <mpicxx/startup/multiple_spawner.hpp>

using namespace std::string_literals;


TEST(MultipleSpawnerTest, ConstructFromIteratorRange) {
    // create new multiple_spawner object
    std::vector<std::pair<std::string, int>> vec1 = {
            { "foo", 1 },
            { "bar"s, 1 }
    };
    mpicxx::multiple_spawner ms1(vec1.begin(), vec1.end());

    // create another multiple_spawner object
    std::vector<std::pair<const char*, int>> vec2 = {
            { "foo", 1 },
            { "bar", 1 }
    };
    mpicxx::multiple_spawner ms2(vec2.begin(), vec2.end());
}

TEST(MultipleSpawnerDeathTest, ConstructFromInvalidIteratorRange) {
    // try creating a new multiple_spawner from an invalid iterator range
    std::vector<std::pair<std::string, int>> vec = { { "foo", 1 } };

    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.begin(), vec.begin()) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.end(), vec.begin()) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromIteratorRangeInvalidName) {
    // try to create new multiple_spawner with an empty executable name
    std::vector<std::pair<std::string, int>> vec = { { "", 1 } };
    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.begin(), vec.end()) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromIteratorRangeInvalidMaxprocs) {
    // try to create new multiple_spawner with invalid number of maxprocs
    std::vector<std::pair<std::string, int>> vec = { { "foo", -1 } };
    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.begin(), vec.end()) , "");
    vec[0].second = 0;
    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.begin(), vec.end()) , "");
    vec[0].second = std::numeric_limits<int>::max();
    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.begin(), vec.end()) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromIteratorRangeInvalidTotalMaxprocs) {
    // try to create a new multiple_spawner with an invalid total number of maxprocs
    const auto max = mpicxx::universe_size().value_or(std::numeric_limits<int>::max() - 4);
    std::vector<std::pair<std::string, int>> vec = {
            { "foo", max / 4 + 1 }, { "bar", max / 4 + 1 },
            { "baz", max / 4 + 1 }, { "qux", max / 4 + 1 }
    };
    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.begin(), vec.end()) , "");
}