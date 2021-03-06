/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::add_argv(InputIt, InputIt) and
 *        @ref mpicxx::multiple_spawner::add_argv_at(std::size_t, InputIt, InputIt) member functions provided by the
 *        @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                        | test case description                                                                                |
 * |:--------------------------------------|:-----------------------------------------------------------------------------------------------------|
 * | AddArgvsViaIteratorRange              | add command line arguments from an iterator range to all executables                                 |
 * | AddArgvsViaInvalidIteratorRange       | illegal iterator range while adding command line arguments to all executables (death test)           |
 * | AddArgvsViaIteratorRangeInvalidSize   | iterator range with illegal size while adding command line arguments to all executables (death test) |
 * | AddArgvsAtViaIteratorRange            | add command line arguments from an iterator range to the i-th executable                             |
 * | AddArgvsAtViaIteratorRangeOutOfBounce | try adding command line arguments at an out of bounce index                                          |
 * | AddArgvsAtViaInvalidIteratorRange     | illegal iterator range while adding command line arguments to the i-th executable (death test)       |
 */

#include <mpicxx/startup/multiple_spawner.hpp>
#include <test_utility.hpp>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

TEST(MultipleSpawnerTest, AddArgvsViaIteratorRange) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments
    std::vector<std::string> argvs_1 = { "-foo", "bar", "-baz", "qux", "--quux" };
    std::vector<std::string> argvs_2 = { "-bar", "foo", "-qux", "baz", "--foobar" };
    std::vector<std::vector<std::string>> vec = { argvs_1, argvs_2 };

    ms.add_argv(vec.begin(), vec.end());

    // check if the command line arguments were added correctly
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
    mpicxx::multiple_spawner ms({ {"foo", 1 }, { "bar", 1 } });

    // add command line arguments with illegal iterator range
    std::vector<std::vector<std::string>> vec(2);
    ASSERT_DEATH( ms.add_argv(vec.end(), vec.begin()) , "");
}

TEST(MultipleSpawnerDeathTest, AddArgvsViaIteratorRangeInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments with different size
    std::vector<std::vector<std::string>> vec(3);
    ASSERT_DEATH( ms.add_argv(vec.begin(), vec.begin() + 1) , "");
    ASSERT_DEATH( ms.add_argv(vec.begin(), vec.end()) , "");
}


TEST(MultipleSpawnerTest, AddArgvsAtViaIteratorRange) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments
    std::vector<std::string> argvs_1 = { "-foo", "bar", "-baz", "qux", "--quux" };
    std::vector<std::string> argvs_2 = { "-bar", "foo", "-qux", "baz", "--foobar" };
    std::vector<std::vector<std::string>> vec = { argvs_1, argvs_2 };

    ms.add_argv_at(0, vec[0].begin(), vec[0].end());
    ms.add_argv_at(1, vec[1].begin(), vec[1].end());

    // check if the command line arguments were added correctly
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
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try adding command line arguments at an illegal index
    std::vector<std::string> vec = { "foo" };
    EXPECT_THROW_WHAT(
            ms.add_argv_at(2, vec.begin(), vec.end()),
            std::out_of_range,
            "multiple_spawner::add_argv_at(const std::size_t, T&&) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::add_argv_at(const std::size_t, T&&) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            ms.add_argv_at(-1, vec.begin(), vec.end()),
            std::out_of_range,
            expected_msg);
}

TEST(MultipleSpawnerDeathTest, AddArgvsAtViaInvalidIteratorRange) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try adding command line arguments via an invalid iterator range
    std::vector<std::string> vec = { "foo", "bar" };
    ASSERT_DEATH( ms.add_argv_at(0, vec.end(), vec.begin()) , "");
}