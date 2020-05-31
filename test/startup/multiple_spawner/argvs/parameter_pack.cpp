/**
 * @file test/startup/multiple_spawner/argvs/initializer_list.cpp
 * @author Marcel Breyer
 * @date 2020-05-31
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::add_argv(T&&...) and
 *        @ref mpicxx::multiple_spawner::add_argv_at(const std::size_t, T&&...) member
 *        functions provided by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                        | test case description                                                             |
 * |:--------------------------------------|:----------------------------------------------------------------------------------|
 * | AddArgvsViaParameterPack              | add argvs from a parameter pack to all processes                                  |
 * | AddArgvsViaParameterPackSize          | parameter pack with illegal size while adding argvs to all processes (death test) |
 * | AddArgvsAtViaParameterPack            | add argvs from a parameter pack to the i-th process                               |
 * | AddArgvsAtViaParameterPackOutOfBounce | try adding argvs at an out of bounce index                                        |
 */

#include <array>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <test_utility.hpp>

#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, AddArgvsViaParameterPack) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // add argvs
    mpicxx::multiple_spawner::argv_value_type argvs_1[] = { { "-foo", "bar" }, { "-baz", "qux" }, { "--quux", "" } };
    std::array<std::pair<std::string, int>, 3> argvs_2 = { { { "-bar", 0 }, { "-qux", 1 }, { "--foobar", 2 } } };

    ms.add_argv(argvs_1, argvs_2);

    // check if the argvs were added correctly
    ASSERT_EQ(ms.argv().size(), 2);
    for (std::size_t i = 0; i < ms.argv_at(0).size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(ms.argv_at(0, i), argvs_1[i]);
    }
    for (std::size_t i = 0; i < ms.argv_at(1).size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(ms.argv_at(1, i).first, argvs_2[i].first);
        EXPECT_EQ(ms.argv_at(1, i).second, std::to_string(argvs_2[i].second));
    }
}

TEST(MultipleSpawnerDeathTest, AddArgvsViaParameterPackInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // add argvs with different size
    std::vector<mpicxx::multiple_spawner::argv_value_type> vec_1, vec_2, vec_3;
    ASSERT_DEATH( ms.add_argv(vec_1) , "");
    ASSERT_DEATH( ms.add_argv(vec_1, vec_2, vec_3) , "");
}


TEST(MultipleSpawnerTest, AddArgvsAtViaParameterPack) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // add argvs
    std::vector<mpicxx::multiple_spawner::argv_value_type> argvs =
            { { "-foo", "bar" }, { "-baz", "qux" }, { "--quux", "" },
              { "-bar", "0" }, { "-qux", std::to_string(3.1415) }, { "--foobar", "2" } };

    ms.add_argv_at(0, std::make_pair( "-foo", "bar"), std::make_pair("-baz", "qux"), std::make_pair("--quux", ""));
    ms.add_argv_at(1, std::make_pair("-bar", 0), std::make_pair("-qux", 3.1415), std::make_pair("--foobar", '2'));


    // check if the argvs were added correctly
    ASSERT_EQ(ms.argv().size(), 2);
    for (std::size_t i = 0; i < argvs.size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(ms.argv_at(i / 3, i % 3), argvs[i]);
    }
}

TEST(MultipleSpawnerTest, AddArgvsAtViaParameterPackOutOfBounce) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // try adding argvs at an illegal index
    EXPECT_THROW_WHAT(
            ms.add_argv_at(2, std::make_pair("foo", "bar")),
            std::out_of_range,
            "multiple_spawner::add_argv_at(const std::size_t, std::string, T&&) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::add_argv_at(const std::size_t, std::string, T&&) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            ms.add_argv_at(-1, std::make_pair("foo", "bar")),
            std::out_of_range,
            expected_msg);
}