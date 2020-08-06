/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the command line member functions provided by the @ref mpicxx::single_spawner class.
 * @details Testsuite: *SingleSpawnerTest*
 * | test case name                  | test case description                                                                                                                       |
 * |:--------------------------------|:--------------------------------------------------------------------------------------------------------------------------------------------|
 * | AddArgv                         | add command line arguments from a parameter pack                                                                                            |
 * | AddInvalidArgv                  | add invalid command line arguments from a parameter pack (death test)                                                                       |
 * | AddArgvByIteratorRange          | add command line arguments from an iterator range                                                                                           |
 * | AddArgvByInvalidIteratorRange   | illegal iterator range while adding command line arguments (death test)                                                                     |
 * | AddInvalidArgvByIteratorRange   | add invalid command line argument from an iterator range (death test)                                                                       |
 * | AddArgvByInitializerList        | add command line arguments from a [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list)                     |
 * | AddInvalidArgvByInitializerList | add invalid command line argument from a [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) (death test) |
 * | GetArgv                         | get all command line arguments                                                                                                              |
 * | GetSingleArgvAt                 | get a single command line argument                                                                                                          |
 * | GetSingleArgvAtInvalidIndex     | illegal index                                                                                                                               |
 * | GetArgvSize                     | get the number of all command line arguments                                                                                                |
 */

#include <mpicxx/startup/single_spawner.hpp>
#include <mpicxx/startup/thread_support.hpp>
#include <test_utility.hpp>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

using namespace std::string_literals;

TEST(SingleSpawnerTest, AddArgv) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // add command line arguments
    ss.add_argv("foo");
    ss.add_argv("bar"s);
    ss.add_argv("--baz", 42, 3.1415, mpicxx::thread_support::single);

    // create lookup vector for correct command line arguments
    std::vector<std::string> argvs = { "foo", "bar", "--baz", "42", std::to_string(3.1415), "MPI_THREAD_SINGLE" };

    // check if added command line arguments are correct
    for (std::size_t i = 0; i < argvs.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(ss.argv_at(i),  argvs[i]);
    }
}

TEST(SingleSpawnerDeathTest, AddInvalidArgv) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // add invalid command line argument
    ASSERT_DEATH( ss.add_argv("") , "");
}


TEST(SingleSpawnerTest, AddArgvByIteratorRange) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // create vector containing the command line arguments
    std::vector<std::string> argvs = { "foo", "bar", "--baz", "42", std::to_string(3.1415), "MPI_THREAD_SINGLE" };

    // add command line arguments
    ss.add_argv(argvs.begin(), argvs.end());

    // check if added command line arguments are correct
    for (std::size_t i = 0; i < argvs.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(ss.argv_at(i),  argvs[i]);
    }


    // create a second single_spawner object
    mpicxx::single_spawner ss_2("a.out", 1);

    // create second vector containing the command line arguments
    std::vector<int> argvs_2 = { 1, 2, 3, 4, 5 };

    // add command line arguments
    ss_2.add_argv(argvs_2.begin(), argvs_2.end());

    // check if added command line arguments are correct
    for (std::size_t i = 0; i < argvs_2.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(ss_2.argv_at(i),  std::to_string(argvs_2[i]));
    }
}

TEST(SingleSpawnerDeathTest, AddInvalidArgvByIteratorRange) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // create vector containing the invalid command line arguments
    std::vector<std::string> argvs = { "", "" };

    // add invalid command line arguments
    ASSERT_DEATH( ss.add_argv(argvs.begin(), argvs.end()) , "");
}


TEST(SingleSpawnerTest, AddArgvByInitializerList) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // create vector containing the command line arguments
    std::vector<std::string> argvs = { "--foo", "-bar", "baz", };

    // add command line arguments
    ss.add_argv({ "--foo", "-bar", "baz" });

    // check if added command line arguments are correct
    for (std::size_t i = 0; i < argvs.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(ss.argv_at(i),  argvs[i]);
    }


    // create a second single_spawner object
    mpicxx::single_spawner ss_2("a.out", 1);

    // create second vector containing the command line arguments
    std::vector<std::string> argvs_2 = { "1", "2", "3", "4" };

    // add command line arguments
    ss_2.add_argv({ 1, 2, 3, 4 });

    // check if added command line arguments are correct
    for (std::size_t i = 0; i < argvs_2.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(ss_2.argv_at(i),  argvs_2[i]);
    }
}

TEST(SingleSpawnerDeathTest, AddInvalidArgvByInitializerList) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // add invalid command line arguments
    ASSERT_DEATH( ss.add_argv({ "", "" }) , "");
}


TEST(SingleSpawnerTest, GetArgv) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // create vector containing correct command line arguments
    std::vector<std::string> argvs = { "--foo", "-bar", "baz" };

    // add command line arguments
    ss.add_argv(argvs.begin(), argvs.end());

    // get vector containing all added command line arguments
    const auto& added_argvs = ss.argv();

    // check if added command line arguments are correct
    for (std::size_t i = 0; i < argvs.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(added_argvs[i],  argvs[i]);
    }
}

TEST(SingleSpawnerTest, GetSingleArgv) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // add command line arguments
    ss.add_argv("--foo", "-bar", "baz");

    // get single command line argument
    EXPECT_EQ(ss.argv_at(0), "--foo"s);
    EXPECT_EQ(ss.argv_at(2), "baz"s);
}

TEST(SingleSpawnerTest, GetSingleArgvOutOfRangeException) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // add command line argument
    ss.add_argv("--foo", "-bar", "baz");

    // try getting i-th command line argument
    [[maybe_unused]] std::string str;
    EXPECT_THROW_WHAT(
            str = ss.argv_at(3),
            std::out_of_range,
            "single_spawner::argv_at(const std::size_t) range check: i (which is 3) >= argvs_.size() (which is 3)");

    std::string expected_msg =
            fmt::format("single_spawner::argv_at(const std::size_t) range check: "
                        "i (which is {}) >= argvs_.size() (which is 3)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            str = ss.argv_at(-1),
            std::out_of_range,
            expected_msg);
}


TEST(SingleSpawnerTest, GetSize) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // get the number of command line arguments after construction
    EXPECT_EQ(ss.argv_size(), 0);

    // add command line arguments
    ss.add_argv("--foo", "-bar", "baz");

    // get the number of added command line arguments
    EXPECT_EQ(ss.argv_size(), 3);
}