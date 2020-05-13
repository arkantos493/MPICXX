/**
 * @file test/startup/multiple_spawner/constructor/spawner_constructor.cpp
 * @author Marcel Breyer
 * @date 2020-05-14
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::multiple_spawner(Spawner&&...) member function provided by the
 * @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                           | test case description                                                                         |
 * |:-----------------------------------------|:----------------------------------------------------------------------------------------------|
 * | ConstructFromSpawner                     | construct a multiple_spawner object from an parameter pack (variadic template)                |
 * | ConstructFromSpawnerInvalidTotalMaxprocs | try to construct a multiple_spawner object with an illegal total mayprocs number (death test) |
 */

#include <utility>

#include <gtest/gtest.h>

#include <mpicxx/startup/single_spawner.hpp>
#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, ConstructFromSpawner) {
    // create new multiple_spawner using other spawners
    mpicxx::single_spawner ss1("foo", 1);
    mpicxx::multiple_spawner ms1({ {"bar", 1} });

    mpicxx::multiple_spawner ms(ss1, ms1);
}

TEST(MultipleSpawnerDeathTest, ConstructFromSpawnerInvalidTotalMaxprocs) {
    // create spawners with different roots
    mpicxx::single_spawner ss1("foo", 1);
    mpicxx::single_spawner ss2("bar", 1);
    mpicxx::single_spawner ss3("baz", 1);

    ASSERT_DEATH( mpicxx::multiple_spawner ms(ss1, ss2, ss3) , "");
}