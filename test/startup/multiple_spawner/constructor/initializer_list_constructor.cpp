/**
 * @file test/startup/multiple_spawner/constructor/initializer_list_constructor.cpp
 * @author Marcel Breyer
 * @date 2020-05-12
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::multiple_spawner(std::initializer_list<std::pair<std::string, int>>) member function provided by the
 * @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                                   | test case description                                                                        |
 * |:-------------------------------------------------|:---------------------------------------------------------------------------------------------|
 * | ConstructFromInitializerList                     | construct a multiple_spawner object from a [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list)                                    |
 * | ConstructFromInitializerListInvalidCommand       | try to construct a multiple_spawner object with an illegal command name (death test)         |
 * | ConstructFromInitializerListInvalidMaxprocs      | try to construct a multiple_spawner object with an illegal maxprocs number (death test)      |
 * | ConstructFromInitializerListInvalidTotalMaxprocs | ry to construct a multiple_spawner object with an illegal total mayprocs number (death test) |
 */

#include <limits>
#include <utility>

#include <gtest/gtest.h>

#include <mpicxx/startup/multiple_spawner.hpp>

using namespace std::string_literals;


TEST(MultipleSpawnerTest, ConstructFromInitializerList) {
    // create new multiple_spawner using a initializer_list
    mpicxx::multiple_spawner ms({
            { "foo", 1 },
            { "bar"s, 1 }
    });
}

TEST(MultipleSpawnerDeathTest, ConstructFromInitializerListInvalidCommand) {
    // try to create new multiple_spawner with an empty command name
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
    const auto max = mpicxx::multiple_spawner::universe_size().value_or(std::numeric_limits<int>::max() - 4);
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ { "foo", max / 4 + 1 }, { "bar", max / 4 + 1 },
                                                { "baz", max / 4 + 1 }, { "qux", max / 4 + 1 } }) , "");
}