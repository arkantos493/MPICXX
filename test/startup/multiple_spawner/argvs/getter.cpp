/**
 * @file test/startup/multiple_spawner/argvs/getter.cpp
 * @author Marcel Breyer
 * @date 2020-06-02
 *
 * @brief Test cases for the command line arguments getter member functions provided by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name             | test case description                                                                   |
 * |:---------------------------|:----------------------------------------------------------------------------------------|
 * | GetArgv                    | get all command line arguments                                                          |
 * | GetArgvAt                  | get all command line arguments of one executable                                        |
 * | GetArgvAtOutOfBounce       | try getting all command line arguments of one executable at an out of bounce index      |
 * | GetSingleArgvAt            | get a single command line argument of one executable                                    |
 * | GetSingleArgvAtOutOfBounce | try getting a single command line arguments of one executable at an out of bounce index |
 * | GetArgvSizes               | get the number of command line arguments of all executables                             |
 * | GetArgvSizesAt             | get the number of command line arguments of one executable                              |
 * | GetArgvSizesAtOutOfBounce  | try getting the number of command line arguments of once executable                     |
 */

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


TEST(MultipleSpawnerTest, GetArgv) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments
    std::vector<std::string> argvs_1 = { "-foo", "bar", "-baz", "qux", "--quux" };
    std::vector<std::string> argvs_2 = { "-bar", "foo", "-qux", "baz", "--foobar" };
    std::vector<std::vector<std::string>> argvs = { argvs_1, argvs_2 };

    ms.add_argv(argvs.begin(), argvs.end());

    // get command line arguments
    EXPECT_EQ(ms.argv(), argvs);
}

TEST(MultipleSpawnerTest, GetArgvAt) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments
    std::vector<std::string> argvs_1 = { "-foo", "bar", "-baz", "qux", "--quux" };
    std::vector<std::string> argvs_2 = { "-bar", "foo", "-qux", "baz", "--foobar" };
    std::vector<std::vector<std::string>> argvs = { argvs_1, argvs_2 };

    ms.add_argv(argvs.begin(), argvs.end());

    // get command line arguments
    EXPECT_EQ(ms.argv_at(0), argvs_1);
    EXPECT_EQ(ms.argv_at(1), argvs_2);
}

TEST(MultipleSpawnerTest, GetArgvAtOutOfBounce) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try getting command line arguments at an illegal index
    [[maybe_unused]] std::vector<std::string> vec;
    EXPECT_THROW_WHAT(
            vec = ms.argv_at(2),
            std::out_of_range,
            "multiple_spawner::argv_at(const std::size_t) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::argv_at(const std::size_t) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            vec = ms.argv_at(-1),
            std::out_of_range,
            expected_msg);
}

TEST(MultipleSpawnerTest, GetSingleArgvAt) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments
    std::vector<std::string> argvs_1 = { "-foo", "bar", "-baz", "qux", "--quux" };
    std::vector<std::string> argvs_2 = { "-bar", "foo", "-qux", "baz", "--foobar" };
    std::vector<std::vector<std::string>> argvs = { argvs_1, argvs_2 };

    ms.add_argv(argvs.begin(), argvs.end());

    // get single command line argument
    ASSERT_EQ(ms.argv_size_at(0), 5);
    for (std::size_t i = 0; i < ms.argv_size_at(0); ++i) {
        EXPECT_EQ(ms.argv_at(0, i), argvs_1[i]);
    }
    ASSERT_EQ(ms.argv_size_at(1), 5);
    for (std::size_t i = 0; i < ms.argv_size_at(1); ++i) {
        EXPECT_EQ(ms.argv_at(1, i), argvs_2[i]);
    }
}

TEST(MultipleSpawnerTest, GetSingleArgvAtOutOfBounce) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try getting a command line arguments at an illegal index
    [[maybe_unused]] std::string str;
    EXPECT_THROW_WHAT(
            str = ms.argv_at(2, 0),
            std::out_of_range,
            "multiple_spawner::argv_at(const std::size_t, const std::size_t) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::argv_at(const std::size_t, const std::size_t) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            str = ms.argv_at(-1, 0),
            std::out_of_range,
            expected_msg);

    // add command line arguments
    ms.add_argv_at(0, "foo", "bar", 42);

    // try getting a command line arguments at an illegal index
    EXPECT_THROW_WHAT(
            str = ms.argv_at(0, 3),
            std::out_of_range,
            "multiple_spawner::argv_at(const std::size_t, const std::size_t) range check: j (which is 3) >= argvs_[0].size() (which is 3)");

    expected_msg =
            fmt::format("multiple_spawner::argv_at(const std::size_t, const std::size_t) range check: "
                        "j (which is {}) >= argvs_[0].size() (which is 3)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            str = ms.argv_at(0, -1),
            std::out_of_range,
            expected_msg);
}

TEST(MultipleSpawnerTest, GetArgvSizes) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments
    ms.add_argv({ { "-foo", "bar", "-baz", "qux", "--quux" }, { "-bar", "foo", "-qux" } });

    // get command line argument sizes
    EXPECT_EQ(ms.argv_size()[0], 5);
    EXPECT_EQ(ms.argv_size()[1], 3);
}

TEST(MultipleSpawnerTest, GetArgvSizesAt) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments
    ms.add_argv({ { "-foo", "bar", "-baz", "qux", "--quux" }, { "-bar", "foo", "-qux" } });

    // get command line argument sizes
    EXPECT_EQ(ms.argv_size_at(0), 5);
    EXPECT_EQ(ms.argv_size_at(1), 3);
}

TEST(MultipleSpawnerTest, GetArgvSizesAtOutOfBounce) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try getting command line arguments at an illegal index
    [[maybe_unused]] mpicxx::multiple_spawner::argv_size_type i;
    EXPECT_THROW_WHAT(
            i = ms.argv_size_at(2),
            std::out_of_range,
            "multiple_spawner::argv_size_at(const std::size_t) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::argv_size_at(const std::size_t) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            i = ms.argv_size_at(-1),
            std::out_of_range,
            expected_msg);
}