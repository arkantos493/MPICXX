/**
 * @file info/modifier/array_subscript_operator.cpp
 * @author Marcel Breyer
 * @date 2020-02-02
 *
 * @brief Test cases for the @ref mpicxx::info::operator[](detail::string auto&&) member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *ModifierTest*
 * | test case name                       | test case description                            |
 * |:-------------------------------------|:-------------------------------------------------|
 * | ArraySubscriptOperatorRead           | read [key, value]-pairs                          |
 * | ArraySubscriptOperatorWrite          | write [key, value]-pairs                         |
 * | MovedFromArraySubscriptOperator      | info object in the moved-from state (death test) |
 * | ArraySubscriptOperatorWithIllegalKey | try to add an illegal key (death test)           |
 */

#include <string>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ModifierTest, ArraySubscriptOperatorRead) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // read existing value
    std::string value = info["key1"];

    // check if value is correct and nothing was added
    EXPECT_STREQ(value.c_str(), "value1");
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // read non-existing value
    const std::string key2("key2");
    value = info[key2];

    // check if a new empty value has been added
    EXPECT_STREQ(value.c_str(), " ");
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);
}

TEST(ModifierTest, ArraySubscriptOperatorWrite) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // add new elements (various supported variants)
    info[std::string("key2")] = "value2";
    const std::string value3("value3");
    info["key3"] = value3;
    char key4[] = "key4";
    char value4[] = "value4";
    info[key4] = value4;

    // check if all [key, value]-pairs have been added
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
    MPI_Info_get(info.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");
    MPI_Info_get(info.get(), "key4", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value4");

    // override already existing value
    info["key1"] = "value1_override";

    // check that no new [key, value]-pair has been added
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // check if the value has been changed successfully
    MPI_Info_get(info.get(), "key1", 15, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1_override");
}

TEST(ModifierDeathTest, MovedFromArraySubscriptOperator) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // calling operator[]() on an info object in the moved-from state is illegal
    ASSERT_DEATH( info["key"] = "value" , "");
}

TEST(ModifierDeathTest, ArraySubscriptOperatorWithIllegalKey) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');

    // try to add an illegal key
    ASSERT_DEATH( info[key] = "value" , "");
    ASSERT_DEATH( info[""] = "value" , "");
}