/**
 * @file test/startup/multiple_spawner/maxprocs/ith.cpp
 * @author Marcel Breyer
 * @date 2020-06-02
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_maxprocs_at(const std::size_t, const int) member function provided
 *        by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                  | test case description                                                       |
 * |:--------------------------------|:----------------------------------------------------------------------------|
 * | SetIthMaxprocsValue             | set the i-th maxprocs value                                                 |
 * | SetIthMaxprocsInvalidIndex      | illegal index                                                               |
 * | SetIthMaxprocsInvalidValue      | try to set new number of processes with an invalid (death test)             |
 * | SetIthMaxprocsInvalidTotalValue | try to set new number of processes with an invalid total value (death test) |
 */

#include <cstddef>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <utility>

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <test_utility.hpp>

#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, SetIthMaxprocsValue) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set i-th number of processes
    ms.set_maxprocs_at(1, 1);
    ms.set_maxprocs_at(0, 1);

    // check if the values were set correctly
    ASSERT_EQ(ms.maxprocs().size(), 2);
    EXPECT_EQ(ms.maxprocs_at(0), 1);
    EXPECT_EQ(ms.maxprocs_at(1), 1);
}

TEST(MultipleSpawnerTest, SetIthMaxprocsInvalidIndex) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try setting i-th executable name
    EXPECT_THROW_WHAT(
            ms.set_maxprocs_at(2, 1),
            std::out_of_range,
            "multiple_spawner::set_maxprocs_at(const std::size_t, const int) range check: i (which is 2) >= this->size() (which is 2)");

    std::string expected_msg =
            fmt::format("multiple_spawner::set_maxprocs_at(const std::size_t, const int) range check: "
                        "i (which is {}) >= this->size() (which is 2)", static_cast<std::size_t>(-1));
    EXPECT_THROW_WHAT(
            ms.set_maxprocs_at(-1, 1),
            std::out_of_range,
            expected_msg);
}

TEST(MultipleSpawnerDeathTest, SetIthMaxprocsInvalidValue) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try setting i-th number of processes
    ASSERT_DEATH( ms.set_maxprocs_at(0, 0) , "");
    ASSERT_DEATH( ms.set_maxprocs_at(0, std::numeric_limits<int>::max()) , "");
}

TEST(MultipleSpawnerDeathTest, SetIthMaxprocsInvalidTotalValue) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // try setting i-th number of processes
    ASSERT_DEATH( ms.set_maxprocs_at(0, 2) , "");
}