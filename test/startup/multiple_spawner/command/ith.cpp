/**
 * @file test/startup/multiple_spawner/command/ith.cpp
 * @author Marcel Breyer
 * @date 2020-06-02
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_command_at(const std::size_t, T&&) member function provided
 *        by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                   | test case description                                            |
 * |:---------------------------------|:-----------------------------------------------------------------|
 * | SetIthExecutableName             | set the i-th executable name                                     |
 * | SetIthExecutableNameInvalidIndex | illegal index                                                    |
 * | SetIthExecutableNameInvalidName  | try to set new executable name with an invalid name (death test) |
 */

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <test_utility.hpp>

#include <mpicxx/startup/multiple_spawner.hpp>

using namespace std::string_literals;


TEST(MultipleSpawnerTest, SetIthExecutableName) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set i-th executable name
    ms.set_command_at(0, "baz");
    ms.set_command_at(1, "qux");

    // check if names were set correctly
    ASSERT_EQ(ms.command().size(), 2);
    EXPECT_EQ(ms.command_at(0), "baz"s);
    EXPECT_EQ(ms.command_at(1), "qux"s);
}

TEST(MultipleSpawnerTest, SetIthExecutableNameInvalidIndex) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try setting i-th executable name
    EXPECT_THROW_WHAT(
            ms.set_command_at(2, "baz"),
            std::out_of_range,
            "multiple_spawner::set_command_at(const std::size_t, T&&) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::set_command_at(const std::size_t, T&&) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            ms.set_command_at(-1, "baz"),
            std::out_of_range,
            expected_msg);
}

TEST(MultipleSpawnerDeathTest, SetIthExecutableNameInvalidName) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try setting i-th executable name
    ASSERT_DEATH( ms.set_command_at(0, "") , "");
}