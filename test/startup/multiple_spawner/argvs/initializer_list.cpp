/**
 * @file test/startup/multiple_spawner/argvs/initializer_list.cpp
 * @author Marcel Breyer
 * @date 2020-06-02
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::add_argv(std::initializer_list<std::initializer_list<T>>) and
 *        @ref mpicxx::multiple_spawner::add_argv_at(const std::size_t, std::initializer_list<T>) member
 *        functions provided by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                          | test case description                                                                                                                                                       |
 * |:----------------------------------------|:----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
 * | AddArgvsViaInitializerList              | add command line arguments from a [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) to all executables                                  |
 * | AddArgvsViaInitializerListInvalidSize   | [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) with illegal size while adding command line arguments to all executables (death test) |
 * | AddArgvsAtViaInitializerList            | add command line arguments from a [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) to the i-th executable                              |
 * | AddArgvsAtViaInitializerListOutOfBounce | try adding command line arguments at an out of bounce index                                                                                                                 |
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


TEST(MultipleSpawnerTest, AddArgvsViaInitializerList) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments
    std::vector<std::string> argvs_1 = { "-foo", "bar", "-baz", "qux", "--quux" };
    std::vector<std::string> argvs_2 = { "-bar", "foo", "-qux", "baz", "--foobar" };
    std::vector<std::vector<std::string>> vec = { argvs_1, argvs_2 };

    ms.add_argv({ { "-foo", "bar", "-baz", "qux", "--quux" },
                  { "-bar", "foo", "-qux", "baz", "--foobar" } });

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

TEST(MultipleSpawnerDeathTest, AddArgvsViaInitializerListInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments with different size
    ASSERT_DEATH( ms.add_argv({ { } }) , "");
    ASSERT_DEATH( ms.add_argv({ { }, { }, { } }) , "");
}


TEST(MultipleSpawnerTest, AddArgvsAtViaInitializerList) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // add command line arguments
    std::vector<std::string> argvs_1 = { "-foo", "bar", "-baz", "qux", "--quux" };
    std::vector<std::string> argvs_2 = { "1", "2", "3", "4", "5" };
    std::vector<std::vector<std::string>> vec = { argvs_1, argvs_2 };

    ms.add_argv_at(0, { "-foo", "bar", "-baz", "qux", "--quux" });
    ms.add_argv_at(1, { 1, 2, 3, 4, 5 });


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

TEST(MultipleSpawnerTest, AddArgvsAtViaInitializerListOutOfBounce) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try adding command line arguments at an illegal index
    EXPECT_THROW_WHAT(
            ms.add_argv_at(2, { "foo" } ),
            std::out_of_range,
            "multiple_spawner::add_argv_at(const std::size_t, T&&) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::add_argv_at(const std::size_t, T&&) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            ms.add_argv_at(-1, { "foo" }),
            std::out_of_range,
            expected_msg);
}