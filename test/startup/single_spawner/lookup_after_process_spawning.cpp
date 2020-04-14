/**
 * @file test/startup/single_spawner/lookup_after_process_spawning.cpp
 * @author Marcel Breyer
 * @date 2020-04-14
 *
 * @brief Test cases for the @ref mpicxx::single_spawner class lookup after process spawning member functions.
 * @details Testsuite: *SingleSpawnerTest*
 * | test case name                      | test case description                                                                              |
 * |:------------------------------------|:---------------------------------------------------------------------------------------------------|
 * | NumberOfProcessesSpawnedBeforeSpawn | query the number of spawned processes before a call to @ref mpicxx::single_spawner::spawn()        |
 * | MaxprocsProcessesSpawnedBeforeSpawn | query whether all processes could be spawned before a call to @ref mpicxx::single_spawner::spawn() |
 * | GetIntercommunicatorBeforeSpawn     | try to get the intercommunicator before a call to @ref mpicxx::single_spawner::spawn()             |
 * | GetErrcodesBeforeSpawn              | try to get the errcodes before a call to @ref mpicxx::single_spawner::spawn()                      |
 * | PrintErrorsToBeforeSpawn            | try to print all errors before a call to @ref mpicxx::single_spawner::spawn()                      |
 */

#include <vector>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/startup/single_spawner.hpp>

using namespace std::string_literals;


TEST(SingleSpawnerDeathTest, NumberOfProcessesSpawnedBeforeSpawn) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // calling number_of_spawned_processes() before spawn() is illegal
    [[maybe_unused]] int count;
    ASSERT_DEATH( count = ss.number_of_spawned_processes() , "");
}

TEST(SingleSpawnerDeathTest, MaxprocsProcessesSpawnedBeforeSpawn) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // calling maxprocs_processes_spawned() before spawn() is illegal
    [[maybe_unused]] bool flag;
    ASSERT_DEATH( flag = ss.maxprocs_processes_spawned() , "");
}

TEST(SingleSpawnerDeathTest, GetIntercommunicatorBeforeSpawn) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // calling intercommunicator() before spawn() is illegal
    [[maybe_unused]] MPI_Comm comm;
    ASSERT_DEATH( comm = ss.intercommunicator() , "");
}

TEST(SingleSpawnerDeathTest, GetErrcodesBeforeSpawn) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // calling errcodes() before spawn() is illegal
    [[maybe_unused]] std::vector<int> codes;
    ASSERT_DEATH( codes = ss.errcodes() , "");
}

TEST(SingleSpawnerDeathTest, PrintErrorsToBeforeSpawn) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // calling print_errors_to() before spawn() is illegal
    ASSERT_DEATH( ss.print_errors_to() , "");
}