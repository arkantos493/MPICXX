/**
 * @file test/info/assignment/initializer_list_assignment.cpp
 * @author Marcel Breyer
 * @date 2020-04-11
 *
 * @brief Test cases for the @ref mpicxx::info::operator=(const std::initializer_list<value_type>) member function provided by the
 * @ref mpicxx::info class.
 * @details Testsuite: *AssignmentTest*
 * | test case name                         | test case description                                                                     |
 * |:---------------------------------------|:------------------------------------------------------------------------------------------|
 * | AssignInitializerListToValid           | assign all elements of the initializer list to the info object                            |
 * | AssignInitializerListToNull            | assign all elements of the initializer list to the info object referring to MPI_INFO_NULL |
 * | AssignInitializerListToNonFreeable     | assign all elements of the initializer list to the non-freeable info object               |
 * | AssignInitializerListIllegalKeyOrValue | try to assign an illegal key/value to the info object (death test)                        |
 */

#include <string>
#include <utility>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(AssignmentTest, AssignInitializerListToValid) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key", "value");

    // assign initializer_list
    info = { { "key1", "value1" }, { "key2", "value2" } };

    // check if the info object now contains the correct entries
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);

    // old [key, value]-pair should not be present anymore
    MPI_Info_get(info.get(), "key", 5, value, &flag);
    EXPECT_FALSE(static_cast<bool>(flag));

    // ney [key, value]-pairs should be present now
    MPI_Info_get(info.get(), "key1", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");
    MPI_Info_get(info.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
}

TEST(AssignmentTest, AssignInitializerListToNull) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // assign initializer_list
    info = { { "key1", "value1" }, { "key2", "value2" } };

    // info should not refer to MPI_INFO_NULL anymore
    EXPECT_NE(info.get(), MPI_INFO_NULL);

    // check if the info object now contains the correct entries
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);

    // ney [key, value]-pairs should be present now
    MPI_Info_get(info.get(), "key1", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");
    MPI_Info_get(info.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
}

TEST(AssignmentTest, AssignInitializerListToNonFreeable) {
    // create non-freeable info object
    mpicxx::info info(MPI_INFO_ENV, false);

    // assign initializer_list
    info = { { "key1", "value1" }, { "key2", "value2" } };

    // info should be freeable now
    EXPECT_TRUE(info.freeable());

    // check if the info object now contains the correct entries
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);

    // ney [key, value]-pairs should be present now
    MPI_Info_get(info.get(), "key1", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");
    MPI_Info_get(info.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
}

TEST(AssignmentDeathTest, AssignInitializerListIllegalKeyOrValue) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');
    std::string value(MPI_MAX_INFO_VAL, ' ');

    // assign initializer list with illegal key
    ASSERT_DEATH( (info = { { key, "value" } }) , "");
    ASSERT_DEATH( (info = { { "", "value" } }) , "");

    // assign initializer list with illegal value
    ASSERT_DEATH( (info = { { "key", value } }) , "");
    ASSERT_DEATH( (info = { { "key", "" } }) , "");
}