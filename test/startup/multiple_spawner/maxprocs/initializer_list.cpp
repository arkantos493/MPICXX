/**
 * @file test/startup/multiple_spawner/maxprocs/initializer_list.cpp
 * @author Marcel Breyer
 * @date 2020-05-15
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_maxprocs(std::initializer_list<int>) member function provided by the
 * @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                                 | test case description                                                       |
 * |:-----------------------------------------------|:----------------------------------------------------------------------------|
 * | SetMaxprocsViaInitializerList                  | set new number of maxprocs from a [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) |
 * | SetMaxprocsViaInitializerListInvalidSize       | [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) with illegal size (death test)    |
 * | SetMaxprocsViaInitializerListInvalidValue      | try to set new number of processes with an invalid value (death test)       |
 * | SetMaxprocsViaInitializerListInvalidTotalValue | try to set new number of processes with an invalid total value (death test) |
 */

#include <initializer_list>

#include <gtest/gtest.h>

#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, SetMaxprocsViaInitializerList) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // set number of processes
    ms.set_maxprocs({ 1, 1 });

    // check if the values were set correctly
    ASSERT_EQ(ms.maxprocs().size(), 2);
    EXPECT_EQ(ms.maxprocs_at(0), 1);
    EXPECT_EQ(ms.maxprocs_at(1), 1);
}

TEST(MultipleSpawnerDeathTest, SetMaxprocsViaInitializerListInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // set new number of processes with different size
    ASSERT_DEATH( ms.set_maxprocs({ 1 }) , "");
    ASSERT_DEATH( ms.set_maxprocs({ 1, 1, 1 }) , "");
}

TEST(MultipleSpawnerDeathTest, SetMaxprocsViaInitializerListInvalidValue) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // set new number of processes with illegal value
    ASSERT_DEATH( ms.set_maxprocs({ 1, 3 }), "");
    ASSERT_DEATH( ms.set_maxprocs({ 0, 1 }), "");
}

TEST(MultipleSpawnerDeathTest, SetMaxprocsViaInitializerListInvalidTotalValue) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // set new executable names with illegal total value
    ASSERT_DEATH( ms.set_maxprocs({ 2, 2 }), "");
}