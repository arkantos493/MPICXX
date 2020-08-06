/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_spawn_info(T&&...) member function provided
 *        by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                          | test case description                         |
 * |:----------------------------------------|:----------------------------------------------|
 * | SetSpawnInfoViaParameterPack            | set new spawn info from a parameter pack      |
 * | SetSpawnInfoViaParameterPackInvalidSize | parameter pack with illegal size (death test) |
 */

#include <mpicxx/info/info.hpp>
#include <mpicxx/startup/multiple_spawner.hpp>

#include <gtest/gtest.h>

#include <initializer_list>
#include <utility>

TEST(MultipleSpawnerTest, SetSpawnInfoViaParameterPack) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new spawn info
    mpicxx::info spawn_info = { { "key", "value" } };
    ms.set_spawn_info(mpicxx::info::env, spawn_info);

    // check if spawn info were set correctly
    ASSERT_EQ(ms.spawn_info().size(), 2);
    EXPECT_EQ(ms.spawn_info_at(0), mpicxx::info::env);
    EXPECT_EQ(ms.spawn_info_at(1), spawn_info);
}

TEST(MultipleSpawnerDeathTest, SetSpawnInfoViaParameterPackInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new spawn info with different size
    ASSERT_DEATH( ms.set_spawn_info(mpicxx::info::env) , "");
    mpicxx::info spawn_info = { { "key", "value" } };
    ASSERT_DEATH( ms.set_spawn_info(mpicxx::info::env, mpicxx::info::null, spawn_info) , "");
}