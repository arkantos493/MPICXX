/**
 * @file test/startup/spawner.cpp
 * @author Marcel Breyer
 * @date 2020-04-09
 *
 * @brief Test cases for the @ref mpicxx::single_spawner class wrapping the *MPI_COMM_SPAWN* function.
 * @details Testsuite: *StartupTest*
 * | test case name           | test case description                                                                                    |
 * |:-------------------------|:---------------------------------------------------------------------------------------------------------|
 * | Command                  | check whether `command` is set correctly                                                                 |
 * | EmptyCommand             | create spawner with empty `command` (death test)                                                         |
 * | Maxprocs                 | check whether `maxprocs` is set correctly                                                                |
 * | IllegalMaxprocs          | create spawner with illegal `maxprocs` (death test)                                                      |
 * | NumberOfProcessesSpawned | only allowed to query the information after a call to @ref mpicxx::single_spawner::spawn() (death test)  |
 * | MaxprocsProcessesSpawned | only allowed to query the information after a call to @ref mpicxx::single_spawner::spawn() (death test)  |
 * | UniverseSize             | check whether the universe size is correct                                                               |
 * | SetInfo                  | check whether the info object is set correctly                                                           |
 * | SetRoot                  | check whether the root is set correctly                                                                  |
 * | SetIllegalRoot           | set an illegal root value (death test)                                                                   |
 * | SetCommunicator          | check whether the communicator is set correctly                                                          |
 * | SetIllegalCommunicator   | set an illegal communicator (death test)                                                                 |
 * | AddingArgv               | check whether adding argvs works                                                                         |
 * | OutOfBounceArgv          | request a non-existing argv (index out of bounce)                                                        |
 * | ChainingCalls            | test the chaining of calls                                                                               |
 * | GetIntercommunicator     | only allowed to query the information after a call to @ref mpicxx::single_spawner::spawn() (death test)  |
 * | GetErrcodes              | only allowed to query the information after a call to @ref mpicxx::single_spawner::spawn() (death test)  |
 * | PrintErrcodesTo          | printing errcodes strings only allowed after a call to @ref mpicxx::single_spawner::spawn() (death test) |
 */

#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <mpicxx/startup/spawner.hpp>
#include <mpicxx/startup/thread_support.hpp>

using namespace std::string_literals;


TEST(StartupTest, Command) {
    // create spawner
    mpicxx::spawner sp("a.out", 4);

    EXPECT_EQ(sp.command(), "a.out"s);
}

TEST(StartupDeathTest, EmptyCommand) {
    // create spawner with empty command
    ASSERT_DEATH( mpicxx::spawner("", 2) , "");
}

TEST(StartupTest, Maxprocs) {
    // create spawner
    mpicxx::spawner sp("a.out", 4);

    EXPECT_EQ(sp.maxprocs(), 4);
}

TEST(StartupDeathTest, IllegalMaxprocs) {
    // create spawner with illegal maxprocs values
    ASSERT_DEATH( mpicxx::spawner("a.out", -1) , "");
    ASSERT_DEATH( mpicxx::spawner("a.out", std::numeric_limits<int>::max()) , "");
}

TEST(StartupDeathTest, NumberOfProcessesSpawned) {
    // call function before spawn()
    mpicxx::spawner sp("a.out", 4);

    [[maybe_unused]] int number;
    ASSERT_DEATH( number = sp.number_of_spawned_processes() , "");
}

TEST(StartupDeathTest, MaxprocsProcessesSpawned) {
    // call function before spawn()
    mpicxx::spawner sp("a.out", 4);

    [[maybe_unused]] bool flag;
    ASSERT_DEATH( flag = sp.maxprocs_processes_spanwed() , "");
}

TEST(StartupTest, UniverseSize) {
    // test universe size
    void* size;
    int flag;
    MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_UNIVERSE_SIZE, &size, &flag);
    ASSERT_TRUE(flag);
    mpicxx::spawner sp("a.out", 4);
    EXPECT_EQ(sp.universe_size(), *reinterpret_cast<int*>(size));
}

TEST(StartupTest, SetInfo) {
    // create spawn object and set info object
    mpicxx::spawner sp("a.out", 4);
    sp.set_spawn_info(mpicxx::info::env);

    // check if it has been set correctly
    EXPECT_EQ(sp.spawn_info(), mpicxx::info::env);
}

TEST(StartupTest, SetRoot) {
    // create spawn object and set a legal root
    mpicxx::spawner sp("a.out", 4);
    sp.set_root(0);

    // check if it has been set correctly
    EXPECT_EQ(sp.root(), 0);
}

