/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::spawn_info() const and
 *        @ref mpicxx::multiple_spawner::spawn_info_at(const std::size_t) const member function provided by the
 *        @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name              | test case description |
 * |:----------------------------|:----------------------|
 * | GetSpawnInfo                | get all spawn info    |
 * | GetIthSpawnInfo             | get i-th spawn info   |
 * | GetIthSpawnInfoInvalidIndex | illegal index         |
 */

#include <mpicxx/info/info.hpp>
#include <mpicxx/startup/multiple_spawner.hpp>
#include <test_utility.hpp>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <vector>

TEST(MultipleSpawnerTest, GetSpawnInfo) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // check getter for correctness
    const std::vector<mpicxx::info>& vec = ms.spawn_info();
    EXPECT_EQ(vec[0], mpicxx::info::null);
    EXPECT_EQ(vec[1], mpicxx::info::null);
}

TEST(MultipleSpawnerTest, GetIthSpawnInfo) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // check getter for correctness
    EXPECT_EQ(ms.spawn_info_at(0), mpicxx::info::null);
    EXPECT_EQ(ms.spawn_info_at(1), mpicxx::info::null);
}

TEST(MultipleSpawnerTest, GetIthSpawnInfoInvalidIndex) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try getting i-th spawn info
    [[maybe_unused]] mpicxx::info spawn_info;
    EXPECT_THROW_WHAT(
            spawn_info = ms.spawn_info_at(2),
            std::out_of_range,
            "multiple_spawner::spawn_info_at(const std::size_t) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::spawn_info_at(const std::size_t) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            spawn_info = ms.spawn_info_at(-1),
            std::out_of_range,
            expected_msg);
}