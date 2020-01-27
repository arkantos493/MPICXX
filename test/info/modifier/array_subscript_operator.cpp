/**
 * @file info/modifier/array_subscript_operator.cpp
 * @author Marcel Breyer
 * @date 2019-12-18
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the `operator[]` member-function of a mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ModifierTest, AccessOperatorRead) {
    // create info object and add element
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // read existing value
    const std::string value1 = info["key1"];

    // check if value1 is correct and nothing was added
    EXPECT_STREQ(value1.c_str(), "value1");
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // read non-existing value2
    const std::string key2("key2");
    const std::string value2 = info[key2];

    // check if a new empty value2 has been added
    EXPECT_STREQ(value2.c_str(), " ");
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);
}

TEST(ModifierTest, AccessOperatorConstRead) {
    // create const empty info object
    const mpicxx::info info;

    // try reading a (non-existing) value
//    info["key"];        // -> shouldn't compile
}

TEST(ModifierTest, AccessOperatorWrite) {
    // create info object and add element
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // add new elements
    info[std::string("key2")] = "value2";
    const std::string value3("value3");
    info["key3"] = value3;
    char key4[] = "key4";
    char value4[] = "value4";
    info[key4] = value4;

    // check if all [key, value]-pairs has been added
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // override already existing value
    info["key1"] = "value1_override";

    // check that no new [key, value]-pair has been added
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // check if values has been changed successfully
    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info.get(), "key1", 15, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1_override");
}

TEST(ModifierTest, MovedFromAccessOperator) {
    // create info object and set it to the "moved-from" state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // try adding [key, value]-pair to an object in the "moved-from" state
//    info["key"] = "value";       // -> should assert
}

TEST(ModifierTest, AccessOperatorIllegalArguments) {
    // create emtpy info object
    mpicxx::info info;

    // try adding a [key, value]-pair with a key that is too long
    const std::string long_key(MPI_MAX_INFO_KEY, 'x');
//    info[long_key] = "value";       // -> should assert

    // try adding a [key, value]-pair with a value that is too long
    const std::string long_value(MPI_MAX_INFO_VAL, 'x');
//    info["key"] = long_value;       // -> should assert
}