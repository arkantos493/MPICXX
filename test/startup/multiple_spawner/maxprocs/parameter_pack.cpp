/**
 * @file test/startup/multiple_spawner/maxprocs/parameter_pack.cpp
 * @author Marcel Breyer
 * @date 2020-05-15
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_maxprocs(T...) member function provided by the
 * @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                               | test case description                                                       |
 * |:---------------------------------------------|:----------------------------------------------------------------------------|
 * | SetMaxprocsViaParameterPack                  | set new number of processes from a parameter pack                           |
 * | SetMaxprocsViaParameterPackInvalidSize       | parameter pack with illegal size (death test)                               |
 * | SetMaxprocsViaParameterPackInvalidValue      | try to set new number of processes with an invalid value (death test)       |
 * | SetMaxprocsViaParameterPackInvalidTotalValue | try to set new number of processes with an invalid total value (death test) |
 */


#include <gtest/gtest.h>

#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, SetMaxprocsViaParameterPack) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // set new number of processes
    ms.set_maxprocs(1, 1);

    // check if names were set correctly
    ASSERT_EQ(ms.maxprocs().size(), 2);
    EXPECT_EQ(ms.maxprocs(0), 1);
    EXPECT_EQ(ms.maxprocs(1), 1);
}

TEST(MultipleSpawnerDeathTest, SetMaxprocsViaParameterPackInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // set new number of processes with different size
    ASSERT_DEATH( ms.set_maxprocs(1) , "");
    int i = 1;
    ASSERT_DEATH( ms.set_maxprocs(1, 1, i) , "");
}

TEST(MultipleSpawnerDeathTest, SetMaxprocsViaParameterPackInvalidName) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // set new number of processes with illegal value
    ASSERT_DEATH( ms.set_maxprocs(1, 3), "");
    ASSERT_DEATH( ms.set_maxprocs(0, 1), "");
}


TEST(MultipleSpawnerDeathTest, SetMaxprocsViaParameterpackInvalidTotalValue) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // set new number of processes with illegal total value
    ASSERT_DEATH( ms.set_maxprocs(2, 2), "");
}