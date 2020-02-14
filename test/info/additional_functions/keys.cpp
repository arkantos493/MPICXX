/**
 * @file test/info/additional_functions/keys.cpp
 * @author Marcel Breyer
 * @date 2020-02-14
 *
 * @brief Test cases for the @ref mpicxx::info::keys() const member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *NonMemberFunctionTest*
 * | test case name | test case description                            |
 * |:---------------|:-------------------------------------------------|
 * | NoKeys         | empty info object                                |
 * | Keys           | info object with [key, value]-pairs              |
 * | MovedFromKeys  | info object in the moved-from state (death test) |
 */

#include <vector>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(NonMemberFunctionTest, NoKeys) {
    // create empty info object
    mpicxx::info info;

    // vector of keys should be empty
    std::vector<std::string> keys = info.keys();
    EXPECT_TRUE(keys.empty());
}

TEST(NonMemberFunctionTest, Keys) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");
    MPI_Info_set(info.get(), "key3", "value3");
    MPI_Info_set(info.get(), "key4", "value4");

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // create vector containing all keys (to compare against)
    std::vector<std::string> correct_keys = { "key1", "key2", "key3", "key4" };

    // get all keys in the info object
    std::vector<std::string> keys = info.keys();

    // compare keys
    ASSERT_EQ(keys.size(), correct_keys.size());
    for (std::size_t i = 0; i < keys.size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(keys[i], correct_keys[i]);
    }
}

TEST(NonMemberFunctionDeathTest, MovedFromKeys) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // calling key() on an info object in the moved-from state is illegal
    ASSERT_DEATH( info.keys() , "");
}