TEST(StartupDeathTest, SetIllegalRoot) {
    // create spawn object
    mpicxx::spawner sp("a.out", 4);

    // set a negative root
    ASSERT_DEATH( sp.set_root(-1) , "");

    // set a root which is greater than the communicators size
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    ASSERT_DEATH( sp.set_root(size) , "");
}

TEST(StartupTest, SetCommunicator) {
    // create spawn object an set a legal communicator
    mpicxx::spawner sp("a.out", 4);
    sp.set_communicator(MPI_COMM_SELF);

    // check if it has been set correctly
    EXPECT_EQ(sp.communicator(), MPI_COMM_SELF);
}

TEST(StartupDeathTest, SetIllegalCommunicator) {
    // create spawn object
    mpicxx::spawner sp("a.out", 4);

    // set null communicator
    ASSERT_DEATH( sp.set_communicator(MPI_COMM_NULL) , "");

    // set communicator with an illegal ol root value
    sp.set_root(1);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm comm;
    MPI_Comm_split(MPI_COMM_WORLD, world_rank, world_rank, &comm);
    ASSERT_DEATH( sp.set_communicator(comm) , "");
}

TEST(StartupTest, AddingArgv) {
    // create spawn object
    mpicxx::spawner sp("a.out", 4);

    // add pair of strings without leading '-'
    sp.add_argv("key1"s, "value1"s);
    // add pair of strings with leading '-'
    sp.add_argv("-key2"s, "value2"s);
    // add string and integer
    sp.add_argv("key3", 42);
    // add string and double
    sp.add_argv("key4", 3.1415);
    // add string and enum class
    sp.add_argv("key5", mpicxx::thread_support::multiple);
    // add duplicated key
    sp.add_argv("-key1", "value6");

    // no 6 argcs should be present
    ASSERT_EQ(sp.argv().size(), 6);

    // create vector containing correct [key, value]-pairs
    std::vector<std::pair<std::string, std::string>> correct_argvs =
            { { "-key1"s, "value1"s },
              { "-key2"s, "value2"s },
              { "-key3"s, "42"s },
              { "-key4"s, std::to_string(3.1415) },
              { "-key5"s, "MPI_THREAD_MULTIPLE"s },
              { "-key1"s, "value6"} };

    // check whether all [key, value]-pairs were added successfully
    for (std::size_t i = 0; i < correct_argvs.size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(sp.argv()[i].first, correct_argvs[i].first);
        EXPECT_EQ(sp.argv()[i].second, correct_argvs[i].second);
        EXPECT_EQ(sp.argv(i).first, correct_argvs[i].first);
        EXPECT_EQ(sp.argv(i).second, correct_argvs[i].second);
    }
}

TEST(StartupTest, OutOfBounceArgv) {
    // create spawner object
    mpicxx::spawner sp("a.out", 4);

    // try to access illegal element
    try {
        [[maybe_unused]] const auto argv = sp.argv(1);
        FAIL() << "expected std::out_of_range exception";
    } catch(const std::out_of_range& e) {
        EXPECT_STREQ(e.what(), "Out-of-bounce access!: 1 < 0");
    } catch(...) {
        FAIL() << "expected std::out_of_range exception";
    }
}

TEST(StartupTest, ChainingCalls) {
    // create spawner object
    mpicxx::spawner sp("a.out", 4);

    // chain function calls
    sp.set_communicator(MPI_COMM_SELF).set_root(0).set_spawn_info(mpicxx::info::env);
    sp.add_argv("key1", "value1").add_argv("key2", "value2");

    // check set values
    EXPECT_EQ(sp.communicator(), MPI_COMM_SELF);
    EXPECT_EQ(sp.root(), 0);
    EXPECT_EQ(sp.spawn_info(), mpicxx::info::env);
    EXPECT_EQ(sp.argv(0).first, "-key1"s);
    EXPECT_EQ(sp.argv(0).second, "value1"s);
    EXPECT_EQ(sp.argv(1).first, "-key2"s);
    EXPECT_EQ(sp.argv(1).second, "value2"s);
}

TEST(StartupDeathTest, GetIntercommunicator) {
    // create spawner object
    mpicxx::spawner sp("a.out", 4);

    [[maybe_unused]] MPI_Comm comm;
    ASSERT_DEATH( comm = sp.intercommunicator() , "");
}

TEST(StartupDeathTest, GetErrcodes) {
    // create spawner object
    mpicxx::spawner sp("a.out", 4);

    [[maybe_unused]] std::vector<int> errcodes;
    ASSERT_DEATH( errcodes = sp.errcodes() , "");
}

TEST(StartupDeathTest, PrintErrcodesTo) {
    // create spawner object
    mpicxx::spawner sp("a.out", 4);

    sp.print_errors_to();
}