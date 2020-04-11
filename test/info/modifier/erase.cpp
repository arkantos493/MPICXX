/**
 * @file test/info/modifier/erase.cpp
 * @author Marcel Breyer
 * @date 2020-04-11
 *
 * @brief Test cases for the @ref mpicxx::info::erase(const_iterator), @ref mpicxx::info::erase(const_iterator, const_iterator)
 * and @ref mpicxx::info::erase(const std::string_view) member functions provided by the @ref mpicxx::info class.
 * @details Testsuite: *ModifierTest*
 * | test case name                         | test case description                                      |
 * |:---------------------------------------|:-----------------------------------------------------------|
 * | EraseByIterator                        | erase [key, value]-pair at the given iterator position     |
 * | EraseByIllegalIterator                 | iterator doesn't refer to `*this` info object (death test) |
 * | EraseByIteratorNotDereferenceable      | iterator not dereferenceable (death test)                  |
 * | NullEraseByIterator                    | info object referring to MPI_INFO_NULL (death test)        |
 * | EraseByIteratorRange                   | erase all [key, value]-pairs in the given iterator range   |
 * | EraseByIllegalIteratorRange            | iterator range is not valid (death test)                   |
 * | EraseByIteratorRangeNotDereferenceable | iterators not dereferenceable (death test)                 |
 * | NullEraseByIteratorRange               | info object referring to MPI_INFO_NULL (death test)        |
 * | EraseByKey                             | erase [key, value]-pair with the given key                 |
 * | EraseByIllegalKey                      | try to erase with an illegal key (death test)              |
 * | NullEraseByKey                         | info object referring to MPI_INFO_NULL (death test)        |
 */

#include <string>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ModifierTest, EraseByIterator) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");
    MPI_Info_set(info.get(), "key3", "value3");

    // erase [key, value]-pair by iterator
    mpicxx::info::const_iterator it = info.begin() + 1;
    auto key_value_pair_it = info.erase(it);

    // the info object should now contain only two [key, value]-pairs
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 2);

    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info.get(), "key1", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");
    MPI_Info_get(info.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");

    // check the returned iterator
    EXPECT_EQ(key_value_pair_it, it);

    // erase the last element
    key_value_pair_it = info.erase(info.end() - 1);

    // the info object should now contain only one [key, value]-pair
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 1);

    // check the returned iterator
    EXPECT_EQ(key_value_pair_it, info.end());
}

TEST(ModifierDeathTest, EraseByIllegalIterator) {
    // create info objects
    mpicxx::info info_1;
    MPI_Info_set(info_1.get(), "key", "value");

    mpicxx::info info_2;

    // erasing a [key, value]-pair using an iterator which refers to another info object is illegal
    mpicxx::info::const_iterator it = info_2.begin();
    ASSERT_DEATH( info_1.erase(it) , "");
}

TEST(ModifierDeathTest, EraseByIteratorNotDereferenceable) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key", "value");

    // erasing a [key, value]-pair using the past-the-end iterator is illegal
    ASSERT_DEATH( info.erase(info.end()) , "");
}

TEST(ModifierDeathTest, NullEraseByIterator) {
    // create null info object
    mpicxx::info info;
    mpicxx::info::const_iterator it = info.begin();
    info = mpicxx::info(MPI_INFO_NULL, false);

    // calling erase() on an info object referring to MPI_INFO_NULL is illegal
    ASSERT_DEATH( info.erase(it) , "");
}


TEST(ModifierTest, EraseByIteratorRange) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");
    MPI_Info_set(info.get(), "key3", "value3");

    // erase [key, value]-pair by iterator range
    auto key_value_pair_it = info.erase(info.begin() + 1, info.end());

    // the info object should now contain only one [key, value]-pair
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 1);

    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info.get(), "key1", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");

    // check the returned iterator
    EXPECT_EQ(key_value_pair_it, info.end());

    // erase the last element
    key_value_pair_it = info.erase(info.begin(), info.end());

    // the info object should now be empty
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 0);

    // check the returned iterator
    EXPECT_EQ(key_value_pair_it, info.end());
}

TEST(ModifierDeathTest, EraseByIllegalIteratorRange) {
    // create info objects
    mpicxx::info info_1;
    MPI_Info_set(info_1.get(), "key", "value");

    mpicxx::info info_2;

    // erasing a [key, value]-pair using an iterator which refers to another info object is illegal
    mpicxx::info::const_iterator it_1 = info_1.begin();
    mpicxx::info::const_iterator it_2 = info_2.begin();
    ASSERT_DEATH( info_1.erase(it_1, it_2) , "");
    ASSERT_DEATH( info_1.erase(it_2, it_1) , "");

    // erasing the range [last, first) is illegal
    ASSERT_DEATH( info_1.erase(info_1.end() , info_1.begin()), "");
}

TEST(ModifierDeathTest, EraseByIteratorRangeNotDereferenceable) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key", "value");

    // erasing a [key, value]-pair using the past-the-end iterator is illegal
    ASSERT_DEATH( info.erase(info.end(), info.end() + 1) , "");
    ASSERT_DEATH( info.erase(info.end() + 1, info.end()) , "");
}

TEST(ModifierDeathTest, NullEraseByIteratorRange) {
    // create null info object
    mpicxx::info info;
    mpicxx::info::const_iterator it = info.begin();
    info = mpicxx::info(MPI_INFO_NULL, false);

    // calling erase() on an info object referring to MPI_INFO_NULL is illegal
    ASSERT_DEATH( info.erase(it, it) , "");
}


TEST(ModifierTest, EraseByKey) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");
    MPI_Info_set(info.get(), "key3", "value3");

    // erase [key, value]-pair by key
    mpicxx::info::size_type count = info.erase("key2");

    // check that the extraction was successful
    ASSERT_EQ(count, 1);

    // the info object should now contain only two [key, value]-pairs
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);

    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info.get(), "key1", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");
    MPI_Info_get(info.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");

    // extract [key, value]-pair by key (not existing)
    count = info.erase("key4");

    // check that the extraction wasn't successful
    ASSERT_EQ(count, 0);

    // the info object should still contain only two [key, value]-pairs
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);
}

TEST(ModifierDeathTest, EraseByIllegalKey) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');

    // try accessing illegal keys
    ASSERT_DEATH( info.erase(key) , "");
    ASSERT_DEATH( info.erase("") , "");
}

TEST(ModifierDeathTest, NullEraseByKey) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling erase() on an info object referring to MPI_INFO_NULL is illegal
    ASSERT_DEATH( info.erase("key") , "");
}