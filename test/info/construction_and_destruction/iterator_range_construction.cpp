/**
 * @file iterator_range_construction.cpp
 * @author Marcel Breyer
 * @date 2019-12-15
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the iterator range constructor of the mpicxx::info class.
 */

#include <vector>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(IteratorRangeConstructionTest, IteratorRangeConstruction) {
    // create vector with alle [key, value]-pairs
    std::vector<std::pair<std::string, std::string>> key_value_pairs;
    key_value_pairs.emplace_back("key1", "value1");
    key_value_pairs.emplace_back("key2", "value2");
    key_value_pairs.emplace_back("key1", "value1_override");
    key_value_pairs.emplace_back("key3", "value3");

    // construct a info object from an iterator range
    mpicxx::info info(key_value_pairs.cbegin(), key_value_pairs.cend());

    // info object should now contain 3 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 3);

    // check if all [key, value]-pairs were added
    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info.get(), "key1", 15, value, &flag);
    // check if the key exists
    EXPECT_TRUE(static_cast<bool>(flag));
    // be sure that, if the same key is provided multiple times, the last value is used
    EXPECT_STREQ(value, "value1_override");

    MPI_Info_get(info.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");

    MPI_Info_get(info.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");

    // an info object constructed from an iterator range is always freeable
    EXPECT_TRUE(info.freeable());
}