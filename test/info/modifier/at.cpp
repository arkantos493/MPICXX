/**
 * @file test/info/modifier/at.cpp
 * @author Marcel Breyer
 * @date 2020-04-11
 *
 * @brief Test cases for the @ref mpicxx::info::at(detail::string auto&&) and @ref mpicxx::info::at(const std::string_view) const member
 * functions provided by the @ref mpicxx::info class.
 * @details Testsuite: *ModifierTest*
 * | test case name             | test case description                                         |
 * |:---------------------------|:--------------------------------------------------------------|
 * | AtRead                     | read [key, value]-pairs                                       |
 * | ConstAtRead                | read [key, value]-pairs (const info object)                   |
 * | AtWrite                    | overwrite already existing [key, value]-pair                  |
 * | NullAt                     | info object referring to MPI_INFO_NULL (death test)           |
 * | NullConstAt                | const info object referring to MPI_INFO_NULL (death test)     |
 * | AtOutOfRangeException      | try to access a non-existing key                              |
 * | ConstAtOutOfRangeException | try to access a non-existing key (const info object)          |
 * | AtWithIllegalKey           | try to access an illegal key (death test)                     |
 * | ConstAtWithIllegalKey      | try to access an illegal key (const info object) (death test) |
 */

#include <stdexcept>
#include <string>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ModifierTest, AtRead) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key", "value");

    // read existing value
    const std::string value = info.at("key");

    // check if value is correct and nothing was added
    EXPECT_STREQ(value.c_str(), "value");
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
}

TEST(ModifierTest, ConstAtRead) {
    // create const info object
    const mpicxx::info info;
    MPI_Info_set(info.get(), "key", "value");

    // read existing value
    const std::string value = info.at("key");

    // check if value is correct and nothing was added
    EXPECT_STREQ(value.c_str(), "value");
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
}

TEST(ModifierTest, AtWrite) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key", "value");

    // override already existing value
    info["key"] = "value_override";

    // check that no new [key, value]-pair has been added
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // check if values has been changed successfully
    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info.get(), "key", 14, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value_override");
}

TEST(ModifierDeathTest, NullAt) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling at() on an info object referring to MPI_INFO_NULL is illegal
    ASSERT_DEATH( info.at("key") , "");
}

TEST(ModifierDeathTest, NullConstAt) {
    // create const null info object
    const mpicxx::info info(MPI_INFO_NULL, false);

    // calling at() on an info object referring to MPI_INFO_NULL is illegal
    ASSERT_DEATH( info.at("key") , "");
}

TEST(ModifierTest, AtOutOfRangeException) {
    // create emtpy info
    mpicxx::info info;
    try {
        info.at("key");
        FAIL() << "expected std::out_of_range exception";
    } catch(const std::out_of_range& e) {
        EXPECT_STREQ(e.what(), "key doesn't exist!");
    } catch(...) {
        FAIL() << "expected std::out_of_range exception";
    }
}

TEST(ModifierTest, ConstAtOutOfRangeException) {
    // create const emtpy info
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

TEST(ModifierDeathTest, AtWithIllegalKey) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');

    // try accessing illegal keys
    ASSERT_DEATH( info.at(key) , "");
    ASSERT_DEATH( info.at("") , "");
}

TEST(ModifierDeathTest, ConstAtWithIllegalKey) {
    // create const info object
    const mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');

    // try accessing illegal keys
    ASSERT_DEATH( info.at(key) , "");
    ASSERT_DEATH( info.at("") , "");
}