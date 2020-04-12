/**
 * @file test/info/lookup/equal_range.cpp
 * @author Marcel Breyer
 * @date 2020-04-11
 *
 * @brief Test cases for the @ref mpicxx::info::equal_range(const std::string_view) and
 * @ref mpicxx::info::equal_range(const std::string_view) const member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *LookupTest*
 * | test case name                   | test case description                                        |
 * |:---------------------------------|:-------------------------------------------------------------|
 * | EqualRangeExisting               | find key in info object                                      |
 * | ConstEqualRangeExisting          | find key in const info object                                |
 * | EqualRangeNonExisting            | find non-existing key in info object                         |
 * | ConstEqualRangeNonExisting       | find non-existing key in const info object                   |
 * | NullEqualRangeExisting           | info object referring to MPI_INFO_NULL (death test)          |
 * | NullConstEqualRangeExisting      | const info object referring to MPI_INFO_NULL (death test)    |
 * | EqualRangeWithIllegalKey         | try to find an illegal key in info object (death test)       |
 * | ConstEqualRangeWithIllegalKey    | try to find an illegal key in const info object (death test) |
 */

#include <string>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(LookupTest, EqualRangeExisting) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // try finding the keys
    std::pair<mpicxx::info::iterator, mpicxx::info::iterator> it_pair_1 = info.equal_range("key1");
    ASSERT_NE(it_pair_1.first, info.end());
    ASSERT_NE(it_pair_1.second, info.end());
    ASSERT_EQ(it_pair_1.first + 1, it_pair_1.second);
    EXPECT_STREQ(it_pair_1.first->first.c_str(), "key1");
    std::string value_1 = it_pair_1.first->second;
    EXPECT_STREQ(value_1.c_str(), "value1");

    auto it_pair_2 = info.equal_range("key2");
    ASSERT_NE(it_pair_2.first, info.end());
    ASSERT_EQ(it_pair_2.second, info.end());
    ASSERT_EQ(it_pair_2.first + 1, it_pair_2.second);
    EXPECT_STREQ(it_pair_2.first->first.c_str(), "key2");
    std::string value_2 = it_pair_2.first->second;
    EXPECT_STREQ(value_2.c_str(), "value2");
}

TEST(LookupTest, ConstEqualRangeExisting) {
    // create info object with [key, value]-pairs
    const mpicxx::info info = { { "key1", "value1" } , { "key2", "value2" } };

    // try finding the keys
    std::pair<mpicxx::info::const_iterator, mpicxx::info::const_iterator> it_pair_1 = info.equal_range("key1");
    ASSERT_NE(it_pair_1.first, info.end());
    ASSERT_NE(it_pair_1.second, info.end());
    ASSERT_EQ(it_pair_1.first + 1, it_pair_1.second);
    EXPECT_STREQ(it_pair_1.first->first.c_str(), "key1");
    EXPECT_STREQ(it_pair_1.first->second.c_str(), "value1");

    auto it_pair_2 = info.equal_range("key2");
    ASSERT_NE(it_pair_2.first, info.end());
    ASSERT_EQ(it_pair_2.second, info.end());
    ASSERT_EQ(it_pair_2.first + 1, it_pair_2.second);
    EXPECT_STREQ(it_pair_2.first->first.c_str(), "key2");
    EXPECT_STREQ(it_pair_2.first->second.c_str(), "value2");
}

TEST(LookupTest, EqualRangeNonExisting) {
    // create info object and add [key, value]-pair
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // try finding non-existing key
    auto it_pair = info.equal_range("key2");
    EXPECT_EQ(it_pair.first, info.end());
    EXPECT_EQ(it_pair.second, info.end());
    EXPECT_EQ(it_pair.first, it_pair.second);
}

TEST(LookupTest, ConstEqualRangeNonExisting) {
    // create empty info object
    const mpicxx::info info;

    // try finding non-existing key
    auto it_pair = info.equal_range("key");
    EXPECT_EQ(it_pair.first, info.end());
    EXPECT_EQ(it_pair.second, info.end());
    EXPECT_EQ(it_pair.first, it_pair.second);
}

TEST(LookupDeathTest, NullEqualRange) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling equal_range() on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] std::pair<mpicxx::info::iterator, mpicxx::info::iterator> it_pair;
    ASSERT_DEATH( it_pair = info.equal_range("key") , "");
}

TEST(LookupDeathTest, NullConstEqualRange) {
    // create const null info object
    const mpicxx::info info(MPI_INFO_NULL, false);

    // calling equal_range() const on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] std::pair<mpicxx::info::const_iterator, mpicxx::info::const_iterator> it_pair;
    ASSERT_DEATH( it_pair = info.equal_range("key") , "");
}


TEST(LookupDeathTest, EqualRangeWithIllegalKey) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');

    // try to find an illegal key
    [[maybe_unused]] std::pair<mpicxx::info::iterator, mpicxx::info::iterator> it_pair;
    ASSERT_DEATH( it_pair = info.equal_range(key) , "");
    ASSERT_DEATH( it_pair = info.equal_range("") , "");
}

TEST(LookupDeathTest, ConstEqualRangeWithIllegalKey) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');

    // try to find an illegal key
    [[maybe_unused]] std::pair<mpicxx::info::const_iterator, mpicxx::info::const_iterator> it_pair;
    ASSERT_DEATH( it_pair = info.equal_range(key) , "");
    ASSERT_DEATH( it_pair = info.equal_range("") , "");
}