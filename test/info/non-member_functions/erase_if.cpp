/**
 * @file test/info/non-member_functions/erase_if.cpp
 * @author Marcel Breyer
 * @date 2020-04-10
 *
 * @brief Test cases for the @ref mpicxx::info::erase_if(info&, Pred) function provided by the @ref mpicxx::info class.
 * @details Testsuite: *NonMemberFunctionTest*
 * | test case name | test case description                                  |
 * |:---------------|:-------------------------------------------------------|
 * | EraseIfNone    | erase nothing from the info object                     |
 * | EraseIfSome    | erase some [key, value]-pairs from the info object     |
 * | EraseIfAll     | erase all [key, value]-pairs from the info object      |
 * | NullEraseIf    | info object ireferring to *MPI_INFO_NULL* (death test) |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(NonMemberFunctionTest, EraseIfNone) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");
    MPI_Info_set(info.get(), "key3", "value3");
    MPI_Info_set(info.get(), "key4", "value4");

    // erase nothing
    using std::erase_if;
    erase_if(info, []([[maybe_unused]] const auto& pair) { return false; });

    // info object should not have changed
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);
}

TEST(NonMemberFunctionTest, EraseIfSome) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");
    MPI_Info_set(info.get(), "key3", "value3");
    MPI_Info_set(info.get(), "key4", "value4");

    // erase some [key, value]-pairs
    using std::erase_if;
    erase_if(info, [](const auto& pair) { return pair.first == "key1" || pair.second == "value4"; });

    // info object should now have only two [key, value]-pairs
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);

    MPI_Info_get(info.get(), "key1", 6, value, &flag);
    EXPECT_FALSE(static_cast<bool>(flag));
    MPI_Info_get(info.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
    MPI_Info_get(info.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");
    MPI_Info_get(info.get(), "key4", 6, value, &flag);
    EXPECT_FALSE(static_cast<bool>(flag));
}

TEST(NonMemberFunctionTest, EraseIfAll) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");
    MPI_Info_set(info.get(), "key3", "value3");
    MPI_Info_set(info.get(), "key4", "value4");

    // erase everything
    using std::erase_if;
    erase_if(info, []([[maybe_unused]] const auto& pair) { return true; });

    // info object should be empty now
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
}

TEST(NonMemberFunctionDeathTest, NullEraseIf) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling erase_if() with an info object referring to MPI_INFO_NULL is illegal
    using std::erase_if;
    ASSERT_DEATH( erase_if(info, []([[maybe_unused]] const auto& pair) { return true; }) , "");
}