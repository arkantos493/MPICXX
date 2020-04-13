/**
 * @file test/startup/single_spawner/communicator.cpp
 * @author Marcel Breyer
 * @date 2020-04-13
 *
 * @brief Test cases for the @ref mpicxx::single_spawner class communicator member functions.
 * @details Testsuite: *SingleSpawnerTest*
 * | test case name         | test case description                                          |
 * |:-----------------------|:---------------------------------------------------------------|
 * | SetCommunicator        | set a new intracommunicator                                    |
 * | SetInvalidCommunicator | set a new illegal intracommunicator (death test)               |
 * | ChainSetCommunicator   | chain calls to @ref mpicxx::single_spawner::set_communicator() |
 * | GetCommunicator        | get the current intracommunicator                              |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/startup/single_spawner.hpp>

using namespace std::string_literals;


TEST(SingleSpawnerTest, SetCommunicator) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // TODO 2020-04-13 21:30 breyerml: use mpicxx communicator equivalent
}

TEST(SingleSpawnerDeathTest, SetInvalidCommunicator) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // set a new illegal root
    ASSERT_DEATH( ss.set_communicator(MPI_COMM_NULL) , "");
    // TODO 2020-04-13 21:30 breyerml: use mpicxx communicator equivalent (with comm size = 1 and root = 1)
}

TEST(SingleSpawnerTest, ChainSetCommunicator) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // chain multiple calls to set_communicator
    // TODO 2020-04-13 21:30 breyerml: use mpicxx communicator equivalent
}

TEST(SingleSpawnerTest, GetCommunicator) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // check getter
    EXPECT_EQ(ss.communicator(), MPI_COMM_WORLD);
}