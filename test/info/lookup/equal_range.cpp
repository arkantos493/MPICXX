/**
 * @file find.cpp
 * @author Marcel Breyer
 * @date 2020-01-20
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the `equal_range` member function of the mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(LookupTest, EqualRangeExisting) {
    // create empty info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // try finding the keys
    std::pair<mpicxx::info::iterator, mpicxx::info::iterator> it_pair_1 = info.equal_range("key1");
    ASSERT_NE(it_pair_1.first, info.end());
    ASSERT_NE(it_pair_1.second, info.end());
    ASSERT_EQ(it_pair_1.first + 1, it_pair_1.second);
    EXPECT_STREQ(it_pair_1.first->first.c_str(), "key1");
    EXPECT_STREQ(static_cast<std::string>(it_pair_1.first->second).c_str(), "value1");

    auto it_pair_2 = info.equal_range("key2");
    ASSERT_NE(it_pair_2.first, info.end());
    ASSERT_EQ(it_pair_2.second, info.end());
    ASSERT_EQ(it_pair_2.first + 1, it_pair_2.second);
    EXPECT_STREQ(it_pair_2.first->first.c_str(), "key2");
    EXPECT_STREQ(static_cast<std::string>(it_pair_2.first->second).c_str(), "value2");
}

TEST(LookupTest, EqualRangeNonExisting) {
    // create empty info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // try finding non-existing key
    auto it_pair = info.equal_range("key2");
    EXPECT_EQ(it_pair.first, info.end());
    EXPECT_EQ(it_pair.second, info.end());
    EXPECT_EQ(it_pair.first, it_pair.second);
}

TEST(LookupTest, MovedFromEqualRange) {
    // create info object and set it to the "moved-from" state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // call to empty from a "moved-from" info object is invalid
//    [[maybe_unused]] const auto range = info.equal_range("key");       // -> should assert
}


TEST(LookupTest, ConstEqualRangeExisting) {
    // create info object with [key, value]-pairs
    const mpicxx::info info = { { "key1", "value1" } , { "key2", "value2" } };

    // try finding the keys
    std::pair<mpicxx::info::const_iterator, mpicxx::info::const_iterator> it_pair_1 = info.equal_range("key1");
    ASSERT_NE(it_pair_1.first, info.end());
    ASSERT_NE(it_pair_1.second, info.end());
    ASSERT_EQ(it_pair_1.first + 1, it_pair_1.second);
    EXPECT_STREQ(it_pair_1.first->first.c_str(), "key1");
    EXPECT_STREQ(it_pair_1.first->second.c_str(), "value1");

    auto it_pair_2 = info.equal_range("key2");
    ASSERT_NE(it_pair_2.first, info.end());
    ASSERT_EQ(it_pair_2.second, info.end());
    ASSERT_EQ(it_pair_2.first + 1, it_pair_2.second);
    EXPECT_STREQ(it_pair_2.first->first.c_str(), "key2");
    EXPECT_STREQ(it_pair_2.first->second.c_str(), "value2");
}

TEST(LookupTest, ConstEqualRangeNonExisting) {
    // create empty info object
    const mpicxx::info info;

    // try finding non-existing key
    auto it_pair = info.equal_range("key");
    EXPECT_EQ(it_pair.first, info.end());
    EXPECT_EQ(it_pair.second, info.end());
    EXPECT_EQ(it_pair.first, it_pair.second);
}