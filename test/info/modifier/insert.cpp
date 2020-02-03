/**
 * @file info/modifier/insert.cpp
 * @author Marcel Breyer
 * @date 2020-02-03
 *
 * @brief Test cases for the @ref mpicxx::info::insert(const std::string_view, const std::string_view),
 * @ref mpicxx::info::insert(InputIt, InputIt) and @ref mpicxx::info::insert(std::initializer_list<value_type>) member functions provided
 * by the @ref mpicxx::info class.
 * @details Testsuite: *ModifierTest*
 * | test case name                         | test case description                                                  |
 * |:---------------------------------------|:-----------------------------------------------------------------------|
 * | InsertByKeyValuePair                   | insert single [key, value]-pair                                        |
 * | InsertByIllegalKeyOrValue              | try to insert [key, value]-pair with illegal key or value (death test) |
 * | MovedFromInsertByKeyValuePair          | info object in the moved-from state (death test)                       |
 * | InsertByIteratorRange                  | insert all [key, value]-pairs from the iterator range                  |
 * | InsertByIllegalIteratorRange           | iterator range is not valid (death test)                               |
 * | InsertByIllegalIteratorRangeKeyOrValue | key or value in the iterator range illegal (death test)                |
 * | MovedFromInsertByIteratorRange         | info object in the moved-from state (death test)                       |
 * | InsertByInitializerList                | insert all [key, value]-pairs from the initializer list                |
 * | InsertByInitializerListKeyOrValue      | key or value in the initializer list illegal (death test)              |
 * | MovedFromInsertByInitializerList       | info object in the moved-from state (death test)                       |
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
    ASSERT_DEATH(info.insert(key, "value"), "");
    ASSERT_DEATH(info.insert("", "value"), "");

    // try accessing illegal value
    ASSERT_DEATH(info.insert("key", value), "");
    ASSERT_DEATH(info.insert("key", ""), "");
}

TEST(ModifierDeathTest, MovedFromInsertByKeyValuePair) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // calling insert() on an info object in the moved-from state is illegal
    ASSERT_DEATH(info.insert("key", "value"), "");
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

TEST(ModifierDeathTest, InsertByIllegalIteratorRange) {
    // create info object
    mpicxx::info info;

    // create vector with [key, value]-pair
    std::vector<mpicxx::info::value_type> vec = { { "key", "value" } };

    // try inserting with illegal iterator range
    ASSERT_DEATH(info.insert(vec.end(), vec.begin()), "");
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
    ASSERT_DEATH(info.insert(vec.begin(), vec.begin() + 1), "");
    ASSERT_DEATH(info.insert(vec.begin() + 1, vec.begin() + 2), "");

    // try accessing illegal value
    ASSERT_DEATH(info.insert(vec.begin() + 2, vec.begin() + 3), "");
    ASSERT_DEATH(info.insert(vec.begin() + 3, vec.end()), "");
}

TEST(ModifierDeathTest, MovedFromInsertByIteratorRange) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // create vector with [key, value]-pair
    std::vector<mpicxx::info::value_type> vec = { { "key", "value" } };

    // calling insert() on an info object in the moved-from state is illegal
    ASSERT_DEATH(info.insert(vec.begin(), vec.end()), "");
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
    ASSERT_DEATH(info.insert({ { key, "value" } }), "");
    ASSERT_DEATH(info.insert({ { "", "value" }  }), "");

    // try accessing illegal value
    ASSERT_DEATH(info.insert({ { "key", value } }), "");
    ASSERT_DEATH(info.insert({ { "key", ""   }  }), "");
}

TEST(ModifierDeathTest, MovedFromInsertByInitializerList) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // calling insert() on an info object in the moved-from state is illegal
    ASSERT_DEATH(info.insert({ { "key", "value" } }), "");
}
