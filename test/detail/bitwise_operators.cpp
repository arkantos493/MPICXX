/**
 * @file
 * @author Marcel Breyer
 * @date 2020-09-01
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the bitwise logic operators defined be the `MPICXX_DEFINE_ENUM_BITWISE_OPERATORS` macro.
 * @details Testsuite: *DetailTest*
 * | test case name     | test case description                                                                 |
 * |:-------------------|:--------------------------------------------------------------------------------------|
 * | BitwiseNot         | check the via the `MPICXX_DEFINE_ENUM_BITWISE_OPERATORS` macro generated `operator~`  |
 * | BitwiseOr          | check the via the `MPICXX_DEFINE_ENUM_BITWISE_OPERATORS` macro generated `operator|`  |
 * | BitwiseCompoundOr  | check the via the `MPICXX_DEFINE_ENUM_BITWISE_OPERATORS` macro generated `operator|=` |
 * | BitwiseAnd         | check the via the `MPICXX_DEFINE_ENUM_BITWISE_OPERATORS` macro generated `operator&`  |
 * | BitwiseCompoundAnd | check the via the `MPICXX_DEFINE_ENUM_BITWISE_OPERATORS` macro generated `operator&=` |
 * | BitwiseXor         | check the via the `MPICXX_DEFINE_ENUM_BITWISE_OPERATORS` macro generated `operator^`  |
 * | BitwiseCompoundXor | check the via the `MPICXX_DEFINE_ENUM_BITWISE_OPERATORS` macro generated `operator^=` |
 */

#include <mpicxx/detail/bitmask.hpp>

#include <gtest/gtest.h>

// create enum class
enum class test : std::uint32_t {
    one   = 1 << 0,
    two   = 1 << 1,
    three = 1 << 2,
};

// define bitwise operators
MPICXX_DEFINE_ENUM_BITWISE_OPERATORS(test)

TEST(DetailTest, BitwiseNot) {
    // test bitwise not operator
    test t1 = ~test::one;
    EXPECT_EQ(static_cast<std::uint32_t>(t1), ~1);
    test t2 = ~test::two;
    EXPECT_EQ(static_cast<std::uint32_t>(t2), ~2);
    test t3 = ~test::three;
    EXPECT_EQ(static_cast<std::uint32_t>(t3), ~4);
}


TEST(DetailTest, BitwiseOr) {
    // test bitwise or operator
    test t1 = test::one | test::one;
    EXPECT_EQ(static_cast<std::uint32_t>(t1), 1);
    test t2 = test::one | test::three;
    EXPECT_EQ(static_cast<std::uint32_t>(t2), 5);
    test t3 = test::one | test::two | test::three;
    EXPECT_EQ(static_cast<std::uint32_t>(t3), 7);
}

TEST(DetailTest, BitwiseCompoundOr) {
    // test bitwise compound or operator
    test t1 = test::one;
    t1 |= test::one;
    EXPECT_EQ(static_cast<std::uint32_t>(t1), 1);
    test t2 = test::one;
    t2 |= test::three;
    EXPECT_EQ(static_cast<std::uint32_t>(t2), 5);
    test t3 = test::one;
    t3 |= test::two | test::three;
    EXPECT_EQ(static_cast<std::uint32_t>(t3), 7);
}


TEST(DetailTest, BitwiseAnd) {
    // test bitwise and operator
    test t1 = test::one & test::one;
    EXPECT_EQ(static_cast<std::uint32_t>(t1), 1);
    test t2 = test::one & test::three;
    EXPECT_EQ(static_cast<std::uint32_t>(t2), 0);
    test t3 = (test::one | test::two) & test::two;
    EXPECT_EQ(static_cast<std::uint32_t>(t3), 2);
}

TEST(DetailTest, BitwiseCompoundAnd) {
    // test bitwise and operator
    test t1 = test::one;
    t1 &= test::one;
    EXPECT_EQ(static_cast<std::uint32_t>(t1), 1);
    test t2 = test::one;
    t2 &= test::three;
    EXPECT_EQ(static_cast<std::uint32_t>(t2), 0);
    test t3 = test::one | test::two;
    t3 &= test::two;
    EXPECT_EQ(static_cast<std::uint32_t>(t3), 2);
}


TEST(DetailTest, BitwiseXor) {
    // test bitwise xor operator
    test t1 = test::one ^ test::one;
    EXPECT_EQ(static_cast<std::uint32_t>(t1), 0);
    test t2 = test::one ^ test::three;
    EXPECT_EQ(static_cast<std::uint32_t>(t2), 5);
    test t3 = test::one ^ test::two ^ test::three;
    EXPECT_EQ(static_cast<std::uint32_t>(t3), 7);
}

TEST(DetailTest, BitwiseCompoundXor) {
    // test bitwise compound xor operator
    test t1 = test::one;
    t1 ^= test::one;
    EXPECT_EQ(static_cast<std::uint32_t>(t1), 0);
    test t2 = test::one;
    t2 ^= test::three;
    EXPECT_EQ(static_cast<std::uint32_t>(t2), 5);
    test t3 = test::one;
    t3 ^= test::two ^ test::three;
    EXPECT_EQ(static_cast<std::uint32_t>(t3), 7);
}
