/**
 * @file test/startup/multiple_spawner/argvs/iterator_range.cpp
 * @author Marcel Breyer
 * @date 2020-05-30
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::add_argv(InputIt, InputIt) member and
 *        @ref mpicxx::multiple_spawner::add_argv_at(std::size_t, InputIt, InputIt) function provided by the
 *        @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                        | test case description                                                             |
 * |:--------------------------------------|:----------------------------------------------------------------------------------|
 * | AddArgvsViaIteratorRange              | add argvs from an iterator range to all processes                                 |
 * | AddArgvsViaInvalidIteratorRange       | illegal iterator range while adding argvs to all processes (death test)           |
 * | AddArgvsViaIteratorRangeInvalidSize   | iterator range with illegal size while adding argvs to all processes (death test) |
 * | AddArgvsAtViaIteratorRange            | add argvs from an iterator range to the i-th process                              |
 * | AddArgvsAtViaIteratorRangeOutOfBounce | try adding argvs at an out of bounce index                                        |
 * | AddArgvsAtViaInvalidIteratorRange     | illegal iterator range while adding argvs to the i-th process (death test)        |
 */

#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <test_utility.hpp>

#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, AddArgvsViaIteratorRange) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // add argvs
    std::vector<mpicxx::multiple_spawner::argv_value_type> argvs_1 = { { "-foo", "bar" }, { "-baz", "qux" }, { "--quux", "" } };
    std::vector<mpicxx::multiple_spawner::argv_value_type> argvs_2 = { { "-bar", "foo" }, { "-qux", "baz" }, { "--foobar", "" } };
    std::vector<std::vector<mpicxx::multiple_spawner::argv_value_type>> vec = { argvs_1, argvs_2 };

    ms.add_argv(vec.begin(), vec.end());


    // check if the argvs were added correctly
    ASSERT_EQ(ms.argv().size(), 2);
    for (std::size_t i = 0; i < 2; ++i) {
        SCOPED_TRACE(i);
        for (std::size_t j = 0; j < ms.argv_at(i).size(); ++j) {
            SCOPED_TRACE(j);
            EXPECT_EQ(ms.argv_at(i, j), vec[i][j]);
        }
    }
}

TEST(MultipleSpawnerDeathTest, AddArgvsViaInvalidIteratorRange) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // add argvs with illegal iterator range
    std::vector<std::vector<mpicxx::multiple_spawner::argv_value_type>> vec(2);
    ASSERT_DEATH( ms.add_argv(vec.end(), vec.begin()) , "");
}

TEST(MultipleSpawnerDeathTest, AddArgvsViaIteratorRangeInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // add argvs with different size
    std::vector<std::vector<mpicxx::multiple_spawner::argv_value_type>> vec(3);
    ASSERT_DEATH( ms.add_argv(vec.begin(), vec.begin() + 1) , "");
    ASSERT_DEATH( ms.add_argv(vec.begin(), vec.end()) , "");
}


TEST(MultipleSpawnerTest, AddArgvsAtViaIteratorRange) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // add argvs
    std::vector<mpicxx::multiple_spawner::argv_value_type> argvs_1 = { { "-foo", "bar" }, { "-baz", "qux" }, { "--quux", "" } };
    std::vector<mpicxx::multiple_spawner::argv_value_type> argvs_2 = { { "-bar", "foo" }, { "-qux", "baz" }, { "--foobar", "" } };
    std::vector<std::vector<mpicxx::multiple_spawner::argv_value_type>> vec = { argvs_1, argvs_2 };

    ms.add_argv_at(0, vec[0].begin(), vec[0].end());
    ms.add_argv_at(1, vec[1].begin(), vec[1].end());


    // check if the argvs were added correctly
    ASSERT_EQ(ms.argv().size(), 2);
    for (std::size_t i = 0; i < 2; ++i) {
        SCOPED_TRACE(i);
        for (std::size_t j = 0; j < ms.argv_at(i).size(); ++j) {
            SCOPED_TRACE(j);
            EXPECT_EQ(ms.argv_at(i, j), vec[i][j]);
        }
    }
}

TEST(MultipleSpawnerTest, AddArgvsAtViaIteratorRangeOutOfBounce) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // try adding argvs at an illegal index
    std::vector<mpicxx::multiple_spawner::argv_value_type> vec(1);
    EXPECT_THROW_WHAT(
            ms.add_argv_at(2, vec.begin(), vec.end()),
            std::out_of_range,
            "multiple_spawner::add_argv_at(const std::size_t, std::string, T&&) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::add_argv_at(const std::size_t, std::string, T&&) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            ms.add_argv_at(-1, vec.begin(), vec.end()),
            std::out_of_range,
            expected_msg);
}

TEST(MultipleSpawnerDeathTest, AddArgvsAtViaInvalidIteratorRange) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // try adding argvs via an invalid iterator ramge
    std::vector<mpicxx::multiple_spawner::argv_value_type> vec = { { "foo", "bar" } };
    ASSERT_DEATH( ms.add_argv_at(0, vec.end(), vec.begin()) , "");
}