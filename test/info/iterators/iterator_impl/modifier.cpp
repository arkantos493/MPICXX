/**
 * @file test/info/iterators/iterator_impl/modifier.cpp
 * @author Marcel Breyer
 * @date 2020-04-11
 *
 * @brief Test cases for the modifying operations of the @ref mpicxx::info::iterator and @ref mpicxx::info::const_iterator class.
 * @details Testsuite: *InfoIteratorImplTest*
 * | test case name       | test case description                                                |
 * |:---------------------|:---------------------------------------------------------------------|
 * | PreIncrementValid    | increment a valid iterator: `++it;`                                  |
 * | PreIncrementInvalid  | increment an invalid iterator: `++it;` (death test)                  |
 * | PostIncrementValid   | increment a valid iterator: `it++;`                                  |
 * | PostIncrementInvalid | increment an invalid iterator: `it++;` (death test)                  |
 * | AdvanceValid         | advance a valid iterator: `it += n; it + n; n + it;`                 |
 * | AdvanceInvalid       | advance an invalid iterator: `it += n; it + n; n + it;` (death test) |
 * | PreDecrementValid    | decrement a valid iterator: `--it;`                                  |
 * | PreDecrementInvalid  | decrement a valid iterator: `--it;` (death test)                     |
 * | PostDecrementValid   | decrement a valid iterator: `it--;`                                  |
 * | PostDecrementInvalid | decrement a valid iterator: `it--;` (death test)                     |
 * | RetreatValid         | retreat a valid iterator: `it -= n; it - n;`                         |
 * | RetreatInvalid       | retreat an invalid iterator: `it -= n; it - n;` (death test)         |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(InfoIteratorImplTest, PreIncrementValid) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // check if pre-increment works on an iterator
    mpicxx::info::iterator it = info.begin();
    ++it;
    EXPECT_TRUE(it == info.begin() + 1);
    EXPECT_TRUE(++it == info.begin() + 2);

    // check if pre-increment works on a const_iterator
    mpicxx::info::const_iterator const_it = info.cbegin();
    ++const_it;
    EXPECT_TRUE(const_it == info.begin() + 1);
    EXPECT_TRUE(++const_it == info.begin() + 2);
}

TEST(InfoIteratorImplDeathTest, PreIncrementInvalid) {
    // create info object
    mpicxx::info info;

    // create singular iterator
    mpicxx::info::iterator sit;

    // create iterator referring to an info object referring to MPI_INFO_NULL
    mpicxx::info info_null;
    mpicxx::info::iterator info_null_it = info_null.begin();
    info_null = mpicxx::info(MPI_INFO_NULL, false);

    // incrementing a singular iterator is not permitted
    EXPECT_DEATH( ++sit , "");

    // incrementing a iterator referring to an info object referring to MPI_INFO_NULL is not permitted
    EXPECT_DEATH( ++info_null_it , "");

    // incrementing a past-the-end iterator is not permitted
    EXPECT_DEATH( ++info.begin() , "");
}


TEST(InfoIteratorImplTest, PostIncrementValid) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // check if post-increment works on an iterator
    mpicxx::info::iterator it = info.begin();
    it++;
    EXPECT_TRUE(it == info.begin() + 1);
    EXPECT_TRUE(it++ == info.begin() + 1);
    EXPECT_TRUE(it == info.begin() + 2);

    // check if post-increment works on a const_iterator
    mpicxx::info::const_iterator const_it = info.cbegin();
    const_it++;
    EXPECT_TRUE(const_it == info.begin() + 1);
    EXPECT_TRUE(const_it++ == info.begin() + 1);
    EXPECT_TRUE(const_it == info.begin() + 2);
}

TEST(InfoIteratorImplDeathTest, PostIncrementInvalid) {
    // create info object
    mpicxx::info info;

    // create singular iterator
    mpicxx::info::iterator sit;

    // create iterator referring to an info object referring to MPI_INFO_NULL
    mpicxx::info info_null;
    mpicxx::info::iterator info_null_it = info_null.begin();
    info_null = mpicxx::info(MPI_INFO_NULL, false);

    // incrementing a singular iterator is not permitted
    EXPECT_DEATH( sit++ , "");

    // incrementing a iterator referring to an info object referring to MPI_INFO_NULL is not permitted
    EXPECT_DEATH( info_null_it++ , "");

    // incrementing a past-the-end iterator is not permitted
    EXPECT_DEATH( (info.begin())++ , "");
}


TEST(InfoIteratorImplTest, AdvanceValid) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    {
        // check if compound-add works on an iterator
        mpicxx::info::iterator it = info.begin();
        it += 2;
        EXPECT_TRUE(it == info.begin() + 2);
        it = info.begin();
        EXPECT_TRUE((it += 2) == info.begin() + 2);
        it += -1;
        EXPECT_TRUE(it == info.begin() + 1);

        // check if compound-add works on a const_iterator
        mpicxx::info::const_iterator const_it = info.cbegin();
        const_it += 2;
        EXPECT_TRUE(const_it == info.begin() + 2);
        const_it = info.cbegin();
        EXPECT_TRUE((const_it += 2) == info.begin() + 2);
        const_it += -1;
        EXPECT_TRUE(const_it == info.begin() + 1);
    }
    {
        // check if add works on an iterator
        mpicxx::info::iterator it = info.begin();
        it = it + 2;
        EXPECT_TRUE(it == info.begin() + 2);
        it = info.begin();
        EXPECT_TRUE(it + 2 == info.begin() + 2);
        it = info.end();
        EXPECT_TRUE(it + -1 == info.begin() + 1);

        // check if add works on a const_iterator
        mpicxx::info::const_iterator const_it = info.cbegin();
        const_it = const_it + 2;
        EXPECT_TRUE(const_it == info.begin() + 2);
        const_it = info.cbegin();
        EXPECT_TRUE(const_it + 2 == info.begin() + 2);
        const_it = info.cend();
        EXPECT_TRUE(const_it + -1 == info.begin() + 1);
    }
    {
        // check if add works on an iterator
        mpicxx::info::iterator it = info.begin();
        it = 2 + it;
        EXPECT_TRUE(it == info.begin() + 2);
        it = info.begin();
        EXPECT_TRUE(2 + it == info.begin() + 2);
        it = info.end();
        EXPECT_TRUE(-1 + it == info.begin() + 1);

        // check if add works on a const_iterator
        mpicxx::info::const_iterator const_it = info.cbegin();
        const_it = 2 + const_it;
        EXPECT_TRUE(const_it == info.begin() + 2);
        const_it = info.cbegin();
        EXPECT_TRUE(2 + const_it == info.begin() + 2);
        const_it = info.cend();
        EXPECT_TRUE(-1 + const_it == info.begin() + 1);
    }
}

TEST(InfoIteratorImplDeathTest, AdvanceInvalid) {
    // create info object
    mpicxx::info info;

    // create singular iterator
    mpicxx::info::iterator sit;

    // create iterator referring to an info object referring to MPI_INFO_NULL
    mpicxx::info info_null;
    mpicxx::info::iterator info_null_it = info_null.begin();
    info_null = mpicxx::info(MPI_INFO_NULL, false);

    [[maybe_unused]] mpicxx::info::iterator it;
    // advancing a singular iterator is not permitted
    EXPECT_DEATH( it = sit += 1 , "");
    EXPECT_DEATH( it = sit + 1 , "");
    EXPECT_DEATH( it = 1 + sit , "");

    // advancing a iterator referring to an info object referring to MPI_INFO_NULL is not permitted
    EXPECT_DEATH( it = info_null_it += 1 , "");
    EXPECT_DEATH( it = info_null_it + 1 , "");
    EXPECT_DEATH( it = 1 + info_null_it , "");

    // advancing a past-the-end iterator is not permitted
    EXPECT_DEATH( it = info.end() += 1 , "");
    EXPECT_DEATH( it = info.end() + 1 , "");
    EXPECT_DEATH( it = 1 + info.end() , "");
    EXPECT_DEATH( it = info.begin() += -1 , "");
    EXPECT_DEATH( it = info.begin() + -1 , "");
    EXPECT_DEATH( it = -1 + info.begin() , "");
}


TEST(InfoIteratorImplTest, PreDecrementValid) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // check if pre-decrement works on an iterator
    mpicxx::info::iterator it = info.end();
    --it;
    EXPECT_TRUE(it == info.end() - 1);
    EXPECT_TRUE(--it == info.begin());

    // check if pre-decrement works on a const_iterator
    mpicxx::info::const_iterator const_it = info.cend();
    --const_it;
    EXPECT_TRUE(const_it == info.end() - 1);
    EXPECT_TRUE(--const_it == info.begin());
}

TEST(InfoIteratorImplDeathTest, PreDecrementInvalid) {
    // create info object
    mpicxx::info info;

    // create singular iterator
    mpicxx::info::iterator sit;

    // create iterator referring to an info object referring to MPI_INFO_NULL
    mpicxx::info info_null;
    mpicxx::info::iterator info_null_it = info_null.begin();
    info_null = mpicxx::info(MPI_INFO_NULL, false);

    // decrementing a singular iterator is not permitted
    EXPECT_DEATH( --sit , "");

    // decrementing a iterator referring to an info object referring to MPI_INFO_NULL is not permitted
    EXPECT_DEATH( --info_null_it , "");

    // decrementing a start-of-sequence iterator is not permitted
    EXPECT_DEATH( --info.begin() , "");
}


TEST(InfoIteratorImplTest, PostDecrementValid) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // check if post-decrement works on an iterator
    mpicxx::info::iterator it = info.end();
    it--;
    EXPECT_TRUE(it == info.end() - 1);
    EXPECT_TRUE(it-- == info.end() - 1);
    EXPECT_TRUE(it == info.begin());

    // check if post-decrement works on a const_iterator
    mpicxx::info::const_iterator const_it = info.cend();
    const_it--;
    EXPECT_TRUE(const_it == info.end() - 1);
    EXPECT_TRUE(const_it-- == info.end() - 1);
    EXPECT_TRUE(const_it == info.begin());
}

TEST(InfoIteratorImplDeathTest, PostDecrementInvalid) {
    // create info object
    mpicxx::info info;

    // create singular iterator
    mpicxx::info::iterator sit;

    // create iterator referring to an info object referring to MPI_INFO_NULL
    mpicxx::info info_null;
    mpicxx::info::iterator info_null_it = info_null.begin();
    info_null = mpicxx::info(MPI_INFO_NULL, false);

    // decrementing a singular iterator is not permitted
    EXPECT_DEATH(sit--, "");

    // decrementing a iterator referring to an info object referring to MPI_INFO_NULL is not permitted
    EXPECT_DEATH(info_null_it--, "");

    // decrementing a start-of-sequence iterator is not permitted
    EXPECT_DEATH((info.begin())--, "");
}


TEST(InfoIteratorImplTest, RetreatValid) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    {
        // check if compound-sub works on an iterator
        mpicxx::info::iterator it = info.end();
        it -= 2;
        EXPECT_TRUE(it == info.begin());
        it = info.end();
        EXPECT_TRUE((it -= 2) == info.begin());
        it -= -1;
        EXPECT_TRUE(it == info.begin() + 1);

        // check if compound-sub works on a const_iterator
        mpicxx::info::const_iterator const_it = info.cend();
        const_it -= 2;
        EXPECT_TRUE(const_it == info.begin());
        const_it = info.cend();
        EXPECT_TRUE((const_it -= 2) == info.begin());
        const_it -= -1;
        EXPECT_TRUE(const_it == info.begin() + 1);
    }
    {
        // check if sub works on an iterator
        mpicxx::info::iterator it = info.end();
        it = it - 2;
        EXPECT_TRUE(it == info.begin());
        it = info.end();
        EXPECT_TRUE(it - 2 == info.begin());
        it = info.begin();
        EXPECT_TRUE(it - -1 == info.begin() + 1);

        // check if add works on a const_iterator
        mpicxx::info::const_iterator const_it = info.cend();
        const_it = const_it - 2;
        EXPECT_TRUE(const_it == info.begin());
        const_it = info.cend();
        EXPECT_TRUE(const_it - 2 == info.begin());
        const_it = info.cbegin();
        EXPECT_TRUE(const_it - -1 == info.begin() + 1);
    }
}

TEST(InfoIteratorImplDeathTest, RetreatInvalid) {
    // create info object
    mpicxx::info info;

    // create singular iterator
    mpicxx::info::iterator sit;

    // create iterator referring to an info object referring to MPI_INFO_NULL
    mpicxx::info info_null;
    mpicxx::info::iterator info_null_it = info_null.begin();
    info_null = mpicxx::info(MPI_INFO_NULL, false);

    [[maybe_unused]] mpicxx::info::iterator it;
    // retreating a singular iterator is not permitted
    EXPECT_DEATH( it = sit -= 1 , "");
    EXPECT_DEATH( it = sit - 1 , "");

    // retreating a iterator referring to an info object referring to MPI_INFO_NULL is not permitted
    EXPECT_DEATH( it = info_null_it -= 1 , "");
    EXPECT_DEATH( it = info_null_it - 1 , "");

    // retreating a start-of-sequence iterator is not permitted
    EXPECT_DEATH( it = info.end() -= 1 , "");
    EXPECT_DEATH( it = info.end() - 1 , "");
    EXPECT_DEATH( it = info.begin() -= -1 , "");
    EXPECT_DEATH( it = info.begin() - -1 , "");
}