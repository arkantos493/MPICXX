/**
 * @file test/startup/multiple_spawner/argvs/parameter_pack.cpp
 * @author Marcel Breyer
 * @date 2020-06-02
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::add_argv(T&&...) and
 *        @ref mpicxx::multiple_spawner::add_argv_at(const std::size_t, T&&...) member
 *        functions provided by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                        | test case description                                                                                |
 * |:--------------------------------------|:-----------------------------------------------------------------------------------------------------|
 * | AddArgvsViaParameterPack              | add command line arguments from a parameter pack to all executables                                  |
 * | AddArgvsViaParameterPackSize          | parameter pack with illegal size while adding command line arguments to all executables (death test) |
 * | AddArgvsAtViaParameterPack            | add command line arguments from a parameter pack to the i-th executable                              |
 * | AddArgvsAtViaParameterPackOutOfBounce | try adding command line arguments at an out of bounce index                                          |
 */

#include <array>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <test_utility.hpp>

#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, AddArgvsViaParameterPack) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments
    std::string argvs_1[] = { "-foo", "bar", "-baz", "qux", "--quux" };
    std::array<int, 3> argvs_2 = { 0, 1, 2 };

    ms.add_argv(argvs_1, argvs_2);

    // check if the command line arguments were added correctly
    ASSERT_EQ(ms.argv().size(), 2);
    for (std::size_t i = 0; i < ms.argv_at(0).size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(ms.argv_at(0, i), argvs_1[i]);
    }
    for (std::size_t i = 0; i < ms.argv_at(1).size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(ms.argv_at(1, i), std::to_string(argvs_2[i]));
    }
}

TEST(MultipleSpawnerDeathTest, AddArgvsViaParameterPackInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // add command line arguments with different size
    std::vector<std::string> vec_1, vec_2, vec_3;
    ASSERT_DEATH( ms.add_argv(vec_1) , "");
    ASSERT_DEATH( ms.add_argv(vec_1, vec_2, vec_3) , "");
}


TEST(MultipleSpawnerTest, AddArgvsAtViaParameterPack) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // add command line arguments
    std::vector<std::string> argvs = { "-foo", "bar", "-baz", "qux", "--quux", "-bar", "0", std::to_string(3.1415), "--foobar", "2" };

    ms.add_argv_at(0, "-foo", "bar", "-baz", "qux", "--quux");
    ms.add_argv_at(1, "-bar", 0, 3.1415, "--foobar", '2');

    // check if the command line arguments were added correctly
    ASSERT_EQ(ms.argv().size(), 2);
    for (std::size_t i = 0; i < argvs.size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(ms.argv_at(i / 5, i % 5), argvs[i]);
    }
}

TEST(MultipleSpawnerTest, AddArgvsAtViaParameterPackOutOfBounce) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // try adding command line arguments at an illegal index
    EXPECT_THROW_WHAT(
            ms.add_argv_at(2, "foo"),
            std::out_of_range,
            "multiple_spawner::add_argv_at(const std::size_t, T&&) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::add_argv_at(const std::size_t, T&&) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            ms.add_argv_at(-1, "foo"),
            std::out_of_range,
            expected_msg);
}