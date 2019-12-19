/**
 * @file find.cpp
 * @author Marcel Breyer
 * @date 2019-12-19
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the `find` member function of the mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(LookupTest, FindExisting) {
    // create empty info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // try finding the keys
    mpicxx::info::iterator it_1 = info.find("key1");
    ASSERT_NE(it_1, info.end());
    EXPECT_STREQ(it_1->first.c_str(), "key1");
    EXPECT_STREQ(static_cast<std::string>(it_1->second).c_str(), "value1");
    auto it_2 = info.find("key2");
    ASSERT_NE(it_2, info.end());
    EXPECT_STREQ(it_2->first.c_str(), "key2");
    EXPECT_STREQ(static_cast<std::string>(it_2->second).c_str(), "value2");
}

TEST(LookupTest, FindNonExisting) {
    // create empty info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // try finding non-existing key
    mpicxx::info::iterator it = info.find("key2");
    EXPECT_EQ(it, info.end());
}

TEST(LookupTest, MovedFromFind) {
    // create info object and set it to the "moved-from" state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // call to empty from a "moved-from" info object is invalid
//    [[maybe_unused]] const auto find = info.find("key");       // -> should assert
}


TEST(LookupTest, ConstFindExisting) {
    // create info object with [key, value]-pairs
    const mpicxx::info info = { { "key1", "value1" } , { "key2", "value2" } };

    // try finding the keys
    mpicxx::info::const_iterator it_1 = info.find("key1");
    ASSERT_NE(it_1, info.end());
    EXPECT_STREQ(it_1->first.c_str(), "key1");
    EXPECT_STREQ(it_1->second.c_str(), "value1");
    auto it_2 = info.find("key2");
    ASSERT_NE(it_2, info.end());
    EXPECT_STREQ(it_2->first.c_str(), "key2");
    EXPECT_STREQ(it_2->second.c_str(), "value2");
}

TEST(LookupTest, ConstFindNonExisting) {
    // create empty info object
    const mpicxx::info info;

    // try finding non-existing key
    mpicxx::info::const_iterator it = info.find("key");
    EXPECT_EQ(it, info.end());
}