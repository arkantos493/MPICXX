/**
 * @file info/lookup/contains.cpp
 * @author Marcel Breyer
 * @date 2020-01-30
 *
 * @brief Test cases for the @ref mpicxx::info::contains(const std::string_view) const const member function provided by the
 * @ref mpicxx::info class.
 * @details Testsuite: *LookupTest*
 * | test case name      | test case description                            |
 * |:--------------------|:-------------------------------------------------|
 * | ContainsExisting    | check for existing keys                          |
 * | ContainsNonExisting | check for non-existing key                       |
 * | MovedFromContains   | info object in the moved-from state (death test) |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(LookupTest, ContainsExisting) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // check for the existence of the keys
    EXPECT_TRUE(info.contains("key1"));
    EXPECT_TRUE(info.contains("key2"));
}

TEST(LookupTest, ContainsNonExisting) {
    // create info object and add [key, value]-pair
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // check for the existence of the key
    EXPECT_FALSE(info.contains("key2"));
}

TEST(LookupDeathTest, MovedFromContains) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // calling contains() on an info object in the moved-from state is illegal
    [[maybe_unused]] bool contains;
    ASSERT_DEATH(contains = info.contains("key"), "");
}