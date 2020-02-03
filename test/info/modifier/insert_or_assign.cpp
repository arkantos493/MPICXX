/**
 * @file info/modifier/insert_or_assign.cpp
 * @author Marcel Breyer
 * @date 2020-02-03
 *
 * @brief Test cases for the @ref mpicxx::info::insert_or_assign(const std::string_view, const std::string_view),
 * @ref mpicxx::info::insert_or_assign(InputIt, InputIt) and @ref mpicxx::info::insert_or_assign(std::initializer_list<value_type>) member
 * functions provided by the @ref mpicxx::info class.
 * @details Testsuite: *ModifierTest*
 * | test case name                                 | test case description                                                            |
 * |:-----------------------------------------------|:---------------------------------------------------------------------------------|
 * | InsertOrAssignByKeyValuePair                   | insert or assign single [key, value]-pair                                        |
 * | InsertOrAssignByIllegalKeyOrValue              | try to insert or assign [key, value]-pair with illegal key or value (death test) |
 * | MovedFromInsertOrAssignByKeyValuePair          | info object in the moved-from state (death test)                                 |
 * | InsertOrAssignByIteratorRange                  | insert or assign all [key, value]-pairs from the iterator range                  |
 * | InsertOrAssignByIllegalIteratorRange           | iterator range is not valid (death test)                                         |
 * | InsertOrAssignByIllegalIteratorRangeKeyOrValue | key or value in the iterator range illegal (death test)                          |
 * | MovedFromInsertOrAssignByIteratorRange         | info object in the moved-from state (death test)                                 |
 * | InsertOrAssignByInitializerList                | insert or assign all [key, value]-pairs from the initializer list                |
 * | InsertOrAssignByInitializerListKeyOrValue      | key or value in the initializer list illegal (death test)                        |
 * | MovedFromInsertOrAssignByInitializerList       | info object in the moved-from state (death test)                                 |
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ModifierTest, InsertOrAssignByKeyValuePair) {
    // create empty info object
    mpicxx::info info;

    // insert or assign [key, value]-pair
    std::pair<mpicxx::info::const_iterator, bool> it_1 = info.insert_or_assign("key1", "value1");

    // the info object should contain one [key, value]-pair
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 1);

    EXPECT_TRUE(it_1.second);
    EXPECT_STREQ(it_1.first->first.c_str(), "key1");
    EXPECT_STREQ(it_1.first->second.c_str(), "value1");

    // insert or assign [key, value]-pair
    char value2[] = "value2";
    auto it_2 = info.insert_or_assign(std::string("key2"), value2);

    // the info object should contain two [key, value]-pairs
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 2);

    EXPECT_TRUE(it_2.second);
    EXPECT_STREQ(it_2.first->first.c_str(), "key2");
    EXPECT_STREQ(static_cast<std::string>(it_2.first->second).c_str(), "value2");

    // insert or assign [key, value]-pair
    auto it_3 = info.insert_or_assign("key2", "value2_override");

    // the info object should contain only two [key, value]-pairs
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 2);

    EXPECT_FALSE(it_3.second);
    EXPECT_STREQ(it_3.first->first.c_str(), "key2");
    EXPECT_STREQ(static_cast<std::string>(it_3.first->second).c_str(), "value2_override");
}

TEST(ModifierDeathTest, InsertOrAssignByIllegalKeyOrValue) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');
    std::string value(MPI_MAX_INFO_VAL, ' ');

    // try accessing illegal keys
    ASSERT_DEATH(info.insert_or_assign(key, "value"), "");
    ASSERT_DEATH(info.insert_or_assign("", "value"), "");

    // try accessing illegal value
    ASSERT_DEATH(info.insert_or_assign("key", value), "");
    ASSERT_DEATH(info.insert_or_assign("key", ""), "");
}

TEST(ModifierDeathTest, MovedFromInsertOrAssignByKeyValuePair) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // calling insert_or_assign() on an info object in the moved-from state is illegal
    ASSERT_DEATH(info.insert_or_assign("key", "value"), "");
}


TEST(ModifierTest, InsertOrAssignByIteratorRange) {
    // create empty info object
    mpicxx::info info;

    // create vector with all [key, value]-pairs
    std::vector<std::pair<const std::string, std::string>> key_value_pairs;
    key_value_pairs.emplace_back("key1", "value1");
    key_value_pairs.emplace_back("key2", "value2");
    key_value_pairs.emplace_back("key1", "value1_override");
    key_value_pairs.emplace_back("key3", "value3");

    // insert or assign [key, value]-pairs
    info.insert_or_assign(key_value_pairs.begin(), key_value_pairs.end());

    // the info object should contain three [key, value]-pairs
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 3);

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
}

TEST(ModifierDeathTest, InsertOrAssignByIllegalIteratorRange) {
    // create info object
    mpicxx::info info;

    // create vector with [key, value]-pair
    std::vector<mpicxx::info::value_type> vec = { { "key", "value" } };

    // try inserting or assigning with illegal iterator range
    ASSERT_DEATH(info.insert_or_assign(vec.end(), vec.begin()), "");
}

TEST(ModifierDeathTest, InsertOrAssignByIllegalIteratorRangeKeyOrValue) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');
    std::string value(MPI_MAX_INFO_VAL, ' ');

    // create vector with [key, value]-pairs
    std::vector<mpicxx::info::value_type> vec = { { key, "value" }, { "", "value" },
                                                  { "key", value }, { "key", ""   } };

    // try accessing illegal keys
    ASSERT_DEATH(info.insert_or_assign(vec.begin(), vec.begin() + 1), "");
    ASSERT_DEATH(info.insert_or_assign(vec.begin() + 1, vec.begin() + 2), "");

    // try accessing illegal value
    ASSERT_DEATH(info.insert_or_assign(vec.begin() + 2, vec.begin() + 3), "");
    ASSERT_DEATH(info.insert_or_assign(vec.begin() + 3, vec.end()), "");
}

TEST(ModifierDeathTest, MovedFromInsertOrAssignByIteratorRange) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // create vector with [key, value]-pair
    std::vector<mpicxx::info::value_type> vec = { { "key", "value" } };

    // calling insert_or_assign() on an info object in the moved-from state is illegal
    ASSERT_DEATH(info.insert_or_assign(vec.begin(), vec.end()), "");
}


TEST(ModifierTest, InsertOrAssignByInitializerList) {
    // create empty info object
    mpicxx::info info;

    // insert [key, value]-pairs
    info.insert_or_assign({ { "key1", "value1" }, { "key2", "value2" },
                            { "key1", "value1_override" }, { "key3", "value3" }});

    // the info object should contain three [key, value]-pairs
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    ASSERT_EQ(nkeys, 3);

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
}

TEST(ModifierDeathTest, InsertOrAssignByIllegalInitializerListKeyOrValue) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');
    std::string value(MPI_MAX_INFO_VAL, ' ');

    // try accessing illegal keys
    ASSERT_DEATH(info.insert_or_assign({ { key, "value" } }), "");
    ASSERT_DEATH(info.insert_or_assign({ { "", "value" }  }), "");

    // try accessing illegal value
    ASSERT_DEATH(info.insert_or_assign({ { "key", value } }), "");
    ASSERT_DEATH(info.insert_or_assign({ { "key", ""   }  }), "");
}

TEST(ModifierDeathTest, MovedFromInsertOrAssignByInitializerList) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // calling insert_or_assign() on an info object in the moved-from state is illegal
    ASSERT_DEATH(info.insert_or_assign({ { "key", "value" } }), "");
}
