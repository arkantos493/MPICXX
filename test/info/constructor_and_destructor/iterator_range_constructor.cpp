/**
 * @file info/constructor_and_destructor/iterator_range_constructor.cpp
 * @author Marcel Breyer
 * @date 2020-01-31
 *
 * @brief Test cases for the @ref mpicxx::info::info(InputIter, InputIter) member function provided by the  @ref mpicxx::info class.
 * @details Testsuite: *ConstructionTest*
 * | test case name                   | test case description                                               |
 * |:---------------------------------|:--------------------------------------------------------------------|
 * | IteratorRangeConstruction        | construct info object from an iterator range                        |
 * | EmptyIteratorRangeConstruction   | construct info object from an empty iterator range                  |
 * | InvalidIteratorRangeConstruction | illegal iterator range (death test)                                 |
 * | IteratorRangeIllegalKeyOrValue   | try to construct info object from an illegal key/value (death test) |
 */

#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ConstructionTest, IteratorRangeConstruction) {
    // create vector with all [key, value]-pairs
    std::vector<std::pair<const std::string, std::string>> key_value_pairs;
    key_value_pairs.emplace_back("key1", "value1");
    key_value_pairs.emplace_back("key2", "value2");
    key_value_pairs.emplace_back("key1", "value1_override");
    key_value_pairs.emplace_back("key3", "value3");

    // construct an info object from an iterator range
    mpicxx::info info(key_value_pairs.cbegin(), key_value_pairs.cend());

    // info object should now contain three entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 3);

    // check if all [key, value]-pairs were added
    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info.get(), "key1", 15, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
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

TEST(ConstructionTest, EmptyIteratorRangeConstruction) {
    // create empty vector
    std::vector<std::pair<const std::string, std::string>> key_value_pairs;

    // construct an info object from an empty iterator range
    mpicxx::info info(key_value_pairs.begin(), key_value_pairs.end());

    // info object should be empty
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);

    // an info object constructed from an iterator range is always freeable
    EXPECT_TRUE(info.freeable());
}

TEST(ConstructionDeathTest, InvalidIteratorRangeConstruction) {
    // create vector with all [key, value]-pairs
    std::vector<std::pair<const std::string, std::string>> key_value_pairs;
    key_value_pairs.emplace_back("key1", "value1");

    // iterator range construction where first > last is illegal
    ASSERT_DEATH( mpicxx::info info(key_value_pairs.end(), key_value_pairs.begin()) , "");

}

TEST(ConstructionDeathTest, IteratorRangeIllegalKeyOrValue) {
    std::string key(MPI_MAX_INFO_KEY, ' ');
    std::string value(MPI_MAX_INFO_VAL, ' ');

    std::vector<std::pair<const std::string, std::string>> vec;
    vec.emplace_back(std::make_pair(key, "value"));
    vec.emplace_back(std::make_pair("", "value"));
    vec.emplace_back(std::make_pair("key", value));
    vec.emplace_back(std::make_pair("key", ""));

    // create info object from iterator range with illegal key
    ASSERT_DEATH( mpicxx::info info(vec.begin(), vec.begin() + 1) , "");
    ASSERT_DEATH( mpicxx::info info(vec.begin() + 1, vec.begin() + 2) , "");

    // create info object from iterator range with illegal value
    ASSERT_DEATH( mpicxx::info info(vec.begin() + 2, vec.begin() + 3) , "");
    ASSERT_DEATH( mpicxx::info info(vec.begin() + 3, vec.end()) , "");
}