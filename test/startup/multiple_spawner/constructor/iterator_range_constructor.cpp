/**
 * @file test/startup/multiple_spawner/constructor/iterator_range_constructor.cpp
 * @author Marcel Breyer
 * @date 2020-06-15
 *
 * @brief Test cases for the
 *        @ref mpicxx::multiple_spawner::multiple_spawner(InputItCommands, InputItCommands, InputItMaxprocs, InputItMaxprocs) and
 *        @ref mpicxx::multiple_spawner::multiple_spawner(InputIt, InputIt) member function provided by the
 *        @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                                     | test case description                                                                               |
 * |:---------------------------------------------------|:----------------------------------------------------------------------------------------------------|
 * | ConstructFromIteratorRange                         | construct a multiple_spawner object from an iterator range                                          |
 * | ConstructFromInvalidIteratorRange                  | illegal iterator range (death test)                                                                 |
 * | ConstructFromIteratorRangeInvalidName              | try to construct a multiple_spawner object with an illegal executable name (death test)             |
 * | ConstructFromIteratorRangeInvalidMaxprocs          | try to construct a multiple_spawner object with an illegal maxprocs number (death test)             |
 * | ConstructFromIteratorRangeInvalidTotalMaxprocs     | try to construct a multiple_spawner object with an illegal total maxprocs number (death test)       |
 * | ConstructFromTwoIteratorRanges                     | construct a multiple_spawner object from two iterator ranges                                        |
 * | ConstructFromTwoIteratorRangesDifferentSizes       | try to construct a multiple_spawner object with two iterator ranges of different sizes (death test) |
 * | ConstructFromTwoInvalidIteratorRanges              | illegal iterator ranges (death test)                                                                |
 * | ConstructFromTwoIteratorRangesInvalidName          | try to construct a multiple_spawner object with an illegal executable name (death test)             |
 * | ConstructFromTwoIteratorRangesInvalidMaxprocs      | try to construct a multiple_spawner object with an illegal maxprocs number (death test)             |
 * | ConstructFromTwoIteratorRangesInvalidTotalMaxprocs | try to construct a multiple_spawner object with an illegal total maxprocs number (death test)       |
 */

#include <initializer_list>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <mpicxx/info/runtime_info.hpp>
#include <mpicxx/startup/multiple_spawner.hpp>

using namespace std::string_literals;


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

TEST(MultipleSpawnerDeathTest, ConstructFromIteratorRangeInvalidName) {
    // try to create new multiple_spawner with an empty executable name
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
    const auto max = mpicxx::universe_size().value_or(std::numeric_limits<int>::max() - 4);
    std::vector<std::pair<std::string, int>> vec = {
            { "foo", max / 4 + 1 }, { "bar", max / 4 + 1 },
            { "baz", max / 4 + 1 }, { "qux", max / 4 + 1 }
    };
    ASSERT_DEATH( mpicxx::multiple_spawner ms(vec.begin(), vec.end()) , "");
}


TEST(MultipleSpawnerTest, ConstructFromTwoIteratorRanges) {
    // create new multiple_spawner object
    std::vector<std::string> commands = { "foo", "bar" };
    std::vector<int> maxprocs = { 1, 1 };
    mpicxx::multiple_spawner ms1(commands.begin(), commands.end(), maxprocs.cbegin(), maxprocs.cend());
}

TEST(MultipleSpawnerDeathTest, ConstructFromTwoIteratorRangesDifferentSizes) {
    // try creating a new multiple_spawner from iterator ranges of different sizes
    std::vector<std::string> commmands = { "foo", "bar" };
    std::vector<int> maxprocs = { 1 };

    ASSERT_DEATH( mpicxx::multiple_spawner ms(commmands.begin(), commmands.end(), maxprocs.begin(), maxprocs.end()) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromTwoInvalidIteratorRanges) {
    // try creating a new multiple_spawner from invalid iterator ranges
    std::vector<std::string> commands = { "foo" };
    std::vector<int> maxprocs = { 1 };

    ASSERT_DEATH( mpicxx::multiple_spawner ms(commands.begin(), commands.begin(), maxprocs.begin(), maxprocs.end()) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms(commands.end(), commands.begin(), maxprocs.begin(), maxprocs.end()) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms(commands.begin(), commands.end(), maxprocs.begin(), maxprocs.begin()) , "");
    ASSERT_DEATH( mpicxx::multiple_spawner ms(commands.begin(), commands.end(), maxprocs.end(), maxprocs.begin()) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromTwoIteratorRangesInvalidName) {
    // try to create new multiple_spawner with an empty executable name
    std::vector<std::string> commands = { "" };
    std::vector<int> maxprocs = { 1 };
    ASSERT_DEATH( mpicxx::multiple_spawner ms(commands.begin(), commands.end(), maxprocs.begin(), maxprocs.end()) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromTwoIteratorRangesInvalidMaxprocs) {
    // try to create new multiple_spawner with invalid number of maxprocs
    std::vector<std::string> commands = { "foo" };
    std::vector<int> maxprocs = { -1 };
    ASSERT_DEATH( mpicxx::multiple_spawner ms(commands.begin(), commands.end(), maxprocs.begin(), maxprocs.end()) , "");
    maxprocs[0] = 0;
    ASSERT_DEATH( mpicxx::multiple_spawner ms(commands.begin(), commands.end(), maxprocs.begin(), maxprocs.end()) , "");
    maxprocs[0] = std::numeric_limits<int>::max();
    ASSERT_DEATH( mpicxx::multiple_spawner ms(commands.begin(), commands.end(), maxprocs.begin(), maxprocs.end()) , "");
}

TEST(MultipleSpawnerDeathTest, ConstructFromTwoIteratorRangesInvalidTotalMaxprocs) {
    // try to create a new multiple_spawner with an invalid total number of maxprocs
    const auto max = mpicxx::universe_size().value_or(std::numeric_limits<int>::max() - 4);
    std::vector<std::string> commands = { "foo", "bar", "baz", "qux" };
    std::vector<int> maxprocs(4, max / 4 + 1);
    ASSERT_DEATH( mpicxx::multiple_spawner ms(commands.begin(), commands.end(), maxprocs.begin(), maxprocs.end()) , "");
}