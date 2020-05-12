/**
 * @file test/startup/multiple_spawner/constructor.cpp
 * @author Marcel Breyer
 * @date 2020-05-12
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner class command member functions.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name    | test case description                                     |
 * |:------------------|:----------------------------------------------------------|
 */

#include <utility>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/startup/multiple_spawner.hpp>
#include <mpicxx/startup/single_spawner.hpp>

using namespace std::string_literals;

#include <iostream>

TEST(MultipleSpawnerTest, ConstructFromIteratorRange) {
    // create new multiple_spawner object
    std::vector<std::pair<std::string, int>> vec1 = {
            { "foo", 1 },
            { "bar"s, 1 }
    };
    mpicxx::multiple_spawner ms1(vec1.begin(), vec1.end());

    // create another multiple_spawner object
    std::vector<std::pair<const char*, int>> vec2 = {
            { "foo", 1 },
            { "bar", 1 }
    };
    mpicxx::multiple_spawner ms2(vec2.begin(), vec2.end());
}

TEST(MultipleSpawnerDeathTest, ConstructFromInvalidIteratorRange) {
    // try creating a new multiple_spawner from an invalid iterator range
    std::vector<std::pair<std::string, int>> vec = { { "foo", 1 } };

    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.begin(), vec.begin()) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.end(), vec.begin()) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromIteratorRangeInvalidCommand) {
    // try to create new multiple_spawner with an empty command name
    std::vector<std::pair<std::string, int>> vec = { { "", 1 } };
    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.begin(), vec.end()) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromIteratorRangeInvalidMaxprocs) {
    // try to create new multiple_spawner with invalid number of maxprocs
    std::vector<std::pair<std::string, int>> vec = { { "foo", -1 } };
    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.begin(), vec.end()) , "");
    vec[0].second = 0;
    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.begin(), vec.end()) , "");
    vec[0].second = std::numeric_limits<int>::max();
    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.begin(), vec.end()) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromIteratorRangeInvalidTotalMaxprocs) {
    // try to create a new multiple_spawner with an invalid total number of maxprocs
    const auto max = mpicxx::multiple_spawner::universe_size().value_or(std::numeric_limits<int>::max() - 4);
    std::vector<std::pair<std::string, int>> vec = {
            { "foo", max / 4 + 1 }, { "bar", max / 4 + 1 },
            { "baz", max / 4 + 1 }, { "qux", max / 4 + 1 }
    };
    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.begin(), vec.end()) , "");
}


TEST(MultipleSpawnerTest, ConstructFromInitializerList) {
    // create new multiple_spawner using a initializer_list
    mpicxx::multiple_spawner ms({
            { "foo", 1 },
            { "bar"s, 1 }
    });
}

TEST(MultipleSpawnerDeathTest, ConstructFromInitializerListInvalidCommand) {
    // try to create new multiple_spawner with an empty command name
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ { "", 1 } }) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromInitializerListInvalidMaxprocs) {
    // try to create new multiple_spawner with invalid number of maxprocs
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ { "foo", -1 } }) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ { "foo", 0 } }) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ { "foo", std::numeric_limits<int>::max() } }) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromInitializerListInvalidTotalMaxprocs) {
    // try to create a new multiple_spawner with an invalid total number of maxprocs
    const auto max = mpicxx::multiple_spawner::universe_size().value_or(std::numeric_limits<int>::max() - 4);
    ASSERT_DEATH( mpicxx::multiple_spawner ms({ { "foo", max / 4 + 1 }, { "bar", max / 4 + 1 },
                                                { "baz", max / 4 + 1 }, { "qux", max / 4 + 1 } }) , "");
}


TEST(MultipleSpawnerTest, ConstructFromParameterPack) {
    // create new multiple_spawner using variadic templates
    std::pair<std::string, int> pair = std::make_pair("bar", 1);
    mpicxx::multiple_spawner ms(std::make_pair( "foo", 1 ), pair);
}

TEST(MultipleSpawnerDeathTest, ConstructFromParameterPackInvalidCommand) {
    // try to create new multiple_spawner with an empty command name
    ASSERT_DEATH( mpicxx::multiple_spawner ms(std::make_pair("", 1)) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromParameterPackInvalidMaxprocs) {
    // try to create new multiple_spawner with invalid number of maxprocs
    ASSERT_DEATH( mpicxx::multiple_spawner ms(std::make_pair("foo", -1)) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms(std::make_pair("foo", 0)) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms(std::make_pair("foo", std::numeric_limits<int>::max())) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromParameterPackInvalidTotalMaxprocs) {
    // try to create a new multiple_spawner with an invalid total number of maxprocs
    const auto max = mpicxx::multiple_spawner::universe_size().value_or(std::numeric_limits<int>::max() - 4);
    ASSERT_DEATH( mpicxx::multiple_spawner ms(std::make_pair("foo", max / 4 + 1), std::make_pair("bar", max / 4 + 1),
            std::make_pair("baz", max / 4 + 1), std::make_pair("qux", max / 4 + 1)) , "");
}


TEST(MultipleSpanwerTest, ConstructFromSpawner) {
    // create new multiple_spawner using other spawners
    mpicxx::single_spawner ss1("foo", 1);
    mpicxx::multiple_spawner ms1({ {"bar", 1} });

    mpicxx::multiple_spawner ms(ss1, ms1);
}

//TEST(MultipleSpawnerTest, ConstructFromSpawnerInvalidRoots) {
//    // create spawners with different roots
//    mpicxx::single_spawner ss1("foo", 1);
//    ss1.set_root(0);
//    mpicxx::single_spawner ss2("bar", 1);
//    ss2.set_root(1);
//
//    mpicxx::multiple_spawner ms(ss1, ss2);
//}