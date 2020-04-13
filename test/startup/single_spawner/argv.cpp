/**
 * @file test/startup/single_spawner/argv.cpp
 * @author Marcel Breyer
 * @date 2020-04-13
 *
 * @brief Test cases for the @ref mpicxx::single_spawner class argv member functions.
 * @details Testsuite: *SingleSpawnerTest*
 * | test case name                   | test case description                                                              |
 * |:---------------------------------|:-----------------------------------------------------------------------------------|
 * | AddArgv                          | add a single argv                                                                  |
 * | AddInvalidArgv                   | add a single and invalid argv (death test)                                         |
 * | AddArgvVbyIteratorRange          | add multiple argvs denote by the iterator range [first, last)                      |
 * | AddInvalidArgvVbyIteratorRange   | add multiple invalid argvs denote by the iterator range [first, last) (death test) |
 * | AddArgvByInitializerList         | add multiple argvs denoted by the initializer list                                 |
 * | AddInvalidArgvByInitializerList  | add multiple invalid argvs denoted by the initializer list (death test)            |
 * | ChainAddArgv                     | chain calls to @ref mpicxx::single_spawner::add_argv()                             |
 * | GetArgv                          | get the current argvs                                                              |
 * | GetSingleArgv                    | get a single argv out of all argvs                                                 |
 * | GetSingleArgvOutOfRangeException | get a single argv out of all argvs out-of-bounce (death test)                      |
 * | GetSize                          | get the number of all argvs                                                        |
 */

#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/startup/single_spawner.hpp>
#include <mpicxx/startup/thread_support.hpp>

using namespace std::string_literals;


TEST(SingleSpawnerTest, AddArgv) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // add argvs
    ss.add_argv("key_1",  "value_1");
    ss.add_argv("key_2"s, "value_2"s);
    ss.add_argv("-key_3", 42);
    ss.add_argv("-key_4"s, mpicxx::thread_support::single);

    // create lookup vector for correct [key, value]-pairs
    std::vector<std::pair<std::string, std::string>> correct_argvs = {
            { "-key_1"s, "value_1"s },
            { "-key_2"s, "value_2"s },
            { "-key_3"s, "42"s },
            { "-key_4"s, "MPI_THREAD_SINGLE"s }
    };

    // check if added argvs are correct
    for (std::size_t i = 0; i < correct_argvs.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(ss.argv(i).first,  correct_argvs[i].first);
        EXPECT_EQ(ss.argv(i).second, correct_argvs[i].second);
    }
}

TEST(SingleSpawnerDeathTest, AddInvalidArgv) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // add invalid argvs
    ASSERT_DEATH( ss.add_argv("-"s, "value"s) , "");
    ASSERT_DEATH( ss.add_argv("", 42) ,"");
}


TEST(SingleSpawnerTest, AddArgvByIteratorRange) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // create vector containing the [key, value]-pairs
    std::vector<std::pair<std::string, std::string>> argvs = {
            { "-key_1"s, "value_1"s },
            { "-key_2"s, "value_2"s },
            { "-key_3"s, "42"s },
            { "-key_4"s, "MPI_THREAD_SINGLE"s }
    };

    // add argvs to the single_spawner object
    ss.add_argv(argvs.begin(), argvs.end());

    // check if added argvs are correct
    for (std::size_t i = 0; i < argvs.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(ss.argv(i).first,  argvs[i].first);
        EXPECT_EQ(ss.argv(i).second, argvs[i].second);
    }


    // create a second single_spawner object
    mpicxx::single_spawner ss_2("a.out", 1);

    // create second vector containing the [key, value]-pairs
    std::vector<std::pair<std::string, int>> argvs_2 = {
            { "-key_1"s, 1 },
            { "-key_2"s, 2 },
            { "-key_3"s, 3 },
            { "-key_4"s, 4 }
    };

    // add argvs to the single_spawner object
    ss_2.add_argv(argvs_2.begin(), argvs_2.end());

    // check if added argvs are correct
    for (std::size_t i = 0; i < argvs_2.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(ss_2.argv(i).first,  argvs_2[i].first);
        EXPECT_EQ(ss_2.argv(i).second, std::to_string(argvs_2[i].second));
    }
}

TEST(SingleSpawnerDeathTest, AddInvalidArgvByIteratorRange) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // create vector containing the [key, value]-pairs
    std::vector<std::pair<std::string, std::string>> argvs = {
            { "-"s, "value"s },
            { ""s, "42"s }
    };

    // add invalid argvs to the single_spawner object
    ASSERT_DEATH( ss.add_argv(argvs.begin(), argvs.end()) , "");
}


