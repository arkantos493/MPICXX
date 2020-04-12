/**
 * @file test/info/assignment/move_assignment.cpp
 * @author Marcel Breyer
 * @date 2020-04-12
 *
 * @brief Test cases for the @ref mpicxx::info::operator=(info&&) member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *AssignmentTest*
 * | test case name            | test case description                                                         |
 * |:--------------------------|:------------------------------------------------------------------------------|
 * | MoveAssignValidToValid    | `info1 = std::move(info2);`                                                   |
 * | MoveAssignNullToValid     | `info1 = std::move(info2); // where info2 refers to MPI_INFO_NULL´            |
 * | MoveAssignValidToNull     | `info1 = std::move(info2); // where info1 refers to MPI_INFO_NULL´            |
 * | MoveAssignNullToNull      | `info1 = std::move(info2); // where info1 and info2 refer to MPI_INFO_NULL´   |
 * | MoveSelfAssignment        | `info1 = std::move(info1); // no-op` (death test)                             |
 * | MoveAssignToNonFreeable   | non-freeable info object should be freeable now                               |
 * | MoveAssignFromNonFreeable | info object should be non-freeable (because the copied-from was non-freeable) |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(AssignmentTest, MoveAssignValidToValid) {
    // create first info object
    mpicxx::info valid_1;
    MPI_Info_set(valid_1.get(), "key1", "value1");
    // create second info object
    mpicxx::info valid_2;
    MPI_Info_set(valid_2.get(), "key2", "value2");

    // perform move assignment
    valid_1 = std::move(valid_2);

    // info_2 should now be in the moved-from state (referring to MPI_INFO_NNULL)
    EXPECT_EQ(valid_2.get(), MPI_INFO_NULL);
    EXPECT_FALSE(valid_2.freeable());

    // info_1 should be in a valid state containing ["key2", "value2"]
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(valid_1.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(valid_1.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
}

TEST(AssignmentTest, MoveAssignNullToValid) {
    // create info object and null info object
    mpicxx::info info_null(MPI_INFO_NULL, false);
    mpicxx::info valid;

    // perform move assignment
    valid = std::move(info_null);

    // info_null should now be in the moved-from state (referring to MPI_INFO_NULL)
    EXPECT_EQ(info_null.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_null.freeable());

    // valid should refer to MPI_INFO_NULL
    EXPECT_EQ(valid.get(), MPI_INFO_NULL);
    EXPECT_FALSE(valid.freeable());
}

TEST(AssignmentTest, MoveAssignValidToNull) {
    // create null info object
    mpicxx::info info_null(MPI_INFO_NULL, false);
    // create second info object
    mpicxx::info valid;
    MPI_Info_set(valid.get(), "key", "value");

    // perform move assignment
    info_null = std::move(valid);

    // valid should now be in the moved-from state (referring to MPI_INFO_NULL)
    EXPECT_EQ(valid.get(), MPI_INFO_NULL);
    EXPECT_FALSE(valid.freeable());

    // info_null should now be in a valid state containing only ["key2", "value2"]
    ASSERT_NE(info_null.get(), MPI_INFO_NULL);
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(info_null.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(info_null.get(), "key", 5, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value");
}

TEST(AssignmentTest, MoveAssignNullToNull) {
    // create two null info objects
    mpicxx::info info_null_1(MPI_INFO_NULL, false);
    mpicxx::info info_null_2(MPI_INFO_NULL, false);

    // perform move assignment
    info_null_1 = std::move(info_null_2);

    // info_null_1 and info_null_2 should refer to MPI_INFO_NULL
    EXPECT_EQ(info_null_1.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_null_1.freeable());
    EXPECT_EQ(info_null_2.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_null_2.freeable());
}

TEST(AssignmentDeathTest, MoveSelfAssignment) {
    // create info object
    mpicxx::info info;

    // perform self assignment
    EXPECT_DEATH( info = std::move(info), "");
}

TEST(AssignmentTest, MoveAssignToNonFreeable) {
    // create empty info object
    mpicxx::info info;
    // create non-freeable info object
    mpicxx::info non_freeable(MPI_INFO_ENV, false);

    // perform move assignment
    non_freeable = std::move(info);

    // non_freeable should now be freeable and empty
    int nkeys;
    MPI_Info_get_nkeys(non_freeable.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
    EXPECT_TRUE(non_freeable.freeable());

    // info should be in the moved-from state
    EXPECT_EQ(info.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info.freeable());

    // -> if non_freeable would have been freed, the MPI runtime would crash
}

TEST(AssignmentTest, MoveAssignFromNonFreeable) {
    // create info objects and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info mpi_info;
    MPI_Info_create(&mpi_info);
    MPI_Info_set(mpi_info, "key", "value");
    // create non-freeable info object
    mpicxx::info non_freeable(mpi_info, false);

    // perform move assignment
    info = std::move(non_freeable);

    // info shouldn't be empty anymore and marked as non-freeable (as non_freeable was)
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    EXPECT_FALSE(info.freeable());

    // non_freeable should be in the moved-from state (referring to MPI_INFO_NULL)
    EXPECT_EQ(non_freeable.get(), MPI_INFO_NULL);
    EXPECT_FALSE(non_freeable.freeable());

    // -> if info would have been freed, the MPI runtime would crash
    MPI_Info_free(&mpi_info);
}