/**
 * @file test/info/modifier/insert.cpp
 * @author Marcel Breyer
 * @date 2020-06-25
 *
 * @brief Test cases for the @ref mpicxx::info::insert(const std::string_view, const std::string_view),
 * @ref mpicxx::info::insert(InputIt, InputIt) and @ref mpicxx::info::insert(std::initializer_list<value_type>) member functions provided
 * by the @ref mpicxx::info class.
 * @details Testsuite: *ModifierTest*
 * | test case name                         | test case description                                                               |
 * |:---------------------------------------|:------------------------------------------------------------------------------------|
 * | InsertByKeyValuePair                   | insert single [key, value]-pair                                                     |
 * | InsertByIllegalKeyOrValue              | try to insert [key, value]-pair with illegal key or value (death test)              |
 * | NullInsertByKeyValuePair               | info object referring to MPI_INFO_NULL (death test)                                 |
 * | InsertByIteratorRange                  | insert all [key, value]-pairs from the iterator range                               |
 * | InsertByIteratorRangeFromInfo          | insert all [key, value]-pairs from the iterator range retrieved from an info object |
 * | InsertByIllegalIteratorRange           | iterator range is not valid (death test)                                            |
 * | InsertByIllegalIteratorRangeKeyOrValue | key or value in the iterator range illegal (death test)                             |
 * | NullInsertByIteratorRange              | info object referring to MPI_INFO_NULL (death test)                                 |
 * | InsertByInitializerList                | insert all [key, value]-pairs from the initializer list                             |
 * | InsertByInitializerListKeyOrValue      | key or value in the initializer list illegal (death test)                           |
 * | NullInsertByInitializerList            | info object referring to MPI_INFO_NULL (death test)                                 |
 * | InsertByParameterPack                  | insert all [key, value]-pairs from the parameter pack                               |
 * | InsertByParameterPackIllegalKeyOrValue | key or value in the parameter pack illegal (death test)                             |
 * | NullInsertByParameterPack              | info object referring to MPI_INFO_NULL (death test)                                 |
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ModifierTest, InsertByKeyValuePair) {
    // create empty info object
    mpicxx::info info;

    // insert [key, value]-pair
    std::pair<mpicxx::info::const_iterator, bool> it_1 = info.insert("key1", "value1");

    // the info object should contain one [key, value]-pair
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 1);

    EXPECT_TRUE(it_1.second);
    EXPECT_STREQ(it_1.first->first.c_str(), "key1");
    EXPECT_STREQ(it_1.first->second.c_str(), "value1");

    // insert [key, value]-pair
    char value2[] = "value2";
    auto it_2 = info.insert(std::string("key2"), value2);

    // the info object should contain two [key, value]-pairs
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 2);

    EXPECT_TRUE(it_2.second);
    EXPECT_STREQ(it_2.first->first.c_str(), "key2");
    EXPECT_STREQ(static_cast<std::string>(it_2.first->second).c_str(), "value2");

    // insert [key, value]-pair
    auto it_3 = info.insert("key2", "value2_override");

    // the info object should contain only two [key, value]-pairs
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 2);

    EXPECT_FALSE(it_3.second);
    EXPECT_STREQ(it_3.first->first.c_str(), "key2");
    EXPECT_STREQ(static_cast<std::string>(it_3.first->second).c_str(), "value2");
}

TEST(ModifierDeathTest, InsertByIllegalKeyOrValue) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');
    std::string value(MPI_MAX_INFO_VAL, ' ');

    // try accessing illegal keys
    ASSERT_DEATH( info.insert(key, "value") , "");
    ASSERT_DEATH( info.insert("", "value") , "");

    // try accessing illegal value
    ASSERT_DEATH( info.insert("key", value) , "");
    ASSERT_DEATH( info.insert("key", "") , "");
}

TEST(ModifierDeathTest, NullInsertByKeyValuePair) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling insert() on an info object referring to MPI_INFO_NULL is illegal
    ASSERT_DEATH( info.insert("key", "value") , "");
}


TEST(ModifierTest, InsertByIteratorRange) {
    // create empty info object
    mpicxx::info info;

    // create vector with all [key, value]-pairs
    std::vector<std::pair<const std::string, std::string>> key_value_pairs;
    key_value_pairs.emplace_back("key1", "value1");
    key_value_pairs.emplace_back("key2", "value2");
    key_value_pairs.emplace_back("key1", "value1_override");
    key_value_pairs.emplace_back("key3", "value3");

    // insert [key, value]-pairs
    info.insert(key_value_pairs.begin(), key_value_pairs.end());

    // the info object should contain three [key, value]-pairs
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 3);

    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info.get(), "key1", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");
    MPI_Info_get(info.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
    MPI_Info_get(info.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");
}

TEST(ModifierTest, InsertByIteratorRangeFromInfo) {
    // create info objects and add [key, value]-pairs
    mpicxx::info info_1;
    MPI_Info_set(info_1.get(), "key1", "value1");
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key1", "value1_override");
    MPI_Info_set(info_2.get(), "key2", "value2");
    MPI_Info_set(info_2.get(), "key3", "value3");

    // insert all elements from info_2 in info_1
    info_1.insert(info_2.begin(), info_2.end());

    // check info_1 for the correct values
    int nkeys;
    MPI_Info_get_nkeys(info_1.get(), &nkeys);
    ASSERT_EQ(nkeys, 3);

    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info_1.get(), "key1", 6, value, &flag);
    ASSERT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");
    MPI_Info_get(info_1.get(), "key2", 6, value, &flag);
    ASSERT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
    MPI_Info_get(info_1.get(), "key2", 6, value, &flag);
    ASSERT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
}

TEST(ModifierDeathTest, InsertByIllegalIteratorRange) {
    // create info object
    mpicxx::info info;

    // create vector with [key, value]-pair
    std::vector<mpicxx::info::value_type> vec = { { "key", "value" } };

    // try inserting with illegal iterator range
    ASSERT_DEATH( info.insert(vec.end(), vec.begin()) , "");
}

TEST(ModifierDeathTest, InsertByIllegalIteratorRangeKeyOrValue) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');
    std::string value(MPI_MAX_INFO_VAL, ' ');

    // create vector with [key, value]-pairs
    std::vector<mpicxx::info::value_type> vec = { { key, "value" }, { "", "value" },
                                                  { "key", value }, { "key", ""   } };

    // try accessing illegal keys
    ASSERT_DEATH( info.insert(vec.begin(), vec.begin() + 1) , "");
    ASSERT_DEATH( info.insert(vec.begin() + 1, vec.begin() + 2) , "");

    // try accessing illegal value
    ASSERT_DEATH( info.insert(vec.begin() + 2, vec.begin() + 3) , "");
    ASSERT_DEATH( info.insert(vec.begin() + 3, vec.end()) , "");
}

TEST(ModifierDeathTest, NullInsertByIteratorRange) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // create vector with [key, value]-pair
    std::vector<mpicxx::info::value_type> vec = { { "key", "value" } };

    // calling insert() on an info object referring to MPI_INFO_NULL is illegal
    ASSERT_DEATH( info.insert(vec.begin(), vec.end()) , "");
}


TEST(ModifierTest, InsertByInitializerList) {
    // create empty info object
    mpicxx::info info;

    // insert [key, value]-pairs
    info.insert({ { "key1", "value1" }, { "key2", "value2" },
                  { "key1", "value1_override" }, { "key3", "value3" }});

    // the info object should contain three [key, value]-pairs
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 3);

    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info.get(), "key1", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");
    MPI_Info_get(info.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
    MPI_Info_get(info.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");
}

TEST(ModifierDeathTest, InsertByIllegalInitializerListKeyOrValue) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');
    std::string value(MPI_MAX_INFO_VAL, ' ');

    // try accessing illegal keys
    ASSERT_DEATH( info.insert({ { key, "value" } }) , "");
    ASSERT_DEATH( info.insert({ { "", "value" }  }) , "");

    // try accessing illegal value
    ASSERT_DEATH( info.insert({ { "key", value } }) , "");
    ASSERT_DEATH( info.insert({ { "key", ""   }  }) , "");
}

TEST(ModifierDeathTest, NullInsertByInitializerList) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling insert() on an info object referring to MPI_INFO_NULL is illegal
    ASSERT_DEATH( info.insert({ { "key", "value" } }) , "");
}


TEST(ModifierTest, InsertByParameterPack) {
    // create empty info object
    mpicxx::info info;

    // create [key, value]-pairs
    std::pair<const std::string, std::string> p1("key1", "value1");
    std::pair<const std::string, std::string> p2("key2", "value2");

    // insert [key, value]-pairs
    info.insert(p1, p2, std::make_pair("key1", "value1_override"));
    info.insert(std::make_pair("key3", "value3"));

    // the info object should contain three [key, value]-pairs
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 3);

    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info.get(), "key1", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");
    MPI_Info_get(info.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
    MPI_Info_get(info.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");
}

TEST(ModifierDeathTest, InsertByParameterPackIllegalKeyOrValue) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');
    std::string value(MPI_MAX_INFO_VAL, ' ');

    // try using illegal keys
    ASSERT_DEATH( info.insert(std::make_pair(key, "value")) , "");
    ASSERT_DEATH( info.insert(std::make_pair("", "value")) , "");

    // try using illegal value
    ASSERT_DEATH( info.insert(std::make_pair("key", value)) , "");
    ASSERT_DEATH( info.insert(std::make_pair("key", "")) , "");
}

TEST(ModifierDeathTest, NullInsertByParameterPack) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling insert() on an info object referring to MPI_INFO_NULL is illegal
    ASSERT_DEATH( info.insert(std::make_pair("key", "value")) , "");
}