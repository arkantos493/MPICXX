/**
 * @file test/startup/multiple_spawner/command/getter.cpp
 * @author Marcel Breyer
 * @date 2020-05-16
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::command() const and
 * qref mpicxx::multiple_spawner::command_at(const std::sze_t) const member function provided by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                   | test case description    |
 * |:---------------------------------|:-------------------------|
 * | GetExecutableNames               | get all executable names |
 * | GetIthExecutableName             | get i-th executable name |
 * | GetIthExecutableNameInvalidIndex | illegal index            |
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <mpicxx/startup/multiple_spawner.hpp>

using namespace std::string_literals;


TEST(MultipleSpawnerTest, GetExecutableNames) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // check getter for correctness
    std::vector<std::string> vec = ms.command();
    EXPECT_EQ(vec[0], "foo"s);
    EXPECT_EQ(vec[1], "bar"s);
}

TEST(MultipleSpawnerTest, GetIthExecutableName) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // check getter for correctness
    EXPECT_EQ(ms.command_at(0), "foo"s);
    EXPECT_EQ(ms.command_at(1), "bar"s);
}

TEST(MultipleSpawnerTest, GetIthExecutableNameInvalidIndex) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // try getting i-th executable name
    try {
        [[maybe_unused]] const std::string& str = ms.command_at(-1);
        FAIL() << "expected std::out_of_range exception";
    } catch(const std::out_of_range& e) {
        std::string expected_msg = fmt::format(
                "multiple_spawner::command_at(const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                static_cast<std::size_t>(-1), 2);
        EXPECT_STREQ(e.what(), expected_msg.c_str());
    } catch(...) {
        FAIL() << "expected std::out_of_range exception";
    }
    try {
        [[maybe_unused]] const std::string& str = ms.command_at(2);
        FAIL() << "expected std::out_of_range exception";
    } catch(const std::out_of_range& e) {
        std::string expected_msg = fmt::format(
                "multiple_spawner::command_at(const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                2, 2);
        EXPECT_STREQ(e.what(), expected_msg.c_str());
    } catch(...) {
        FAIL() << "expected std::out_of_range exception";
    }
}