/**
 * @file test/startup/multiple_spawner/spawn_info/ith.cpp
 * @author Marcel Breyer
 * @date 2020-06-02
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_spawn_info_at(const std::size_t, info) member function provided
 *        by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name              | test case description   |
 * |:----------------------------|:------------------------|
 * | SetIthSpawnInfo             | set the i-th spawn info |
 * | SetIthSpawnInfoInvalidIndex | illegal index           |
 */

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <test_utility.hpp>

#include <mpicxx/info/info.hpp>
#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, SetIthSpawnInfo) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set i-th spawn info
    ms.set_spawn_info_at(0, mpicxx::info::env);
    ms.set_spawn_info_at(1, mpicxx::info::env);

    // check if spawn info were set correctly
    ASSERT_EQ(ms.spawn_info().size(), 2);
    EXPECT_EQ(ms.spawn_info_at(0), mpicxx::info::env);
    EXPECT_EQ(ms.spawn_info_at(1), mpicxx::info::env);
}

TEST(MultipleSpawnerTest, SetIthSpawnInfoInvalidIndex) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try setting i-th spawn info
    EXPECT_THROW_WHAT(
            ms.set_spawn_info_at(2, mpicxx::info::env),
            std::out_of_range,
            "multiple_spawner::set_spawn_info_at(const std::size_t, info) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::set_spawn_info_at(const std::size_t, info) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            ms.set_spawn_info_at(-1, mpicxx::info::env),
            std::out_of_range,
            expected_msg);
}