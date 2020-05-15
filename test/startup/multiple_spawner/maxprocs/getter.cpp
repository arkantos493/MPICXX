/**
 * @file test/startup/multiple_spawner/maxprocs/getter.cpp
 * @author Marcel Breyer
 * @date 2020-05-16
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::command() const and
 * qref mpicxx::multiple_spawner::command_at(const std::sze_t) const member function provided by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name             | test case description        |
 * |:---------------------------|:-----------------------------|
 * | GetMaxprocs                | get all number of processes  |
 * | GetIthMaxprocs             | get i-th number of processes |
 * | GetIthMaxprocsInvalidIndex | illegal index                |
 */

#include <vector>

#include <gtest/gtest.h>

#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, GetMaxprocs) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // check getter for correctness
    std::vector<int> vec = ms.maxprocs();
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 1);
}

TEST(MultipleSpawnerTest, GetIthMaxprocss) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // check getter for correctness
    EXPECT_EQ(ms.maxprocs_at(0), 1);
    EXPECT_EQ(ms.maxprocs_at(1), 1);
}

TEST(MultipleSpawnerTest, GetIthMaxprocsInvalidIndex) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // try getting i-th number of processes
    try {
        [[maybe_unused]] const int i = ms.maxprocs_at(-1);
        FAIL() << "expected std::out_of_range exception";
    } catch(const std::out_of_range& e) {
        std::string expected_msg = fmt::format(
                "multiple_spawner::maxprocs_at(const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                static_cast<std::size_t>(-1), 2);
        EXPECT_STREQ(e.what(), expected_msg.c_str());
    } catch(...) {
        FAIL() << "expected std::out_of_range exception";
    }
    try {
        [[maybe_unused]] const int i = ms.maxprocs_at(2);
        FAIL() << "expected std::out_of_range exception";
    } catch(const std::out_of_range& e) {
        std::string expected_msg = fmt::format(
                "multiple_spawner::maxprocs_at(const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                2, 2);
        EXPECT_STREQ(e.what(), expected_msg.c_str());
    } catch(...) {
        FAIL() << "expected std::out_of_range exception";
    }
}