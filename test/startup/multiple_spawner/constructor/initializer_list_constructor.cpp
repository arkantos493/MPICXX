/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the
 *        @ref mpicxx::multiple_spawner::multiple_spawner(std::initializer_list<std::string>, std::initializer_list<int>) and
 *        @ref mpicxx::multiple_spawner::multiple_spawner(std::initializer_list<std::pair<std::string, int>>) member
 *        function provided by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                                       | test case description                                                                                                                                                     |
 * |:-----------------------------------------------------|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
 * | ConstructFromInitializerList                         | construct a multiple_spawner object from a [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list)                                          |
 * | ConstructFromInitializerListInvalidName              | try to construct a multiple_spawner object with an illegal executable name (death test)                                                                                   |
 * | ConstructFromInitializerListInvalidMaxprocs          | try to construct a multiple_spawner object with an illegal maxprocs number (death test)                                                                                   |
 * | ConstructFromInitializerListInvalidTotalMaxprocs     | try to construct a multiple_spawner object with an illegal total maxprocs number (death test)                                                                             |
 * | ConstructFromTwoInitializerLists                     | construct a multiple_spawner object from two [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list)                                        |
 * | ConstructFromTwoInitializerListsDifferentSizes       | try to construct a multiple_spawner object from two [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) of different sizes (death test) |
 * | ConstructFromTwoInitializerListsInvalidName          | try to construct a multiple_spawner object with an illegal executable name (death test)                                                                                   |
 * | ConstructFromTwoInitializerListsInvalidMaxprocs      | try to construct a multiple_spawner object with an illegal maxprocs number (death test)                                                                                   |
 * | ConstructFromTwoInitializerListsInvalidTotalMaxprocs | try to construct a multiple_spawner object with an illegal total maxprocs number (death test)                                                                             |
 */

#include <mpicxx/info/runtime_info.hpp>
#include <mpicxx/startup/multiple_spawner.hpp>

#include <gtest/gtest.h>

#include <initializer_list>
#include <limits>
#include <optional>
#include <string>
#include <utility>

using namespace std::string_literals;

TEST(MultipleSpawnerTest, ConstructFromInitializerList) {
    // create new multiple_spawner using a initializer_list
    mpicxx::multiple_spawner ms({
            { "foo", 1 },
            { "bar"s, 1 }
    });
}

TEST(MultipleSpawnerDeathTest, ConstructFromInitializerListInvalidName) {
    // try to create new multiple_spawner with an empty executable name
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ { "", 1 } }) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromInitializerListInvalidMaxprocs) {
    // try to create new multiple_spawner with invalid number of maxprocs
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ { "foo", -1 } }) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ { "foo", 0 } }) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ { "foo", std::numeric_limits<int>::max() } }) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromInitializerListInvalidTotalMaxprocs) {
    // try to create a new multiple_spawner with an invalid total number of maxprocs
    const auto max = mpicxx::universe_size().value_or(std::numeric_limits<int>::max() - 4);
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ { "foo", max / 4 + 1 }, { "bar", max / 4 + 1 },
                                                { "baz", max / 4 + 1 }, { "qux", max / 4 + 1 } }) , "");
}


TEST(MultipleSpawnerTest, ConstructFromTwoInitializerLists) {
    // create new multiple_spawner using two initializer_list
    mpicxx::multiple_spawner ms({ "foo", "bar" }, { 1, 1 });
}

TEST(MultipleSpawnerDeathTest, ConstructFromTwoInitializerListsDifferentSizes) {
    // try creating a new multiple_spawner from two initializer_list of different sizes
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ "foo", "bar" }, { 1 }) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromTwoInitializerListsInvalidName) {
    // try to create new multiple_spawner with an empty executable name
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ "" }, { 1 }) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromTwoInitializerListsInvalidMaxprocs) {
    // try to create new multiple_spawner with invalid number of maxprocs
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ "foo" }, { -1 }) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ "foo" }, { 0 }) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ "foo" }, { std::numeric_limits<int>::max() }) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromTwoInitializerListsInvalidTotalMaxprocs) {
    // try to create a new multiple_spawner with an invalid total number of maxprocs
    const auto max = mpicxx::universe_size().value_or(std::numeric_limits<int>::max() - 4);
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ "foo", "bar", "baz", "qux" }, { max / 4 + 1, max / 4 + 1, max / 4 + 1, max / 4 + 1 }) , "");
}