TEST(SingleSpawnerTest, AddArgvByInitializerList) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // create vector containing the [key, value]-pairs
    std::vector<std::pair<std::string, std::string>> argvs = {
            { "-key_1"s, "value_1"s },
            { "-key_2"s, "value_2"s }
    };

    // add argvs to the single_spawner object
    ss.add_argv({ std::make_pair("key_1"s, "value_1"s), std::make_pair("-key_2"s, "value_2"s) });

    // check if added argvs are correct
    for (std::size_t i = 0; i < argvs.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(ss.argv(i).first,  argvs[i].first);
        EXPECT_EQ(ss.argv(i).second, argvs[i].second);
    }


    // create a second single_spawner object
    mpicxx::single_spawner ss_2("a.out", 1);

    // create second vector containing the [key, value]-pairs
    std::vector<std::pair<std::string, std::string>> argvs_2 = {
            { "-key_1"s, "1"s },
            { "-key_2"s, "2"s }
    };

    // add argvs to the single_spawner object
    ss_2.add_argv({ std::make_pair("key_1"s, 1), std::make_pair("-key_2"s, 2) });

    // check if added argvs are correct
    for (std::size_t i = 0; i < argvs_2.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(ss_2.argv(i).first,  argvs_2[i].first);
        EXPECT_EQ(ss_2.argv(i).second, argvs_2[i].second);
    }
}

TEST(SingleSpawnerDeathTest, AddInvalidArgvByInitializerList) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // add invalid argvs to the single_spawner object
    ASSERT_DEATH( ss.add_argv({ std::make_pair("-"s, "value"s), std::make_pair(""s, "42"s) }) , "");
}


TEST(SingleSpawnerTest, ChainAddArgv) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // create vector containing the correct [key, value]-pairs
    std::vector<std::pair<std::string, std::string>> correct_argvs = {
            { "-key_1"s, "value_1"s },
            { "-key_2"s, "42"s },
            { "-key_3"s, "84"s },
            { "-key_4"s, "MPI_THREAD_SINGLE"s },
            { "-key_5"s, "MPI_THREAD_MULTIPLE"s },
    };

    std::vector<std::pair<std::string, int>> argvs = { { "key_2", 42 }, { "-key_3", 84 } };
    // add argvs to the single_spawner object using chained calls
    ss.add_argv("key_1", "value_1")
      .add_argv(argvs.begin(), argvs.end())
      .add_argv({ std::make_pair("key_4"s, mpicxx::thread_support::single), std::make_pair("-key_5"s, mpicxx::thread_support::multiple) });

    // check if added argvs are correct
    for (std::size_t i = 0; i < correct_argvs.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(ss.argv(i).first,  correct_argvs[i].first);
        EXPECT_EQ(ss.argv(i).second, correct_argvs[i].second);
    }
}


TEST(SingleSpawnerTest, GetArgv) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // create vector containing correct argvs
    std::vector<std::pair<std::string, std::string>> correct_argvs = {
            { "-key_1"s, "value_1"s },
            { "-key_2"s, "42"s },
            { "-key_3"s, "MPI_THREAD_SINGLE"s },
    };

    // add agvs
    ss.add_argv("key_1", "value_1").add_argv("-key_2", 42).add_argv("key_3", mpicxx::thread_support::single);

    // get vector containing all added argvs
    const auto& added_argvs = ss.argv();

    // check if added argvs are correct
    for (std::size_t i = 0; i < correct_argvs.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(added_argvs[i].first,  correct_argvs[i].first);
        EXPECT_EQ(added_argvs[i].second, correct_argvs[i].second);
    }
}

TEST(SingleSpawnerTest, GetSingleArgv) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // add argv
    ss.add_argv("key", "value");

    // get single argv
    EXPECT_EQ(ss.argv(0).first, "-key"s);
    EXPECT_EQ(ss.argv(0).second, "value"s);
}

TEST(SingleSpawnerTest, GetSingleArgvOutOfRangeException) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // add argv
    ss.add_argv("key", "value");

    try {
        [[maybe_unused]] const std::pair<std::string, std::string>& argv = ss.argv(1);
        FAIL() << "expected std::out_of_range exception";
    } catch(const std::out_of_range& e) {
        EXPECT_STREQ(e.what(), "Out-of-bounce access!: 1 < 1");
    } catch(...) {
        FAIL() << "expected std::out_of_range exception";
    }
}


TEST(SingleSpawnerTest, GetSize) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // get the number of argvs after construction
    EXPECT_EQ(ss.argv_size(), 0);

    // add argvs
    ss.add_argv("-key_1", "value_1");
    ss.add_argv("-key_2", "value_2");
    ss.add_argv("-key_3", "value_3");

    // get the number of added argvs
    EXPECT_EQ(ss.argv_size(), 3);
}