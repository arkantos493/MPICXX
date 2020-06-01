/**
 * @file test/startup/multiple_spawner/argvs/remove.cpp
 * @author Marcel Breyer
 * @date 2020-06-02
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::remove_argv(), @ref mpicxx::multiple_spawner::remove_argv_at(const std::size_t)
 *        and @ref mpicxx::multiple_spawner::remove_argv_at(const std::size_t, const std::size_t) member functions provided by the
 *        @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                | test case description                                                                    |
 * |:------------------------------|:-----------------------------------------------------------------------------------------|
 * | RemoveArgv                    | remove all previously added command line arguments                                       |
 * | RemoveArgvAt                  | remove all previously added command line arguments of one executable                     |
 * | RemoveArgvAtOutOfBounce       | try removing all command line arguments of one executable at an out of bounce index      |
 * | RemoveSingleArgvAt            | remove a previously added command line argument of one executable                        |
 * | RemoveSingleArgvAtOutOfBounce | try removing a single command line arguments of one executable at an out of bounce index |
 */

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <test_utility.hpp>

#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, RemoveArgv) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments
    ms.add_argv({ { "-foo", "bar", "-baz", "qux", "--quux" },
                  { "-bar", "foo", "-qux", "baz", "--foobar" } });

    // remove all command line arguments
    ms.remove_argv();

    // check whether all command line arguments were removed successfully
    ASSERT_EQ(ms.argv().size(), 2);
    for (std::size_t i = 0; i < ms.argv().size(); ++i) {
        EXPECT_EQ(ms.argv_size_at(i), 0);
    }
}

TEST(MultipleSpawnerTest, RemoveArgvAt) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments
    ms.add_argv({ { "-foo", "bar", "-baz", "qux", "--quux" },
                  { "-bar", "foo", "-qux", "baz", "--foobar" } });

    // remove all command line arguments of the first executable
    ms.remove_argv_at(0);

    // check whether the command line arguments of the first executable were removed successfully
    ASSERT_EQ(ms.argv().size(), 2);
    EXPECT_EQ(ms.argv_size_at(0), 0);
    EXPECT_EQ(ms.argv_size_at(1), 5);

    // remove all command line arguments of the second executable
    ms.remove_argv_at(1);

    // check whether the command line arguments of the second executable were removed successfully
    ASSERT_EQ(ms.argv().size(), 2);
    EXPECT_EQ(ms.argv_size_at(0), 0);
    EXPECT_EQ(ms.argv_size_at(1), 0);
}

TEST(MultipleSpawnerTest, RemoveArgvAtOutOfBounce) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try removing command line arguments at an illegal index
    EXPECT_THROW_WHAT(
            ms.remove_argv_at(2),
            std::out_of_range,
            "multiple_spawner::remove_argv_at(const std::size_t) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::remove_argv_at(const std::size_t) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            ms.remove_argv_at(-1),
            std::out_of_range,
            expected_msg);
}

TEST(MultipleSpawnerTest, RemoveSingleArgvAt) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments
    ms.add_argv({ { "-foo", "bar", "-baz", "qux", "--quux" },
                  { "-bar", "foo", "-qux", "baz", "--foobar" } });

    // remove specific command line arguments of the first executable
    ms.remove_argv_at(0, 0);
    ms.remove_argv_at(0, 2);

    // check whether the command line arguments of the first executable were removed successfully
    ASSERT_EQ(ms.argv().size(), 2);
    EXPECT_EQ(ms.argv_size_at(0), 3);
    std::vector<std::string> expected_argvs = { "bar", "-baz", "--quux" };
    for (std::size_t i = 0; i < ms.argv_size_at(0); ++i) {
        EXPECT_EQ(ms.argv_at(0, i), expected_argvs[i]);
    }
    EXPECT_EQ(ms.argv_size_at(1), 5);

    // remove all command line arguments of the second executable
    const std::size_t size = ms.argv_size_at(1);
    for (std::size_t i = 0; i < size; ++i) {
        ms.remove_argv_at(1, 0);
    }

    // check whether the command line arguments of the second executable were removed successfully
    ASSERT_EQ(ms.argv().size(), 2);
    EXPECT_EQ(ms.argv_at(0).size(), 3);
    EXPECT_EQ(ms.argv_at(1).size(), 0);
}

TEST(MultipleSpawnerTest, RemoveSingleArgvAtOutOfBounce) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try removing a command line arguments at an illegal index
    EXPECT_THROW_WHAT(
            ms.remove_argv_at(2, 0),
            std::out_of_range,
            "multiple_spawner::remove_argv_at(const std::size_t, const std::size_t) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::remove_argv_at(const std::size_t, const std::size_t) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            ms.remove_argv_at(-1, 0),
            std::out_of_range,
            expected_msg);

    // add command line arguments
    ms.add_argv_at(0, "foo", "bar", 42);

    // try removing a command line arguments at an illegal index
    EXPECT_THROW_WHAT(
            ms.remove_argv_at(0, 3),
            std::out_of_range,
            "multiple_spawner::remove_argv_at(const std::size_t, const std::size_t) range check: j (which is 3) >= argvs_[0].size() (which is 3)");

    expected_msg =
            fmt::format("multiple_spawner::remove_argv_at(const std::size_t, const std::size_t) range check: "
                        "j (which is {}) >= argvs_[0].size() (which is 3)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            ms.remove_argv_at(0, -1),
            std::out_of_range,
            expected_msg);
}