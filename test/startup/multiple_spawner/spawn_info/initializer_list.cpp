/**
 * @file test/startup/multiple_spawner/spawn_info/initializer_list.cpp
 * @author Marcel Breyer
 * @date 2020-05-17
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_spawn_info(std::initializer_list<info>) member function provided by the
 * @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                            | test case description  |
 * |:------------------------------------------|:-----------------------|
 * | SetSpawnInfoViaInitializerList            | set new spawn info from a [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list)      |
 * | SetSpawnInfoViaInitializerListInvalidSize | [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) with illegal size (death test) |
 */

#include <initializer_list>

#include <gtest/gtest.h>

#include <mpicxx/info/info.hpp>
#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, SetSpawnInfoViaInitializerList) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // set new spawn info
    ms.set_spawn_info({ mpicxx::info::env, mpicxx::info::env });

    // check if spawn info were set correctly
    ASSERT_EQ(ms.spawn_info().size(), 2);
    EXPECT_EQ(ms.spawn_info_at(0), mpicxx::info::env);
    EXPECT_EQ(ms.spawn_info_at(1), mpicxx::info::env);
}

TEST(MultipleSpawnerDeathTest, SetSpawnInfoViaInitializerListInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // set new spawn info with different size
    ASSERT_DEATH( ms.set_spawn_info({ mpicxx::info::env }) , "");
    ASSERT_DEATH( ms.set_spawn_info({ mpicxx::info::env, mpicxx::info::env, mpicxx::info::env }) , "");
}