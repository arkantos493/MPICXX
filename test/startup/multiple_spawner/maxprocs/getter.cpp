/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::maxprocs() const and
 * @ref mpicxx::multiple_spawner::maxprocs_at(const std::size_t) const member function provided by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name             | test case description        |
 * |:---------------------------|:-----------------------------|
 * | GetMaxprocs                | get all number of processes  |
 * | GetIthMaxprocs             | get i-th number of processes |
 * | GetIthMaxprocsInvalidIndex | illegal index                |
 */

#include <mpicxx/startup/multiple_spawner.hpp>
#include <test_utility.hpp>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

TEST(MultipleSpawnerTest, GetMaxprocs) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // check getter for correctness
    std::vector<int> vec = ms.maxprocs();
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 1);
}

TEST(MultipleSpawnerTest, GetIthMaxprocss) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // check getter for correctness
    EXPECT_EQ(ms.maxprocs_at(0), 1);
    EXPECT_EQ(ms.maxprocs_at(1), 1);
}

TEST(MultipleSpawnerTest, GetIthMaxprocsInvalidIndex) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // try getting i-th number of processes
    [[maybe_unused]] int i;
    EXPECT_THROW_WHAT(
            i = ms.maxprocs_at(2),
            std::out_of_range,
            "multiple_spawner::maxprocs_at(const std::size_t) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::maxprocs_at(const std::size_t) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            i = ms.maxprocs_at(-1),
            std::out_of_range,
            expected_msg);
}