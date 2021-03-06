/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::multiple_spawner(T&&...) member function provided by the
 *        @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                                 | test case description                                                                         |
 * |:-----------------------------------------------|:----------------------------------------------------------------------------------------------|
 * | ConstructFromParameterPack                     | construct a multiple_spawner object from an parameter pack (variadic template)                |
 * | ConstructFromParameterPackInvalidName          | try to construct a multiple_spawner object with an illegal executable name (death test)       |
 * | ConstructFromParameterPackInvalidMaxprocs      | try to construct a multiple_spawner object with an illegal maxprocs number (death test)       |
 * | ConstructFromParameterPackInvalidTotalMaxprocs | try to construct a multiple_spawner object with an illegal total maxprocs number (death test) |
 */

#include <mpicxx/startup/multiple_spawner.hpp>

#include <gtest/gtest.h>

#include <initializer_list>
#include <limits>
#include <optional>
#include <string>
#include <utility>

TEST(MultipleSpawnerTest, ConstructFromParameterPack) {
    // create new multiple_spawner using variadic templates
    std::pair<std::string, int> pair = std::make_pair("bar", 1);
    mpicxx::multiple_spawner ms(std::make_pair( "foo", 1 ), pair);
}

TEST(MultipleSpawnerDeathTest, ConstructFromParameterPackInvalidName) {
    // try to create new multiple_spawner with an empty executable name
    ASSERT_DEATH( mpicxx::multiple_spawner ms(std::make_pair("", 1)) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromParameterPackInvalidMaxprocs) {
    // try to create new multiple_spawner with invalid number of maxprocs
    ASSERT_DEATH( mpicxx::multiple_spawner ms(std::make_pair("foo", -1)) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms(std::make_pair("foo", 0)) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms(std::make_pair("foo", std::numeric_limits<int>::max())) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromParameterPackInvalidTotalMaxprocs) {
    // try to create a new multiple_spawner with an invalid total number of maxprocs
    const auto max = mpicxx::universe_size().value_or(std::numeric_limits<int>::max() - 4);
    ASSERT_DEATH( mpicxx::multiple_spawner ms(std::make_pair("foo", max / 4 + 1), std::make_pair("bar", max / 4 + 1),
                          std::make_pair("baz", max / 4 + 1), std::make_pair("qux", max / 4 + 1)) , "");
}