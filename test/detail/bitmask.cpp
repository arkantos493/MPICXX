/**
 * @file
 * @author Marcel Breyer
 * @date 2020-09-02
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the bitmask namespace.
 * @details Testsuite: *DetailTest*
 * | test case name | test case description                                            |
 * |:---------------|:-----------------------------------------------------------------|
 * | BitmaskTest    | check whether (a) specific bit/bits in the enum class is/are set |
 * | BitmaskNone    | check whether no bits in the enum class are set                  |
 * | BitmaskAny     | check whether any bit in the enum class is set                   |
 * | BitmaskAll     | check whether all bits in the enum class are set                 |
 * | BitmaskCount   | get the number of set bits in the enum class                     |
 * | BitmaskSet     | set (a) specific bit/bits in the enum class                      |
 * | BitmaskReset   | reset (a) specific bit/bits in the enum class                    |
 * | BitmaskFlip    | flip 8a) specific bit/bits in the enum class                     |
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

TEST(DetailTest, BitmaskTest) {
    // test bitmask test
    EXPECT_TRUE(mpicxx::detail::bitmask::test(test::one, test::one));
    EXPECT_TRUE(mpicxx::detail::bitmask::test(test::one | test::two, test::one));
    EXPECT_FALSE(mpicxx::detail::bitmask::test(test::one, test::two));
    EXPECT_FALSE(mpicxx::detail::bitmask::test(test::one | test::three, test::one | test::two));
}

TEST(DetailTest, BitmaskNone) {
    // test bitmask none
    EXPECT_TRUE(mpicxx::detail::bitmask::none(static_cast<test>(0)));
    EXPECT_FALSE(mpicxx::detail::bitmask::none(test::one));
    EXPECT_FALSE(mpicxx::detail::bitmask::none(test::one | test::two | test::three));
}

TEST(DetailTest, BitmaskAny) {
    // test bitmask any
    EXPECT_FALSE(mpicxx::detail::bitmask::any(static_cast<test>(0)));
    EXPECT_TRUE(mpicxx::detail::bitmask::any(test::one));
    EXPECT_TRUE(mpicxx::detail::bitmask::any(test::one | test::two | test::three));
}

TEST(DetailTest, BitmaskAll) {
    // test bitmask all
    EXPECT_TRUE(mpicxx::detail::bitmask::all(static_cast<test>(~0)));
    EXPECT_FALSE(mpicxx::detail::bitmask::all(test::one));
    EXPECT_FALSE(mpicxx::detail::bitmask::all(test::one | test::two | test::three));
}

TEST(DetailTest, BitmaskCount) {
    // test bitmask count
    EXPECT_EQ(mpicxx::detail::bitmask::count(static_cast<test>(0)), 0);
    EXPECT_EQ(mpicxx::detail::bitmask::count(test::one), 1);
    EXPECT_EQ(mpicxx::detail::bitmask::count(test::two | test::three), 2);
    EXPECT_EQ(mpicxx::detail::bitmask::count(test::one | test::two | test::three), 3);
}

TEST(DetailTest, BitmaskSet) {
    // test bitmask set
    test t1 = test::one;
    mpicxx::detail::bitmask::set(t1);
    EXPECT_EQ(t1, static_cast<test>(~0));

    test t2 = test::one;
    mpicxx::detail::bitmask::set(t2, test::one);
    EXPECT_EQ(t2, test::one);

    test t3 = static_cast<test>(0);
    mpicxx::detail::bitmask::set(t3, test::three);
    EXPECT_EQ(t3, test::three);
    mpicxx::detail::bitmask::set(t3, test::one | test::two);
    EXPECT_EQ(t3, test::one | test::two | test::three);
}

TEST(DetailTest, BitmaskReset) {
    // test bitmask reset
    test t1 = static_cast<test>(~0);
    mpicxx::detail::bitmask::reset(t1);
    EXPECT_EQ(t1, static_cast<test>(0));

    test t2 = test::one;
    mpicxx::detail::bitmask::reset(t2, test::one);
    EXPECT_EQ(t2, static_cast<test>(0));

    test t3 = test::one | test::two | test::three;
    mpicxx::detail::bitmask::reset(t3, test::three);
    EXPECT_EQ(t3, test::one | test::two);
    mpicxx::detail::bitmask::reset(t3, test::one | test::two);
    EXPECT_EQ(t3, static_cast<test>(0));
}

TEST(DetailTest, BitmaskFlip) {
    // test bitmask flip
    test t1 = static_cast<test>(0);
    mpicxx::detail::bitmask::flip(t1);
    EXPECT_EQ(t1, static_cast<test>(~0));
    mpicxx::detail::bitmask::flip(t1);
    EXPECT_EQ(t1, static_cast<test>(0));

    test t2 = test::one;
    mpicxx::detail::bitmask::flip(t2, test::one);
    EXPECT_EQ(t2, static_cast<test>(0));
    mpicxx::detail::bitmask::flip(t2, test::one);
    EXPECT_EQ(t2, test::one);

    test t3 = test::one | test::two | test::three;
    mpicxx::detail::bitmask::flip(t3, test::three);
    EXPECT_EQ(t3, test::one | test::two);
    mpicxx::detail::bitmask::flip(t3, test::one | test::two);
    EXPECT_EQ(t3, static_cast<test>(0));
    mpicxx::detail::bitmask::flip(t3, test::one | test::two);
    EXPECT_EQ(t3, test::one | test::two);
    mpicxx::detail::bitmask::flip(t3, test::three);
    EXPECT_EQ(t3, test::one | test::two | test::three);
}