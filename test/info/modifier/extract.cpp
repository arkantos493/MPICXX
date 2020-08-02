/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::extract(const_iterator) and @ref mpicxx::info::extract(const std::string_view) member
 *        functions provided by the @ref mpicxx::info class.
 * @details Testsuite: *ModifierTest*
 * | test case name                      | test case description                                                                                                    |
 * |:------------------------------------|:-------------------------------------------------------------------------------------------------------------------------|
 * | ExtractByIterator                   | extract [key, value]-pair at the given iterator position                                                                 |
 * | ExtractByIllegalIterator            | iterator doesn't refer to `*this` info object (death test)                                                               |
 * | ExtractByIteratorNotDereferenceable | iterator not dereferenceable (death test)                                                                                |
 * | NullExtractByIterator               | info object referring to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) (death test) |
 * | ExtractByKey                        | extract [key, value]-pair with the given key                                                                             |
 * | ExtractByIllegalKey                 | try to extract with an illegal key (death test)                                                                          |
 * | NullExtractByKey                    | info object referring to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) (death test) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

#include <optional>
#include <string>

TEST(ModifierTest, ExtractByIterator) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");
    MPI_Info_set(info.get(), "key3", "value3");

    // extract [key, value]-pair by iterator
    mpicxx::info::const_iterator it = info.begin() + 1;
    std::pair<const std::string, std::string> key_value_pair = info.extract(it);

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

    // check the extracted [key, value]-pair
    EXPECT_STREQ(key_value_pair.first.c_str(), "key2");
    EXPECT_STREQ(key_value_pair.second.c_str(), "value2");
}

TEST(ModifierDeathTest, ExtractByIllegalIterator) {
    // create info objects
    mpicxx::info info_1;
    MPI_Info_set(info_1.get(), "key", "value");

    mpicxx::info info_2;

    // extracting a [key, value]-pair using an iterator which refers to another info object is illegal
    mpicxx::info::const_iterator it = info_2.begin();
    [[maybe_unused]] std::pair<std::string, std::string> key_value_pair;
    ASSERT_DEATH( key_value_pair = info_1.extract(it) , "");
}

TEST(ModifierDeathTest, ExtractByIteratorNotDereferenceable) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key", "value");

    // extracting a [key, value]-pair using the past-the-end iterator is illegal
    mpicxx::info::const_iterator it = info.end();
    [[maybe_unused]] std::pair<std::string, std::string> key_value_pair;
    ASSERT_DEATH( key_value_pair = info.extract(it) , "");
}

TEST(ModifierDeathTest, NullExtractByIterator) {
    // create null info object
    mpicxx::info info;
    mpicxx::info::const_iterator it = info.begin();
    info = mpicxx::info(MPI_INFO_NULL, false);

    // calling extract() on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] std::pair<std::string, std::string> key_value_pair;
    ASSERT_DEATH( key_value_pair = info.extract(it) , "");
}

TEST(ModifierTest, ExtractByKey) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");
    MPI_Info_set(info.get(), "key3", "value3");

    // extract [key, value]-pair by key
    using value_type = mpicxx::info::value_type;
    std::optional<value_type> key_value_pair_1 = info.extract("key2");

    // check that the extraction was successful
    ASSERT_TRUE(key_value_pair_1.has_value());

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

    // check the extracted [key, value]-pair
    EXPECT_STREQ(key_value_pair_1->first.c_str(), "key2");
    EXPECT_STREQ(key_value_pair_1->second.c_str(), "value2");

    // extract [key, value]-pair by key (not existing)
    auto key_value_pair_2 = info.extract("key4");

    // check that the extraction wasn't successful
    ASSERT_FALSE(key_value_pair_2.has_value());

    // the info object should still contain only two [key, value]-pairs
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);
}

TEST(ModifierDeathTest, ExtractByIllegalKey) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');

    // try accessing illegal keys
    ASSERT_DEATH( info.extract(key) , "");
    ASSERT_DEATH( info.extract("") , "");
}

TEST(ModifierDeathTest, NullExtractByKey) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling extract() on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] std::optional<std::pair<std::string, std::string>> key_value_pair;
    ASSERT_DEATH( key_value_pair = info.extract("key") , "");
}