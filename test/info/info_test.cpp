#include <gtest/gtest.h>

#include <mpicxx/info/info.hpp>
#include <iostream>

TEST(StrCompare, ReduceTest) {
    mpicxx::info inf;

    EXPECT_EQ(inf.get_nkeys(), 0);
}
