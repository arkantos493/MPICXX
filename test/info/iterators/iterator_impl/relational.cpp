/**
 * @file info/iterators/iterator_impl/relational.cpp
 * @author Marcel Breyer
 * @date 2020-02-04
 *
 * @brief TODO
 * @details Testsuite: *IteratorImplTest*
 * | test case name          | test case description                                        |
 * |:------------------------|:-------------------------------------------------------------|
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(IteratorImplTest, Relational) {
    // create info object and add [key, value]-pairs
    mpicxx::info::iterator it_1;
    mpicxx::info::iterator it_2;

    mpicxx::info info_1;
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key1", "value1");
    MPI_Info_set(info_2.get(), "key2", "value2");

//    const bool b1 = info_1.begin() == info_2.begin(); // asserts
//    const bool b2 = info_1.begin() == info_1.begin();
//    const bool b3 = it_1 == it_2; // asserts
//    const bool b4 = it_1 == info_2.begin() + 1; // asserts
//    const bool b4 = it_1 == info_1.end(); // asserts
//    const bool b5 = info_2.begin() + 1 == it_1; // asserts


//    ASSERT_TRUE(b3);
//    ++it_1;
//    const bool b4 = it_1 == it_2;
//    ASSERT_FALSE(b4);
}