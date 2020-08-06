/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::command() const and
 *        @ref mpicxx::multiple_spawner::command_at(const std::size_t) const member function provided by the
 *        @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                   | test case description    |
 * |:---------------------------------|:-------------------------|
 * | GetExecutableNames               | get all executable names |
 * | GetIthExecutableName             | get i-th executable name |
 * | GetIthExecutableNameInvalidIndex | illegal index            |
 */

#include <mpicxx/startup/multiple_spawner.hpp>
#include <test_utility.hpp>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std::string_literals;

TEST(MultipleSpawnerTest, GetExecutableNames) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // check getter for correctness
    std::vector<std::string> vec = ms.command();
    EXPECT_EQ(vec[0], "foo"s);
    EXPECT_EQ(vec[1], "bar"s);
}

TEST(MultipleSpawnerTest, GetIthExecutableName) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // check getter for correctness
    EXPECT_EQ(ms.command_at(0), "foo"s);
    EXPECT_EQ(ms.command_at(1), "bar"s);
}

TEST(MultipleSpawnerTest, GetIthExecutableNameInvalidIndex) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try getting i-th executable name
    [[maybe_unused]] std::string str;
    EXPECT_THROW_WHAT(
            str = ms.command_at(2),
            std::out_of_range,
            "multiple_spawner::command_at(const std::size_t) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::command_at(const std::size_t) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            str = ms.command_at(-1),
            std::out_of_range,
            expected_msg);
}