/**
 * @file test/startup/multiple_spawner/root.cpp
 * @author Marcel Breyer
 * @date 2020-06-02
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_root(const int) and @ref mpicxx::multiple_spawner::root() const member
 *        function provided by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name | test case description                       |
 * |:---------------|:--------------------------------------------|
 * | SetRoot        | set a new root process                      |
 * | SetInvalidRoot | set a new illegal root process (death test) |
 * | GetRoot        | get the current root process                |
 */

#include <initializer_list>
#include <utility>

#include <gtest/gtest.h>

#include <mpicxx/startup/multiple_spawner.hpp>


TEST(MultipleSpawnerTest, SetRoot) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set a new root
    ms.set_root(1);

    // check whether the root has been updated
    EXPECT_EQ(ms.root(), 1);
}

TEST(MultipleSpawnerDeathTest, SetInvalidRoot) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set a new illegal root
    ASSERT_DEATH( ms.set_root(-1) , "");
    ASSERT_DEATH( ms.set_root(2)  , "");
}

TEST(MultipleSpawnerTest, GetRoot) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // check getter
    EXPECT_EQ(ms.root(), 0);
    ms.set_root(1);
    EXPECT_EQ(ms.root(), 1);
}