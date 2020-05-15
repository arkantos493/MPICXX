/**
 * @file test/startup/multiple_spawner/maxprocs/ith.cpp
 * @author Marcel Breyer
 * @date 2020-05-16
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_maxprocs_at(const std::size_t, const int) member function provided by the
 * @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                  | test case description                                                       |
 * |:--------------------------------|:----------------------------------------------------------------------------|
 * | SetIthMaxprocsValue             | set the ith maxprocs value                                                  |
 * | SetIthMaxprocsInvalidIndex      | illegal index                                                               |
 * | SetIthMaxprocsInvalidValue      | try to set new number of processes with an invalid (death test)             |
 * | SetIthMaxprocsInvalidTotalValue | try to set new number of processes with an invalid total value (death test) |
 */

#include <cstddef>
#include <limits>
#include <stdexcept>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, SetIthMaxprocsValue) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // set i-th number of processes
    ms.set_maxprocs_at(1, 1);
    ms.set_maxprocs_at(0, 1);

    // check if names were set correctly
    ASSERT_EQ(ms.maxprocs().size(), 2);
    EXPECT_EQ(ms.maxprocs_at(0), 1);
    EXPECT_EQ(ms.maxprocs_at(1), 1);
}

TEST(MultipleSpawnerTest, SetIthMaxprocsInvalidIndex) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // try setting i-th executable name
    try {
        ms.set_maxprocs_at(-1, 1);
        FAIL() << "expected std::out_of_range exception";
    } catch(const std::out_of_range& e) {
        std::string expected_msg = fmt::format(
                "multiple_spawner::set_maxprocs_at(const std::size_t, const int) range check: i (which is {}) >= this->size() (which is {})",
                static_cast<std::size_t>(-1), 2);
        EXPECT_STREQ(e.what(), expected_msg.c_str());
    } catch(...) {
        FAIL() << "expected std::out_of_range exception";
    }
    try {
        ms.set_maxprocs_at(2, 1);
        FAIL() << "expected std::out_of_range exception";
    } catch(const std::out_of_range& e) {
        std::string expected_msg = fmt::format(
                "multiple_spawner::set_maxprocs_at(const std::size_t, const int) range check: i (which is {}) >= this->size() (which is {})",
                2, 2);
        EXPECT_STREQ(e.what(), expected_msg.c_str());
    } catch(...) {
        FAIL() << "expected std::out_of_range exception";
    }
}

TEST(MultipleSpawnerDeathTest, SetIthMaxprocsInvalidValue) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // try setting i-th number of processes
    ASSERT_DEATH( ms.set_maxprocs_at(0, 0) , "");
    ASSERT_DEATH( ms.set_maxprocs_at(0, std::numeric_limits<int>::max()) , "");
}

TEST(MultipleSpawnerDeathTest, SetIthMaxprocsInvalidTotalValue) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // try setting i-th number of processes
    ASSERT_DEATH( ms.set_maxprocs_at(0, 2) , "");
}