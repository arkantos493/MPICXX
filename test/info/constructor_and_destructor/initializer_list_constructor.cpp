/**
 * @file test/info/constructor_and_destructor/initializer_list_constructor.cpp
 * @author Marcel Breyer
 * @date 2020-04-10
 *
 * @brief Test cases for the @ref mpicxx::info::info(std::initializer_list<value_type>) member function provided by the
 * @ref mpicxx::info class.
 * @details Testsuite: *ConstructionTest*
 * | test case name                   | test case description                                                                                                    |
 * |:---------------------------------|:-------------------------------------------------------------------------------------------------------------------------|
 * | InitializerListConstruction      | construct info object from a [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list)       |
 * | EmptyInitializerListConstruction | construct empty info object from a [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) |
 * | InitializerListIllegalKeyOrValue | try to construct info object from an illegal key/value (death test)                                                      |
 */

#include <string>
#include <utility>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ConstructionTest, InitializerListConstruction) {
    // construct a info object using a std::initializer_list
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key1", "value1_override"},
                          {"key3", "value3"} };

    // info object should now contain three entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 3);

    // check if all [key, value]-pairs were added
    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info.get(), "key1", 15, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1_override");

    MPI_Info_get(info.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");

    MPI_Info_get(info.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");

    // an info object constructed from an initializer_list is always freeable
    EXPECT_TRUE(info.freeable());
}

TEST(ConstructionTest, EmptyInitializerListConstruction) {
    // construct a info object using a std::initializer_list
    mpicxx::info info = { };

    // info object should be empty
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);

    // an info object constructed from an initializer_list is always freeable
    EXPECT_TRUE(info.freeable());
}

TEST(ConstructionDeathTest, InitializerListIllegalKeyOrValue) {
    std::string key(MPI_MAX_INFO_KEY, ' ');
    std::string value(MPI_MAX_INFO_VAL, ' ');

    // create info object from initializer list with illegal key
    ASSERT_DEATH( mpicxx::info info({ { key, "value" } }) , "");
    ASSERT_DEATH( mpicxx::info info({ { "", "value" } }) , "");

    // create info object from initializer list with illegal value
    ASSERT_DEATH( mpicxx::info info({ { "key", value } }) , "");
    ASSERT_DEATH( mpicxx::info info({ { "key", "" } }) , "");
}