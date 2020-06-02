/**
 * @file test/startup/multiple_spawner/communicator.cpp
 * @author Marcel Breyer
 * @date 2020-06-02
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_communicator(MPI_Comm) and
 *        @ref mpicxx::multiple_spawner::communicator() const member function provided by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name         | test case description                            |
 * |:-----------------------|:-------------------------------------------------|
 * | SetCommunicator        | set a new intracommunicator                      |
 * | SetInvalidCommunicator | set a new illegal intracommunicator (death test) |
 * | GetCommunicator        | get the current intracommunicator                |
 */

#include <initializer_list>
#include <utility>

#include <gtest/gtest.h>

#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, SetCommunicator) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // TODO 2020-04-13 21:30 breyerml: use mpicxx communicator equivalent
}

TEST(MultipleSpawnerDeathTest, SetInvalidCommunicator) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set a new illegal communicator
    ASSERT_DEATH( ms.set_communicator(MPI_COMM_NULL) , "");
    // TODO 2020-04-13 21:30 breyerml: use mpicxx communicator equivalent (with comm size = 1 and root = 1)
}

TEST(MultipleSpawnerTest, GetCommunicator) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // check getter
    EXPECT_EQ(ms.communicator(), MPI_COMM_WORLD);
}