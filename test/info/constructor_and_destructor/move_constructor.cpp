/**
 * @file info/constructor_and_destructor/move_constructor.cpp
 * @author Marcel Breyer
 * @date 2020-04-11
 *
 * @brief Test cases for the @ref mpicxx::info::info(info&&) member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *ConstructionTest*
 * | test case name                   | test case description                                                          |
 * |:---------------------------------|:-------------------------------------------------------------------------------|
 * | MoveConstructFromValidObject     | `mpicxx::info info1(std::move(info2));`                                        |
 * | MoveConstructFromNullObject      | `mpicxx::info info1(std::move(info2)); // where info2 refers to MPI_INFO_NULL` |
 * | MoveConstructFromNonFreeable     | info object should be non-freeable (because the copied-from was non-freeable)  |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ConstructionTest, MoveConstructFromValidObject) {
    // create info object
    mpicxx::info moved_from;
    MPI_Info_set(moved_from.get(), "key", "value");

    // save the freeable state of info
    const bool is_freeable = moved_from.freeable();

    // create an new info object by invoking the move constructor
    mpicxx::info info_move(std::move(moved_from));

    // check if the info_move object has exactly one element too
    int nkeys;
    MPI_Info_get_nkeys(info_move.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // be sure that the moved key and value are present
    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info_move.get(), "key", 5, value, &flag);
    // check if the key exists
    EXPECT_TRUE(static_cast<bool>(flag));
    // check if the value associated with the key is correct
    EXPECT_STREQ(value, "value");

    // be sure that info_moved has the same freeable state as the moved-from object
    EXPECT_EQ(info_move.freeable(), is_freeable);

    // be sure moved_from object has released it's resources and is now in the default-initialized state
    MPI_Info_get_nkeys(moved_from.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
    EXPECT_TRUE(moved_from.freeable());
}

TEST(ConstructionTest, MoveConstructFromNullObject) {
    // create null info object
    mpicxx::info info_null(MPI_INFO_NULL, false);

    // create new info object via the move constructor
    mpicxx::info valid(std::move(info_null));

    // info_null should be in the default-initialized state
    int nkeys;
    MPI_Info_get_nkeys(info_null.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
    EXPECT_TRUE(info_null.freeable());

    // valid should refer to MPI_INFO_NULL
    EXPECT_EQ(valid.get(), MPI_INFO_NULL);
    EXPECT_FALSE(valid.freeable());
}

TEST(ConstructionTest, MoveConstructFromNonFreeable) {
    MPI_Info mpi_info;
    MPI_Info_create(&mpi_info);
    MPI_Info_set(mpi_info, "key", "value");
    // create non-freeable info object
    mpicxx::info non_freeable(mpi_info, false);

    // create an new info object by invoking the move constructor
    mpicxx::info info(std::move(non_freeable));

    // info shouldn't be empty anymore and marked as non-freeable (as non_freeable was)
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    EXPECT_FALSE(info.freeable());

    // non_freeable should be in the default-initialized state
    MPI_Info_get_nkeys(non_freeable.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
    EXPECT_TRUE(non_freeable.freeable());

    // -> if info would have been freed, the MPI runtime would crash
    MPI_Info_free(&mpi_info);
}