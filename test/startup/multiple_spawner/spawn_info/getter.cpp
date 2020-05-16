/**
 * @file test/startup/multiple_spawner/spawn_info/getter.cpp
 * @author Marcel Breyer
 * @date 2020-05-17
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::spawn_info() const and
 * qref mpicxx::multiple_spawner::spawn_info_at(const std::size_t) const member function provided by the
 * @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name              | test case description |
 * |:----------------------------|:----------------------|
 * | GetSpawnInfo                | get all spawn info    |
 * | GetIthSpawnInfo             | get i-th spawn info   |
 * | GetIthSpawnInfoInvalidIndex | illegal index         |
 */

#include <vector>

#include <gtest/gtest.h>

#include <mpicxx/info/info.hpp>
#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, GetSpawnInfo) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // check getter for correctness
    const std::vector<mpicxx::info>& vec = ms.spawn_info();
    EXPECT_EQ(vec[0], mpicxx::info::null);
    EXPECT_EQ(vec[1], mpicxx::info::null);
}

TEST(MultipleSpawnerTest, GetIthSpawnInfo) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // check getter for correctness
    EXPECT_EQ(ms.spawn_info_at(0), mpicxx::info::null);
    EXPECT_EQ(ms.spawn_info_at(1), mpicxx::info::null);
}

TEST(MultipleSpawnerTest, GetIthSpawnInfoInvalidIndex) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // try getting i-th spawn info
    try {
        [[maybe_unused]] const mpicxx::info& spawn_info = ms.spawn_info_at(-1);
        FAIL() << "expected std::out_of_range exception";
    } catch(const std::out_of_range& e) {
        std::string expected_msg = fmt::format(
                "multiple_spawner::spawn_info_at(const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                static_cast<std::size_t>(-1), 2);
        EXPECT_STREQ(e.what(), expected_msg.c_str());
    } catch(...) {
        FAIL() << "expected std::out_of_range exception";
    }
    try {
        [[maybe_unused]] const mpicxx::info& spawn_info = ms.spawn_info_at(2);
        FAIL() << "expected std::out_of_range exception";
    } catch(const std::out_of_range& e) {
        std::string expected_msg = fmt::format(
                "multiple_spawner::spawn_info_at(const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                2, 2);
        EXPECT_STREQ(e.what(), expected_msg.c_str());
    } catch(...) {
        FAIL() << "expected std::out_of_range exception";
    }
}