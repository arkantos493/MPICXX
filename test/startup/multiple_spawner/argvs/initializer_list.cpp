/**
 * @file test/startup/multiple_spawner/argvs/initializer_list.cpp
 * @author Marcel Breyer
 * @date 2020-05-31
 *
 * @brief Test cases for the
 *        @ref mpicxx::multiple_spawner::add_argv(std::initializer_list<std::initializer_list<std::pair<std::string, std::string>>>) and
 *        @ref mpicxx::multiple_spawner::add_argv_at(const std::size_t, std::initializer_list<std::pair<std::string, std::string>>) member
 *        functions provided by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                          | test case description                      |
 * |:----------------------------------------|:-------------------------------------------|
 * | AddArgvsViaInitializerList              | add argvs from a [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) to all processes |
 * | AddArgvsViaInitializerListInvalidSize   | [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) with illegal size while adding argvs to all processes (death test) |
 * | AddArgvsAtViaInitializerList            | add argvs from a [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) to the i-th process |
 * | AddArgvsAtViaInitializerListOutOfBounce | try adding argvs at an out of bounce index |
 */

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <test_utility.hpp>

#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, AddArgvsViaInitializerList) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // add argvs
    std::vector<mpicxx::multiple_spawner::argv_value_type> argvs_1 = { { "-foo", "bar" }, { "-baz", "qux" }, { "--quux", "" } };
    std::vector<mpicxx::multiple_spawner::argv_value_type> argvs_2 = { { "-bar", "foo" }, { "-qux", "baz" }, { "--foobar", "" } };
    std::vector<std::vector<mpicxx::multiple_spawner::argv_value_type>> vec = { argvs_1, argvs_2 };

    ms.add_argv({ { { "-foo", "bar" }, { "-baz", "qux" }, { "--quux", "" } },
                  { { "-bar", "foo" }, { "-qux", "baz" }, { "--foobar", "" } } });


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

TEST(MultipleSpawnerDeathTest, AddArgvsViaInitializerListInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // add argvs with different size
    ASSERT_DEATH( ms.add_argv({ { } }) , "");
    ASSERT_DEATH( ms.add_argv({ { }, { }, { } }) , "");
}


TEST(MultipleSpawnerTest, AddArgvsAtViaInitializerList) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // add argvs
    std::vector<mpicxx::multiple_spawner::argv_value_type> argvs_1 = { { "-foo", "bar" }, { "-baz", "qux" }, { "--quux", "" } };
    std::vector<mpicxx::multiple_spawner::argv_value_type> argvs_2 = { { "-bar", "foo" }, { "-qux", "baz" }, { "--foobar", "" } };
    std::vector<std::vector<mpicxx::multiple_spawner::argv_value_type>> vec = { argvs_1, argvs_2 };

    ms.add_argv_at(0, { { "-foo", "bar" }, { "-baz", "qux" }, { "--quux", "" } });
    ms.add_argv_at(1, { { "-bar", "foo" }, { "-qux", "baz" }, { "--foobar", "" } });


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

TEST(MultipleSpawnerTest, AddArgvsAtViaInitializerListOutOfBounce) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // try adding argvs at an illegal index
    EXPECT_THROW_WHAT(
            ms.add_argv_at(2, { { "foo", "bar" } }),
            std::out_of_range,
            "multiple_spawner::add_argv_at(const std::size_t, std::string, T&&) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::add_argv_at(const std::size_t, std::string, T&&) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            ms.add_argv_at(-1, { { "foo", "bar" } }),
            std::out_of_range,
            expected_msg);
}