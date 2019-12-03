/**
 * @file info_modifier_test.cpp
 * @author Marcel Breyer
 * @date 2019-12-03
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the modifying methods of a mpicxx::info object.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(InfoTests, Erase) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);


    // create copy for erasing
    mpicxx::info info_copy(info);

    // erase first and last elements
    info.erase(info.begin());
    info.erase(info.end() - 1);

    // info object should now contain 2 entries
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);

    // check if the correct elements has been deleted
    char key[MPI_MAX_INFO_KEY];
    MPI_Info_get_nthkey(info.get(), 0, key);
    EXPECT_STREQ(key, "key2");
    MPI_Info_get_nthkey(info.get(), 1, key);
    EXPECT_STREQ(key, "key3");


    // restore state
    info =  info_copy;

    // erase first three elements
    info.erase(info.begin(), info.begin() + 3);

    // info object should now contain one entry
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // check if the correct elements has been deleted
    MPI_Info_get_nthkey(info.get(), 0, key);
    EXPECT_STREQ(key, "key4");

    // restore state
    info =  info_copy;

    // erase nothing (first == last)
    info.erase(info.begin(), info.begin());
    // info object should now still contain 4 entries
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);


    // restore state
    info =  info_copy;

    // erase elements by key
    info.erase("key1");
    info.erase("key3");
    info.erase("key4");

    // info object should now contain one entry
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // check if the correct elements has been deleted
    MPI_Info_get_nthkey(info.get(), 0, key);
    EXPECT_STREQ(key, "key2");


    // assertion tests
    info = info_copy;
//    info.erase(info.end());             // past-the-end iterator -> out-of-bounds access
//    info.erase(info.begin() - 1);       // out-of-bounds access
//    info.erase(info_copy.begin());      // iterator pointing to another info object

//    info.erase(info.end(), info.begin());             // `first` past-the-end iterator -> out-of-bounds access
//    info.erase(info.begin() - 1, info.begin());       // `first` out-of-bounds access
//    info.erase(info_copy.begin(), info.begin());      // `first` iterator pointing to another info object
//    info.erase(info.begin(), info.end());             // `last` past-the-end iterator -> out-of-bounds access
//    info.erase(info.begin(), info.begin() - 1);       // `last` out-of-bounds access
//    info.erase(info.begin(), info_copy.begin());      // `last` iterator pointing to another info object
//    info.erase(info.begin() + 1, info.begin());       // `first` must be less or equal than `last`

//    info.erase("vvvvveeeeerrrrryyyyy llllloooonnnngggg kkkkkeeeeeyyyyy");       // key too long

}

TEST(InfoTests, Clear) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // clear content
    info.clear();
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);

    // invoking another clear should be fine
    info.clear();
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);

}