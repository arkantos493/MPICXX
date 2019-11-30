/**
 * @file info_iterator_test.cpp
 * @author Marcel Breyer
 * @date 2019-11-30
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the iterators of a mpicxx::info object.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>

TEST(InfoTests, Iterator) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // create a copy
    mpicxx::info info_copy(info);
    mpicxx::info::iterator it_comp = info.begin();

    // check relational operator: ==
    EXPECT_TRUE(it_comp == info.begin());           // normal comparison between iterators
    EXPECT_FALSE(it_comp == info.end());            // normal comparison between iterators
    EXPECT_FALSE(info.end() == it_comp);            // normal comparison between iterators
    EXPECT_TRUE(it_comp == info.cbegin());          // it should be possible to compare iterators and const_iterators
    EXPECT_FALSE(it_comp == info.cend());           // it should be possible to compare iterators and const_iterators
//    EXPECT_FALSE(it_comp == info_copy.begin());     // don't compare iterators from different info objects -> should ASSERT in debug mode
//    EXPECT_FALSE(it_comp == info.rbegin());         // it should NOT be possible to compare iterators and reverse_iterators -> shouldn't compile

    // check relational operator: !=
    EXPECT_FALSE(it_comp != info.begin());          // normal comparison between iterators
    EXPECT_TRUE(it_comp != info.end());             // normal comparison between iterators
    EXPECT_TRUE(info.end() != it_comp);             // normal comparison between iterators
    EXPECT_FALSE(it_comp != info.cbegin());         // it should be possible to compare iterators and const_iterators
    EXPECT_TRUE(it_comp != info.cend());            // it should be possible to compare iterators and const_iterators
//    EXPECT_FALSE(it_comp != info_copy.begin());     // don't compare iterators from different info objects -> should ASSERT in debug mode
//    EXPECT_FALSE(it_comp != info.rbegin());         // it should NOT be possible to compare iterators and reverse_iterators -> shouldn't compile

    // check relational operator: <
    EXPECT_FALSE(it_comp < info.begin());           // normal comparison between iterators
    EXPECT_TRUE(it_comp < info.end());              // normal comparison between iterators
    EXPECT_FALSE(info.end() < it_comp);             // normal comparison between iterators
    EXPECT_FALSE(it_comp < info.cbegin());          // it should be possible to compare iterators and const_iterators
    EXPECT_TRUE(it_comp < info.cend());             // it should be possible to compare iterators and const_iterators
//    EXPECT_FALSE(it_comp < info_copy.begin());      // don't compare iterators from different info objects -> should ASSERT in debug mode
//    EXPECT_FALSE(it_comp < info.rbegin());          // it should NOT be possible to compare iterators and reverse_iterators -> shouldn't compile

    // check relational operator: >
    EXPECT_FALSE(it_comp > info.begin());           // normal comparison between iterators
    EXPECT_FALSE(it_comp > info.end());             // normal comparison between iterators
    EXPECT_TRUE(info.end() > it_comp);              // normal comparison between iterators
    EXPECT_FALSE(it_comp > info.cbegin());          // it should be possible to compare iterators and const_iterators
    EXPECT_FALSE(it_comp > info.cend());            // it should be possible to compare iterators and const_iterators
//    EXPECT_FALSE(it_comp > info_copy.begin());      // don't compare iterators from different info objects -> should ASSERT in debug mode
//    EXPECT_FALSE(it_comp > info.rbegin());          // it should NOT be possible to compare iterators and reverse_iterators -> shouldn't compile

    // check relational operator: <=
    EXPECT_TRUE(it_comp <= info.begin());           // normal comparison between iterators
    EXPECT_TRUE(it_comp <= info.end());             // normal comparison between iterators
    EXPECT_FALSE(info.end() <= it_comp);            // normal comparison between iterators
    EXPECT_TRUE(it_comp <= info.cbegin());          // it should be possible to compare iterators and const_iterators
    EXPECT_TRUE(it_comp <= info.cend());            // it should be possible to compare iterators and const_iterators
//    EXPECT_FALSE(it_comp <= info_copy.begin());     // don't compare iterators from different info objects -> should ASSERT in debug mode
//    EXPECT_FALSE(it_comp <= info.rbegin());         // it should NOT be possible to compare iterators and reverse_iterators -> shouldn't compile

    // check relational operator: >=
    EXPECT_TRUE(it_comp >= info.begin());            // normal comparison between iterators
    EXPECT_FALSE(it_comp >= info.end());             // normal comparison between iterators
    EXPECT_TRUE(info.end() >= it_comp);              // normal comparison between iterators
    EXPECT_TRUE(it_comp >= info.cbegin());           // it should be possible to compare iterators and const_iterators
    EXPECT_FALSE(it_comp >= info.cend());            // it should be possible to compare iterators and const_iterators
//    EXPECT_FALSE(it_comp >= info_copy.begin());      // don't compare iterators from different info objects -> should ASSERT in debug mode
//    EXPECT_FALSE(it_comp >= info.rbegin());          // it should NOT be possible to compare iterators and reverse_iterators -> shouldn't compile


    // check if increment works
    mpicxx::info::iterator it_pre_inc = info.begin();
    ++it_pre_inc;
    mpicxx::info::iterator it_post_inc = info.begin();
    it_post_inc++;
    EXPECT_TRUE(it_pre_inc == it_post_inc);
    mpicxx::info::iterator it_compound_add = info.begin();
    it_compound_add += 1;
    EXPECT_TRUE(it_pre_inc == it_compound_add);
    mpicxx::info::iterator it_add = info.begin();
    it_add = it_add + 1;
    EXPECT_TRUE(it_pre_inc == it_add);
    it_add = 1 + it_add;
    EXPECT_TRUE(it_add == (info.begin() + 2));

    // check if decrement works
    mpicxx::info::iterator it_pre_dec = info.end();
    --it_pre_dec;
    mpicxx::info::iterator it_post_dec = info.end();
    it_post_dec--;
    EXPECT_TRUE(it_pre_dec == it_post_dec);
    mpicxx::info::iterator it_compound_sub = info.end();
    it_compound_sub -= 1;
    EXPECT_TRUE(it_pre_dec == it_compound_sub);
    mpicxx::info::iterator it_sub = info.end();
    it_sub = it_sub - 1;
    EXPECT_TRUE(it_pre_dec == it_sub);

    // check distance calculation
    EXPECT_EQ(info.end() - info.begin(), 4);         // normal difference between iterators
    EXPECT_EQ(info.begin() - info.end(), -4);        // normal difference between iterators
    EXPECT_EQ(info.end() - info.begin(), std::distance(info.begin(), info.end()));  // should be the same as std::distance
    EXPECT_EQ(info.end() - info.cbegin(), 4);        // it should be possible to calculate the difference between iterators and const_iterators
    EXPECT_EQ(info.cend() - info.begin(), 4);        // it should be possible to calculate the difference between iterators and const_iterators
//    EXPECT_EQ(info.end() - info_copy.begin(), 4);    // don't calculate the difference between iterators from different info objects -> should ASSERT in debug mode
//    EXPECT_EQ(info.end() - info.rbegin(), 4);    // it should NOT be possible to calculate the difference between iterators and reverse_iterators -> shouldn't compile


    // test dereferencing operator: *
    mpicxx::info info_dereference(info);

    // non-const iterator
    mpicxx::info::iterator it_dereference = info_dereference.begin();
    auto key_value_pair = *it_dereference;
    EXPECT_STREQ(key_value_pair.first.c_str(), "key1");
    EXPECT_STREQ(static_cast<std::string>(key_value_pair.second).c_str(), "value1");
    // check modifying through non-const iterator
    (*it_dereference).second = "value_override1";
    const std::string value = (*it_dereference).second;
    EXPECT_STREQ(value.c_str(), "value_override1");

    // const_iterator
    mpicxx::info::const_iterator const_it_dereference = info_dereference.begin();
    auto const_key_value_pair = *const_it_dereference;
    EXPECT_STREQ(const_key_value_pair.first.c_str(), "key1");
    EXPECT_STREQ(const_key_value_pair.second.c_str(), "value_override1");
    // modifying through const_iterator not allowed -> compile error
//    (*const_it_dereference).second = "value1";

    // test dereferencing operator: ->

    // non-const iterator
    it_dereference = info_dereference.begin();
    EXPECT_STREQ(it_dereference->first.c_str(), "key1");
    EXPECT_STREQ(static_cast<std::string>(it_dereference->second).c_str(), "value_override1");
    it_dereference->second = "value1";
    const std::string value_2 = it_dereference->second;
    EXPECT_STREQ(value_2.c_str(), "value1");
    // modifying through const_iterator not allowed -> compile error
//    const_it_dereference->second = "value1";

    // test dereferencing operator: []
    // non-const iterator
    it_dereference = info_dereference.begin();
    auto key_value_pair_3 = it_dereference[1];
    EXPECT_STREQ(key_value_pair_3.first.c_str(), "key2");
    EXPECT_STREQ(static_cast<std::string>(key_value_pair_3.second).c_str(), "value2");
    // check modifying through non-const iterator
    it_dereference[2].second = "value_override3";
    const std::string value_3 = it_dereference[2].second;
    EXPECT_STREQ(value_3.c_str(), "value_override3");

    // const_iterator
    const_it_dereference = info_dereference.begin();
    auto const_key_value_pair_3 = *const_it_dereference;
    EXPECT_STREQ(const_key_value_pair_3.first.c_str(), "key1");
    EXPECT_STREQ(const_key_value_pair_3.second.c_str(), "value1");
    // modifying through const_iterator not allowed -> compile error
//    (*const_it_dereference).second = "value1";

    // loops

    // check all [key, value]-pairs using a range based for loop
    std::string keys = "";
    std::string values = "";
    for (const auto& [key, value]: info) {
        keys += key;
        values += value;
    }
    EXPECT_STREQ(keys.c_str(), "key1key2key3key4");
    EXPECT_STREQ(values.c_str(), "value1value2value3value4");

    // check all [key, value]-pairs using an iterator for loop
    keys = "";
    values = "";
    for (auto it = info.begin(), end = info.end(); it != end; ++it) {
        keys += it->first;
        values += it->second;
    }
    EXPECT_STREQ(keys.c_str(), "key1key2key3key4");
    EXPECT_STREQ(values.c_str(), "value1value2value3value4");

}

TEST(InfoTests, ReverseIterator) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = {{ "key1", "value1" },
                         { "key2", "value2" },
                         { "key3", "value3" },
                         { "key4", "value4" }};

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // test reverse loop
    std::string keys = "";
    std::string values = "";
    for (auto it = info.rbegin(), end = info.rend(); it != end; ++it) {
        keys += it->first;
        values += it->second;
    }
    EXPECT_STREQ(keys.c_str(), "key4key3key2key1");
    EXPECT_STREQ(values.c_str(), "value4value3value2value1");

}