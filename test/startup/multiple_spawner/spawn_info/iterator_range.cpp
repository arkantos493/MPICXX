/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_spawn_info(InputIt, InputIt) member function provided
 *        by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                          | test case description                         |
 * |:----------------------------------------|:----------------------------------------------|
 * | SetSpawnInfoViaIteratorRange            | set new spawn info from an iterator range     |
 * | SetSpawnInfoViaInvalidIteratorRange     | illegal iterator range (death test)           |
 * | SetSpawnInfoViaIteratorRangeInvalidSize | iterator range with illegal size (death test) |
 */

#include <mpicxx/info/info.hpp>
#include <mpicxx/startup/multiple_spawner.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <initializer_list>
#include <utility>
#include <vector>

TEST(MultipleSpawnerTest, SetSpawnInfoViaIteratorRange) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new spawn info
    std::vector<mpicxx::info> vec = { mpicxx::info::env, mpicxx::info::env };
    ms.set_spawn_info(vec.begin(), vec.end());

    // check if spawn info were set correctly
    ASSERT_EQ(ms.spawn_info().size(), 2);
    for (std::size_t i = 0; i < 2; ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(ms.spawn_info_at(i), vec[i]);
    }
}

TEST(MultipleSpawnerDeathTest, SetSpawnInfoViaInvalidIteratorRange) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new spawn info with illegal iterator range
    std::vector<mpicxx::info> vec = { mpicxx::info::env, mpicxx::info::env };
    ASSERT_DEATH( ms.set_spawn_info(vec.end(), vec.begin()) , "");
}

TEST(MultipleSpawnerDeathTest, SetSpawnInfoViaIteratorRangeInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new spawn info with different size
    std::vector<mpicxx::info> vec = { mpicxx::info::env, mpicxx::info::env, mpicxx::info::env };
    ASSERT_DEATH( ms.set_spawn_info(vec.begin(), vec.begin() + 1) , "");
    ASSERT_DEATH( ms.set_spawn_info(vec.begin(), vec.end()) , "");
}