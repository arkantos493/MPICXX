/**
 * @file info/constructor_and_destructor/move_constructor.cpp
 * @author Marcel Breyer
 * @date 2020-01-27
 *
 * @brief Test cases for the @ref mpicxx::info::info(info&&) member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *ConstructionTest*
 * | test case name                   | test case description                                                         |
 * |:---------------------------------|:------------------------------------------------------------------------------|
 * | MoveConstructFromValidObject     | `mpicxx::info info1(info2);`                                                  |
 * | MoveConstructFromMovedFromObject | `mpicxx::info info1(info2); // where info2 is in the moved-from state`        |
 * | MoveConstructFromNonFreeable     | info object should be non-freeable (because the copied-from was non-freeable) |
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

    // be sure moved_from object has released it's resources and is now in the moved-from state
    EXPECT_EQ(moved_from.get(), MPI_INFO_NULL);
}

TEST(ConstructionTest, MoveConstructFromMovedFromObject) {
    // create info object and set it to the moved-from state
    mpicxx::info moved_from;
    mpicxx::info dummy(std::move(moved_from));

    // create an new info object by invoking the move constructor
    mpicxx::info info_move(std::move(moved_from));

    // check if info_move is in the moved-from state
    EXPECT_EQ(info_move.get(), MPI_INFO_NULL);

    // be sure moved_from is still in the moved-from state
    EXPECT_EQ(moved_from.get(), MPI_INFO_NULL);
}

TEST(ConstructionTest, MoveConstructFromNonFreeable) {
    // create non-freeable info object
    mpicxx::info non_freeable(MPI_INFO_ENV, false);

    // create an new info object by invoking the move constructor
    mpicxx::info info(std::move(non_freeable));

    // info shouldn't be empty anymore and marked as non-freeable (as non_freeable was)
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_NE(nkeys, 0);
    EXPECT_FALSE(info.freeable());

    // non_freeable should be in the moved-from state
    EXPECT_EQ(non_freeable.get(), MPI_INFO_NULL);

    // -> if info would have been freed, the MPI runtime would crash
}