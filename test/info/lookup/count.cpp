/**
 * @file test/info/lookup/count.cpp
 * @author Marcel Breyer
 * @date 2020-04-11
 *
 * @brief Test cases for the @ref mpicxx::info::count(const std::string_view) const member function provided by the @ref mpicxx::info
 * class.
 * @details Testsuite: *LookupTest*
 * | test case name      | test case description                               |
 * |:--------------------|:----------------------------------------------------|
 * | CountExisting       | count existing keys                                 |
 * | CountNonExisting    | count non-existing key                              |
 * | NullCount           | info object referring to MPI_INFO_NULL (death test) |
 * | CountWithIllegalKey | try to count an illegal key (death test)            |
 */

#include <string>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(LookupTest, CountExisting) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // try counting the keys
    EXPECT_EQ(info.count("key1"), 1);
    EXPECT_EQ(info.count("key2"), 1);
}

TEST(LookupTest, CountNonExisting) {
    // create info object and add [key, value]-pair
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // try counting non-existing key
    EXPECT_EQ(info.count("key2"), 0);
}

TEST(LookupDeathTest, NullCount) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling count() on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] int count;
    ASSERT_DEATH( count = info.count("key") , "");
}

TEST(LookupDeathTest, CountWithIllegalKey) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');

    // try to count an illegal key
    [[maybe_unused]] int count;
    ASSERT_DEATH( count = info.count(key) , "");
    ASSERT_DEATH( count = info.count("") , "");
}