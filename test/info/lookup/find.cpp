/**
 * @file test/info/lookup/find.cpp
 * @author Marcel Breyer
 * @date 2020-04-11
 *
 * @brief Test cases for the @ref mpicxx::info::find(const std::string_view) and @ref mpicxx::info::find(const std::string_view) const
 * member functions provided by the @ref mpicxx::info
 * class.
 * @details Testsuite: *LookupTest*
 * | test case name          | test case description                                        |
 * |:------------------------|:-------------------------------------------------------------|
 * | FindExisting            | find key in info object                                      |
 * | ConstFindExisting       | find key in const info object                                |
 * | FindNonExisting         | find non-existing key in info object                         |
 * | ConstFindNonExisting    | find non-existing key in const info object                   |
 * | NullFind                | info object referring to MPI_INFO_NULL (death test)          |
 * | NullConstFind           | const info object referring to MPI_INFO_NULL (death test)    |
 * | FindWithIllegalKey      | try to find an illegal key in info object (death test)       |
 * | ConstFindWithIllegalKey | try to find an illegal key in const info object (death test) |
 */

#include <string>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(LookupTest, FindExisting) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // try finding the keys
    mpicxx::info::iterator it_1 = info.find("key1");
    ASSERT_NE(it_1, info.end());
    EXPECT_STREQ(it_1->first.c_str(), "key1");
    std::string value_1 = it_1->second;
    EXPECT_STREQ(value_1.c_str(), "value1");

    auto it_2 = info.find("key2");
    ASSERT_NE(it_2, info.end());
    EXPECT_STREQ(it_2->first.c_str(), "key2");
    std::string value_2 = it_2->second;
    EXPECT_STREQ(value_2.c_str(), "value2");
}

TEST(LookupTest, ConstFindExisting) {
    // create const info object with [key, value]-pairs
    const mpicxx::info info = { { "key1", "value1" } , { "key2", "value2" } };

    // try finding the keys
    mpicxx::info::const_iterator it_1 = info.find("key1");
    ASSERT_NE(it_1, info.cend());
    EXPECT_STREQ(it_1->first.c_str(), "key1");
    EXPECT_STREQ(it_1->second.c_str(), "value1");

    auto it_2 = info.find("key2");
    ASSERT_NE(it_2, info.cend());
    EXPECT_STREQ(it_2->first.c_str(), "key2");
    EXPECT_STREQ(it_2->second.c_str(), "value2");
}

TEST(LookupTest, FindNonExisting) {
    // create info object and add [key, value]-pair
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // try finding non-existing key
    mpicxx::info::iterator it = info.find("key2");
    EXPECT_EQ(it, info.end());
}

TEST(LookupTest, ConstFindNonExisting) {
    // create empty const info object
    const mpicxx::info info;

    // try finding non-existing key
    mpicxx::info::const_iterator it = info.find("key");
    EXPECT_EQ(it, info.cend());
}

TEST(LookupDeathTest, NullFind) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling find() on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] mpicxx::info::iterator it;
    ASSERT_DEATH( it = info.find("key") , "");
}

TEST(LookupDeathTest, NullConstFind) {
    // create const null info object
    const mpicxx::info info(MPI_INFO_NULL, false);

    // calling find() const on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] mpicxx::info::const_iterator it;
    ASSERT_DEATH( it = info.find("key") , "");
}

TEST(LookupDeathTest, FindWithIllegalKey) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');

    // try to find an illegal key
    [[maybe_unused]] mpicxx::info::iterator it;
    ASSERT_DEATH( it = info.find(key) , "");
    ASSERT_DEATH( it = info.find("") , "");
}

TEST(LookupDeathTest, ConstFindWithIllegalKey) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');

    // try to find an illegal key
    [[maybe_unused]] mpicxx::info::const_iterator it;
    ASSERT_DEATH( it = info.find(key) , "");
    ASSERT_DEATH( it = info.find("") , "");
}