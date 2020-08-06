/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::multiple_spawner(T&&...) member function provided by the
 *        @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                           | test case description                                                                         |
 * |:-----------------------------------------|:----------------------------------------------------------------------------------------------|
 * | ConstructFromSpawner                     | construct a multiple_spawner object from an parameter pack (variadic template)                |
 * | ConstructFromSpawnerInvalidTotalMaxprocs | try to construct a multiple_spawner object with an illegal total maxprocs number (death test) |
 */

#include <mpicxx/startup/single_spawner.hpp>
#include <mpicxx/startup/multiple_spawner.hpp>

#include <gtest/gtest.h>

#include <initializer_list>
#include <utility>

TEST(MultipleSpawnerTest, ConstructFromSpawner) {
    // create new multiple_spawner using other spawners
    mpicxx::single_spawner ss1("foo", 1);
    mpicxx::multiple_spawner ms1({ { "bar", 1 } });

    mpicxx::multiple_spawner ms(ss1, ms1);
}

TEST(MultipleSpawnerDeathTest, ConstructFromSpawnerInvalidTotalMaxprocs) {
    // create spawners with different roots
    mpicxx::single_spawner ss1("foo", 1);
    mpicxx::single_spawner ss2("bar", 1);
    mpicxx::single_spawner ss3("baz", 1);

    ASSERT_DEATH( mpicxx::multiple_spawner ms(ss1, ss2, ss3) , "");
}

// TODO 2020-06-02 21:38 breyerml: test with different communicators