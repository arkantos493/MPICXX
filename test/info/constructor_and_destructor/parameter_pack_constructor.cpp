/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::info(T&&...) member function provided by the  @ref mpicxx::info class.
 * @details Testsuite: *ConstructionTest*
 * | test case name                 | test case description                                               |
 * |:-------------------------------|:--------------------------------------------------------------------|
 * | ParameterPackConstruction      | construct info object from a parameter pack                         |
 * | ParameterPackIllegalKeyOrValue | try to construct info object from an illegal key/value (death test) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

#include <string>
#include <utility>
#include <vector>

TEST(ConstructionTest, ParameterPackConstruction) {
    // create [key, value]-pairs
    std::pair<const std::string, std::string> p1("key1", "value1");
    std::pair<const std::string, std::string> p2("key2", "value2");

    // construct an info object from a parameter pack
    mpicxx::info info(p1, p2, std::make_pair("key1", "value1_override"), std::make_pair("key3", "value3"));

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

    // an info object constructed from a parameter pack is always freeable
    EXPECT_TRUE(info.freeable());
}

TEST(ConstructionDeathTest, ParameterPackIllegalKeyOrValue) {
    std::string key(MPI_MAX_INFO_KEY, ' ');
    std::string value(MPI_MAX_INFO_VAL, ' ');

    // create info object from parameter pack with illegal key
    ASSERT_DEATH( mpicxx::info info(std::make_pair(key, "value")) , "");
    ASSERT_DEATH( mpicxx::info info(std::make_pair("", "value")) , "");

    // create info object from parameter pack with illegal value
    ASSERT_DEATH( mpicxx::info info(std::make_pair("key", value)) , "");
    ASSERT_DEATH( mpicxx::info info(std::make_pair("key", "")) , "");
}