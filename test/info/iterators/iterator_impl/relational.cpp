/**
 * @file test/info/iterators/iterator_impl/relational.cpp
 * @author Marcel Breyer
 * @date 2020-02-11
 *
 * @brief Test cases for the relational operations of the @ref mpicxx::info::iterator and @ref mpicxx::info::const_iterator class.
 * @details Testsuite: *InfoIteratorImplTest*
 * | test case name                      | test case description                                            |
 * |:------------------------------------|:-----------------------------------------------------------------|
 * | EqualityValidComparison             | compare two valid iterators for equality                         |
 * | EqualityInvalidComparison           | compare invalid iterators for equality (death test)              |
 * | InequalityValidComparison           | compare two valid iterators for inequality                       |
 * | InequalityInvalidComparison         | compare invalid iterators for inequality (death test)            |
 * | LessThanValidComparison             | compare two valid iterators for less than                        |
 * | LessThanInvalidComparison           | compare invalid iterators for less than (death test)             |
 * | GreaterThanValidComparison          | compare two valid iterators for greater than                     |
 * | GreaterThanInvalidComparison        | compare invalid iterators for greater than (death test)          |
 * | LessOrEqualThanValidComparison      | compare two valid iterators for or equal less than               |
 * | LessOrEqualThanInvalidComparison    | compare invalid iterators for less or equal than (death test)    |
 * | GreaterOrEqualThanValidComparison   | compare two valid iterators for greater or equal than            |
 * | GreaterOrEqualThanInvalidComparison | compare invalid iterators for greater or equal than (death test) |
 * | CompareConstAndNonConst             | compare const and non-const iterators                            |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(InfoIteratorImplTest, EqualityValidComparison) {
    // create info objects and add [key, value]-pairs
    mpicxx::info info_1;
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key1", "value1");
    MPI_Info_set(info_2.get(), "key2", "value2");

    // empty info object -> begin() and end() should compare equal
    EXPECT_TRUE(info_1.begin() == info_1.begin());
    EXPECT_TRUE(info_1.begin() == info_1.end());
    EXPECT_TRUE(info_1.end() == info_1.begin());

    // non-empty info object -> begin() and end() shouldn't compare equal
    EXPECT_FALSE(info_2.begin() == info_2.end());
    EXPECT_FALSE(info_2.end() == info_2.begin());

    EXPECT_FALSE(info_2.begin() == info_2.begin() + 1);
    EXPECT_FALSE(info_2.begin() + 1 == info_2.begin());
    EXPECT_TRUE(info_2.begin() + 1 == info_2.end() - 1);
    EXPECT_TRUE(info_2.end() - 1 == info_2.begin() + 1);
}

TEST(InfoIteratorImplDeathTest, EqualityInvalidComparison) {
    // create info objects
    mpicxx::info info_1;
    mpicxx::info info_2;

    // create singular iterators
    mpicxx::info::iterator sit_1;
    mpicxx::info::iterator sit_2;

    // create iterators referring to moved-from info objects
    mpicxx::info moved_from_1;
    mpicxx::info moved_from_2;
    mpicxx::info::iterator moved_from_it_1 = moved_from_1.begin();
    mpicxx::info::iterator moved_from_it_2 = moved_from_2.begin();
    mpicxx::info dummy_1(std::move(moved_from_1));
    mpicxx::info dummy_2(std::move(moved_from_2));

    [[maybe_unused]] bool comp;
    // comparisons with singular iterators are not permitted
    EXPECT_DEATH( comp = sit_1 == sit_2 , "");
    EXPECT_DEATH( comp = sit_1 == info_1.begin() , "");
    EXPECT_DEATH( comp = info_1.begin() == sit_1 , "");

    // comparisons with iterators referring to info objects in the moved-from state are not permitted
    EXPECT_DEATH( comp = moved_from_it_1 == moved_from_it_2 , "");
    EXPECT_DEATH( comp = moved_from_it_1 == info_1.begin() , "");
    EXPECT_DEATH( comp = info_1.begin() == moved_from_it_1 , "");

    // comparing iterators from different info objects is not permitted
    EXPECT_DEATH( comp = info_1.begin() == info_2.end() , "");
}


TEST(InfoIteratorImplTest, InequalityValidComparison) {
    // create info objects and add [key, value]-pairs
    mpicxx::info info_1;
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key1", "value1");
    MPI_Info_set(info_2.get(), "key2", "value2");

    // empty info object -> begin() and end() shouldn't compare inequal
    EXPECT_FALSE(info_1.begin() != info_1.begin());
    EXPECT_FALSE(info_1.begin() != info_1.end());
    EXPECT_FALSE(info_1.end() != info_1.begin());

    // non-empty info object -> begin() and end() should compare inequal
    EXPECT_TRUE(info_2.begin() != info_2.end());
    EXPECT_TRUE(info_2.end() != info_2.begin());

    EXPECT_TRUE(info_2.begin() != info_2.begin() + 1);
    EXPECT_TRUE(info_2.begin() + 1 != info_2.begin());
    EXPECT_FALSE(info_2.begin() + 1 != info_2.end() - 1);
    EXPECT_FALSE(info_2.end() - 1 != info_2.begin() + 1);
}

TEST(InfoIteratorImplDeathTest, InequalityInvalidComparison) {
    // create info objects
    mpicxx::info info_1;
    mpicxx::info info_2;

    // create singular iterators
    mpicxx::info::iterator sit_1;
    mpicxx::info::iterator sit_2;

    // create iterators referring to moved-from info objects
    mpicxx::info moved_from_1;
    mpicxx::info moved_from_2;
    mpicxx::info::iterator moved_from_it_1 = moved_from_1.begin();
    mpicxx::info::iterator moved_from_it_2 = moved_from_2.begin();
    mpicxx::info dummy_1(std::move(moved_from_1));
    mpicxx::info dummy_2(std::move(moved_from_2));

    [[maybe_unused]] bool comp;
    // comparisons with singular iterators are not permitted
    EXPECT_DEATH( comp = sit_1 != sit_2 , "");
    EXPECT_DEATH( comp = sit_1 != info_1.begin() , "");
    EXPECT_DEATH( comp = info_1.begin() != sit_1 , "");

    // comparisons with iterators referring to info objects in the moved-from state are not permitted
    EXPECT_DEATH( comp = moved_from_it_1 != moved_from_it_2 , "");
    EXPECT_DEATH( comp = moved_from_it_1 != info_1.begin() , "");
    EXPECT_DEATH( comp = info_1.begin() != moved_from_it_1 , "");

    // comparing iterators from different info objects is not permitted
    EXPECT_DEATH( comp = info_1.begin() != info_2.end() , "");
}


TEST(InfoIteratorImplTest, LessThanValidComparison) {
    // create info objects and add [key, value]-pairs
    mpicxx::info info_1;
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key1", "value1");
    MPI_Info_set(info_2.get(), "key2", "value2");

    // empty info object
    EXPECT_FALSE(info_1.begin() < info_1.begin());
    EXPECT_FALSE(info_1.begin() < info_1.end());
    EXPECT_FALSE(info_1.end() < info_1.begin());

    // non-empty info object
    EXPECT_TRUE(info_2.begin() < info_2.end());
    EXPECT_FALSE(info_2.end() < info_2.begin());

    EXPECT_TRUE(info_2.begin() < info_2.begin() + 1);
    EXPECT_FALSE(info_2.begin() + 1 < info_2.begin());
    EXPECT_FALSE(info_2.begin() + 1 < info_2.end() - 1);
    EXPECT_FALSE(info_2.end() - 1 < info_2.begin() + 1);
}

TEST(InfoIteratorImplDeathTest, LessThanInvalidComparison) {
    // create info objects
    mpicxx::info info_1;
    mpicxx::info info_2;

    // create singular iterators
    mpicxx::info::iterator sit_1;
    mpicxx::info::iterator sit_2;

    // create iterators referring to moved-from info objects
    mpicxx::info moved_from_1;
    mpicxx::info moved_from_2;
    mpicxx::info::iterator moved_from_it_1 = moved_from_1.begin();
    mpicxx::info::iterator moved_from_it_2 = moved_from_2.begin();
    mpicxx::info dummy_1(std::move(moved_from_1));
    mpicxx::info dummy_2(std::move(moved_from_2));

    [[maybe_unused]] bool comp;
    // comparisons with singular iterators are not permitted
    EXPECT_DEATH( comp = sit_1 < sit_2 , "");
    EXPECT_DEATH( comp = sit_1 < info_1.begin() , "");
    EXPECT_DEATH( comp = info_1.begin() < sit_1 , "");

    // comparisons with iterators referring to info objects in the moved-from state are not permitted
    EXPECT_DEATH( comp = moved_from_it_1 < moved_from_it_2 , "");
    EXPECT_DEATH( comp = moved_from_it_1 < info_1.begin() , "");
    EXPECT_DEATH( comp = info_1.begin() < moved_from_it_1 , "");

    // comparing iterators from different info objects is not permitted
    EXPECT_DEATH( comp = info_1.begin() < info_2.end() , "");
}


TEST(InfoIteratorImplTest, GreaterThanValidComparison) {
    // create info objects and add [key, value]-pairs
    mpicxx::info info_1;
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key1", "value1");
    MPI_Info_set(info_2.get(), "key2", "value2");

    // empty info object
    EXPECT_FALSE(info_1.begin() > info_1.begin());
    EXPECT_FALSE(info_1.begin() > info_1.end());
    EXPECT_FALSE(info_1.end() > info_1.begin());

    // non-empty info object
    EXPECT_FALSE(info_2.begin() > info_2.end());
    EXPECT_TRUE(info_2.end() > info_2.begin());

    EXPECT_FALSE(info_2.begin() > info_2.begin() + 1);
    EXPECT_TRUE(info_2.begin() + 1 > info_2.begin());
    EXPECT_FALSE(info_2.begin() + 1 > info_2.end() - 1);
    EXPECT_FALSE(info_2.end() - 1 > info_2.begin() + 1);
}

TEST(InfoIteratorImplDeathTest, GreaterThanInvalidComparison) {
    // create info objects
    mpicxx::info info_1;
    mpicxx::info info_2;

    // create singular iterators
    mpicxx::info::iterator sit_1;
    mpicxx::info::iterator sit_2;

    // create iterators referring to moved-from info objects
    mpicxx::info moved_from_1;
    mpicxx::info moved_from_2;
    mpicxx::info::iterator moved_from_it_1 = moved_from_1.begin();
    mpicxx::info::iterator moved_from_it_2 = moved_from_2.begin();
    mpicxx::info dummy_1(std::move(moved_from_1));
    mpicxx::info dummy_2(std::move(moved_from_2));

    [[maybe_unused]] bool comp;
    // comparisons with singular iterators are not permitted
    EXPECT_DEATH( comp = sit_1 > sit_2 , "");
    EXPECT_DEATH( comp = sit_1 > info_1.begin() , "");
    EXPECT_DEATH( comp = info_1.begin() > sit_1 , "");

    // comparisons with iterators referring to info objects in the moved-from state are not permitted
    EXPECT_DEATH( comp = moved_from_it_1 > moved_from_it_2 , "");
    EXPECT_DEATH( comp = moved_from_it_1 > info_1.begin() , "");
    EXPECT_DEATH( comp = info_1.begin() > moved_from_it_1 , "");

    // comparing iterators from different info objects is not permitted
    EXPECT_DEATH( comp = info_1.begin() > info_2.end() , "");
}


TEST(InfoIteratorImplTest, LessOrEqualThanValidComparison) {
    // create info objects and add [key, value]-pairs
    mpicxx::info info_1;
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key1", "value1");
    MPI_Info_set(info_2.get(), "key2", "value2");

    // empty info object
    EXPECT_TRUE(info_1.begin() <= info_1.begin());
    EXPECT_TRUE(info_1.begin() <= info_1.end());
    EXPECT_TRUE(info_1.end() <= info_1.begin());

    // non-empty info object
    EXPECT_TRUE(info_2.begin() <= info_2.end());
    EXPECT_FALSE(info_2.end() <= info_2.begin());

    EXPECT_TRUE(info_2.begin() <= info_2.begin() + 1);
    EXPECT_FALSE(info_2.begin() + 1 <= info_2.begin());
    EXPECT_TRUE(info_2.begin() + 1 <= info_2.end() - 1);
    EXPECT_TRUE(info_2.end() - 1 <= info_2.begin() + 1);
}

TEST(InfoIteratorImplDeathTest, LessOrEqualThanInvalidComparison) {
    // create info objects
    mpicxx::info info_1;
    mpicxx::info info_2;

    // create singular iterators
    mpicxx::info::iterator sit_1;
    mpicxx::info::iterator sit_2;

    // create iterators referring to moved-from info objects
    mpicxx::info moved_from_1;
    mpicxx::info moved_from_2;
    mpicxx::info::iterator moved_from_it_1 = moved_from_1.begin();
    mpicxx::info::iterator moved_from_it_2 = moved_from_2.begin();
    mpicxx::info dummy_1(std::move(moved_from_1));
    mpicxx::info dummy_2(std::move(moved_from_2));

    [[maybe_unused]] bool comp;
    // comparisons with singular iterators are not permitted
    EXPECT_DEATH( comp = sit_1 <= sit_2 , "");
    EXPECT_DEATH( comp = sit_1 <= info_1.begin() , "");
    EXPECT_DEATH( comp = info_1.begin() <= sit_1 , "");

    // comparisons with iterators referring to info objects in the moved-from state are not permitted
    EXPECT_DEATH( comp = moved_from_it_1 <= moved_from_it_2 , "");
    EXPECT_DEATH( comp = moved_from_it_1 <= info_1.begin() , "");
    EXPECT_DEATH( comp = info_1.begin() <= moved_from_it_1 , "");

    // comparing iterators from different info objects is not permitted
    EXPECT_DEATH( comp = info_1.begin() <= info_2.end() , "");
}


TEST(InfoIteratorImplTest, GreaterOrEqualThanValidComparison) {
    // create info objects and add [key, value]-pairs
    mpicxx::info info_1;
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key1", "value1");
    MPI_Info_set(info_2.get(), "key2", "value2");

    // empty info object
    EXPECT_TRUE(info_1.begin() >= info_1.begin());
    EXPECT_TRUE(info_1.begin() >= info_1.end());
    EXPECT_TRUE(info_1.end() >= info_1.begin());

    // non-empty info object
    EXPECT_FALSE(info_2.begin() >= info_2.end());
    EXPECT_TRUE(info_2.end() >= info_2.begin());

    EXPECT_FALSE(info_2.begin() >= info_2.begin() + 1);
    EXPECT_TRUE(info_2.begin() + 1 >= info_2.begin());
    EXPECT_TRUE(info_2.begin() + 1 >= info_2.end() - 1);
    EXPECT_TRUE(info_2.end() - 1 >= info_2.begin() + 1);
}

TEST(InfoIteratorImplDeathTest, GreaterOrEqualThanInvalidComparison) {
    // create info objects
    mpicxx::info info_1;
    mpicxx::info info_2;

    // create singular iterators
    mpicxx::info::iterator sit_1;
    mpicxx::info::iterator sit_2;

    // create iterators referring to moved-from info objects
    mpicxx::info moved_from_1;
    mpicxx::info moved_from_2;
    mpicxx::info::iterator moved_from_it_1 = moved_from_1.begin();
    mpicxx::info::iterator moved_from_it_2 = moved_from_2.begin();
    mpicxx::info dummy_1(std::move(moved_from_1));
    mpicxx::info dummy_2(std::move(moved_from_2));

    [[maybe_unused]] bool comp;
    // comparisons with singular iterators are not permitted
    EXPECT_DEATH( comp = sit_1 >= sit_2 , "");
    EXPECT_DEATH( comp = sit_1 >= info_1.begin() , "");
    EXPECT_DEATH( comp = info_1.begin() >= sit_1 , "");

    // comparisons with iterators referring to info objects in the moved-from state are not permitted
    EXPECT_DEATH( comp = moved_from_it_1 >= moved_from_it_2 , "");
    EXPECT_DEATH( comp = moved_from_it_1 >= info_1.begin() , "");
    EXPECT_DEATH( comp = info_1.begin() >= moved_from_it_1 , "");

    // comparing iterators from different info objects is not permitted
    EXPECT_DEATH( comp = info_1.begin() >= info_2.end() , "");
}


TEST(InfoIteratorImplTest,CompareConstAndNonConst) {
    // create empty info object
    mpicxx::info info;

    // create const and non const iterators
    mpicxx::info::iterator it = info.begin();
    mpicxx::info::const_iterator const_it = info.cend();

    // perform comparisons
    EXPECT_TRUE(it == it);
    EXPECT_TRUE(it == const_it);
    EXPECT_TRUE(const_it == it);
    EXPECT_TRUE(const_it == const_it);

    EXPECT_FALSE(it != it);
    EXPECT_FALSE(it != const_it);
    EXPECT_FALSE(const_it != it);
    EXPECT_FALSE(const_it != const_it);

    EXPECT_FALSE(it < it);
    EXPECT_FALSE(it < const_it);
    EXPECT_FALSE(const_it < it);
    EXPECT_FALSE(const_it < const_it);

    EXPECT_FALSE(it > it);
    EXPECT_FALSE(it > const_it);
    EXPECT_FALSE(const_it > it);
    EXPECT_FALSE(const_it > const_it);

    EXPECT_TRUE(it <= it);
    EXPECT_TRUE(it <= const_it);
    EXPECT_TRUE(const_it <= it);
    EXPECT_TRUE(const_it <= const_it);

    EXPECT_TRUE(it >= it);
    EXPECT_TRUE(it >= const_it);
    EXPECT_TRUE(const_it >= it);
    EXPECT_TRUE(const_it >= const_it);
}