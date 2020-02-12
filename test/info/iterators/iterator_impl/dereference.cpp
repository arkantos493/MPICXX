/**
 * @file info/iterators/iterator_impl/dereference.cpp
 * @author Marcel Breyer
 * @date 2020-02-12
 *
 * @brief Test cases for the dereference operations of the @ref mpicxx::info::iterator and @ref mpicxx::info::const_iterator class.
 * @details Testsuite: *IteratorImplTest*
 * | test case name        | test case description                                                                    |
 * |:----------------------|:-----------------------------------------------------------------------------------------|
 * | DereferenceValid      | dereference valid iterator via `operator[]`, `operator*` and `operator->`                |
 * | ConstDereferenceValid | dereference valid const_iterator via `operator[]`, `operator*` and `operator->`          |
 * | DereferenceInvalid    | dereference invalid iterator via `operator[]`, `operator*` and `operator->` (death test) |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(IteratorImplTest, DereferenceValid) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // using operator[]
    {
        // check if the retrieved [key, value]-pair is correct and can be changed
        mpicxx::info::iterator it = info.begin();
        auto key_value_pair = it[1];
        EXPECT_STREQ(key_value_pair.first.c_str(), "key2");
        EXPECT_STREQ(static_cast<std::string>(key_value_pair.second).c_str(), "value2");
        key_value_pair.second = "value2_override";
        EXPECT_STREQ(static_cast<std::string>(key_value_pair.second).c_str(), "value2_override");

        // check if the internal value changed
        char value[MPI_MAX_INFO_VAL];
        int flag;
        MPI_Info_get(info.get(), "key2", 15, value, &flag);
        EXPECT_TRUE(static_cast<bool>(flag));
        EXPECT_STREQ(value, "value2_override");
    }
    // using operator*
    {
        // check if the retrieved [key, value]-pair is correct and can be changed
        mpicxx::info::iterator it = info.begin();
        auto key_value_pair = *it;
        EXPECT_STREQ(key_value_pair.first.c_str(), "key1");
        EXPECT_STREQ(static_cast<std::string>(key_value_pair.second).c_str(), "value1");
        key_value_pair.second = "value1_override";
        EXPECT_STREQ(static_cast<std::string>(key_value_pair.second).c_str(), "value1_override");

        // check if the internal value changed
        char value[MPI_MAX_INFO_VAL];
        int flag;
        MPI_Info_get(info.get(), "key1", 15, value, &flag);
        EXPECT_TRUE(static_cast<bool>(flag));
        EXPECT_STREQ(value, "value1_override");
    }
    // using operator->
    {
        // check if the retrieved [key, value]-pair is correct and can be changed
        mpicxx::info::iterator it = info.begin();
        EXPECT_STREQ(it->first.c_str(), "key1");
        EXPECT_STREQ(static_cast<std::string>(it->second).c_str(), "value1_override");
        it->second = "value1";
        EXPECT_STREQ(static_cast<std::string>(it->second).c_str(), "value1");

        // check if the internal value changed
        char value[MPI_MAX_INFO_VAL];
        int flag;
        MPI_Info_get(info.get(), "key1", 15, value, &flag);
        EXPECT_TRUE(static_cast<bool>(flag));
        EXPECT_STREQ(value, "value1");
    }
}

TEST(IteratorImplTest, ConstDereferenceValid) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // using operator[]
    {
        // check if the retrieved [key, value]-pair is correct
        mpicxx::info::const_iterator it = info.cbegin();
        auto key_value_pair = it[1];
        EXPECT_STREQ(key_value_pair.first.c_str(), "key2");
        EXPECT_STREQ(key_value_pair.second.c_str(), "value2");
    }
    // using operator*
    {
        // check if the retrieved [key, value]-pair is correct
        mpicxx::info::const_iterator it = info.cbegin();
        auto key_value_pair = *it;
        EXPECT_STREQ(key_value_pair.first.c_str(), "key1");
        EXPECT_STREQ(key_value_pair.second.c_str(), "value1");
    }
    // using operator->
    {
        // check if the retrieved [key, value]-pair is correct
        mpicxx::info::const_iterator it = info.cbegin();
        EXPECT_STREQ(it->first.c_str(), "key1");
        EXPECT_STREQ(it->second.c_str(), "value1");
    }
}

TEST(IteratorImplDeathTest, DereferenceInvalid) {
    // create info object and add [key, value]-pairs
    mpicxx::info moved_from;
    mpicxx::info::iterator moved_from_it = moved_from.begin();
    mpicxx::info info(std::move(moved_from));
    MPI_Info_set(info.get(), "key", "value");
    mpicxx::info::iterator it = info.begin();
    mpicxx::info::iterator sit;

    // dereference using operator[]
    EXPECT_DEATH(sit[0], "");
    EXPECT_DEATH(moved_from_it[0], "");
    EXPECT_DEATH(it[-1], "");
    EXPECT_DEATH(it[1], "");

    // dereference using operator*
    EXPECT_DEATH(*sit, "");
    EXPECT_DEATH(*moved_from_it, "");
    EXPECT_DEATH(*(it - 1), "");
    EXPECT_DEATH(*(it + 1), "");

    // dereference using operator->
    EXPECT_DEATH(sit->first, "");
    EXPECT_DEATH(moved_from_it->first, "");
    EXPECT_DEATH((it - 2)->first, "");
    EXPECT_DEATH((it + 2)->first, "");
}