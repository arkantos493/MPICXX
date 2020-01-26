/**
 * @file at.cpp
 * @author Marcel Breyer
 * @date 2020-01-26
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the `at` member-function of a mpicxx::info class.
 */

#include <stdexcept>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ModifierTest, AtRead) {
    // create info object and add element
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // read existing value
    const std::string value1 = info.at("key1");

    // check if value1 is correct and nothing was added
    EXPECT_STREQ(value1.c_str(), "value1");
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
}

TEST(ModifierTest, AtConstRead) {
    // create info object and add element
    const mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // read existing value
    const std::string value1 = info.at("key1");

    // check if value1 is correct and nothing was added
    EXPECT_STREQ(value1.c_str(), "value1");
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
}

TEST(ModifierTest, AtWrite) {
    // create info object and add element
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // override already existing value
    info["key1"] = "value1_override";

    // check that no new [key, value]-pair has been added
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // check if values has been changed successfully
    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info.get(), "key1", 15, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1_override");
}

TEST(ModifierTest, MovedFromAt) {
    // create emtpy info and const info objects and set them to the "moved-from" state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // try adding [key, value]-pair to an object in the "moved-from" state
//    info.at("key");       // -> should assert
}

TEST(ModifierTest, AtOutOfRangeException) {
    // create emtpy info and const info objects
    mpicxx::info info;
    try {
        info.at("key");
        FAIL() << "expected std::out_of_range exception";
    } catch(const std::out_of_range& e) {
        EXPECT_STREQ(e.what(), "key doesn't exist!");
    } catch(...) {
        FAIL() << "expected std::out_of_range exception";
    }

    const mpicxx::info const_info;
    try {
        const_info.at("key_2");
        FAIL() << "expected std::out_of_range exception";
    } catch(const std::out_of_range& e) {
        EXPECT_STREQ(e.what(), "key_2 doesn't exist!");
    } catch(...) {
        FAIL() << "expected std::out_of_range exception";
    }
}

TEST(ModifierTest, AtIllegalArguments) {
    // create emtpy info and const info objects
    mpicxx::info info;
    const mpicxx::info const_info;

    // try adding a [key, value]-pair with a key that is too long
    const std::string long_key(MPI_MAX_INFO_KEY, 'x');
//    info.at(long_key) = "value";       // -> should assert
//    const_info.at(long_key);           // -> should assert
}