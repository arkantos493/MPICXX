/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::thread_support enum class.
 * @details Testsuite: *StartupTest*
 * | test case name                                      | test case description                                                                                                                                                                           |
 * |:----------------------------------------------------|:------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
 * | CorrectEnumClassValues                              | check if the enum class reflects the correct MPI values                                                                                                                                         |
 * | ToStringViaFormat                                   | check whether the conversion to [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) via [`fmt::format`](https://fmt.dev/latest/syntax.html) works                            |
 * | ToStringViaToString                                 | check whether the conversion to [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) via [`to_string`](https://en.cppreference.com/w/cpp/string/basic_string/to_string) works |
 * | ToStringViaStreamInsertionOperator                  | check whether the conversion to [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) via the stream-extraction operator works                                                 |
 * | ToEnumClass                                         | check whether the conversion from a string via @ref mpicxx::thread_support_from_string(const std::string_view) works                                                                            |
 * | ToEnumClassInvalidString                            | try to convert an invalid string to the enum class via @ref mpicxx::thread_support_from_string(const std::string_view)                                                                          |
 * | ToEnumClassViaStreamExtractionOperator              | check whether the conversion from an input stream via the stream-insertion operator works                                                                                                       |
 * | ToEnumClassViaStreamExtractionOperatorInvalidString | try to convert an invalid string to the enum class via the stream-insertion operator                                                                                                            |
 */

#include <mpicxx/startup/thread_support.hpp>
#include <test_utility.hpp>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

using namespace std::string_literals;

TEST(StartupTest, CorrectEnumClassValues) {
    // the values should all be equal
    using enum_t = std::underlying_type_t<mpicxx::thread_support>;
    EXPECT_EQ(static_cast<enum_t>(mpicxx::thread_support::single), MPI_THREAD_SINGLE);
    EXPECT_EQ(static_cast<enum_t>(mpicxx::thread_support::funneled), MPI_THREAD_FUNNELED);
    EXPECT_EQ(static_cast<enum_t>(mpicxx::thread_support::serialized), MPI_THREAD_SERIALIZED);
    EXPECT_EQ(static_cast<enum_t>(mpicxx::thread_support::multiple), MPI_THREAD_MULTIPLE);
}

TEST(StartupTest, ToStringViaFormat) {
    // conversion via fmt::format should work as expected
    EXPECT_EQ(fmt::format("{}", mpicxx::thread_support::single),     "MPI_THREAD_SINGLE"s);
    EXPECT_EQ(fmt::format("{}", mpicxx::thread_support::funneled),   "MPI_THREAD_FUNNELED"s);
    EXPECT_EQ(fmt::format("{}", mpicxx::thread_support::serialized), "MPI_THREAD_SERIALIZED"s);
    EXPECT_EQ(fmt::format("{}", mpicxx::thread_support::multiple),   "MPI_THREAD_MULTIPLE"s);
}

TEST(StartupTest, ToStringViaToString) {
    // conversion via to_string should work as expected
    EXPECT_EQ(to_string(mpicxx::thread_support::single),     "MPI_THREAD_SINGLE"s);
    EXPECT_EQ(to_string(mpicxx::thread_support::funneled),   "MPI_THREAD_FUNNELED"s);
    EXPECT_EQ(to_string(mpicxx::thread_support::serialized), "MPI_THREAD_SERIALIZED"s);
    EXPECT_EQ(to_string(mpicxx::thread_support::multiple),   "MPI_THREAD_MULTIPLE"s);
}

TEST(StartupTest, ToStringViaStreamInsertionOperator) {
    // create vectors with valid values
    std::vector<mpicxx::thread_support> enum_vec = { mpicxx::thread_support::single, mpicxx::thread_support::funneled,
                                                     mpicxx::thread_support::serialized, mpicxx::thread_support::multiple };
    std::vector<std::string> string_vec = { "MPI_THREAD_SINGLE"s, "MPI_THREAD_FUNNELED"s,
                                            "MPI_THREAD_SERIALIZED"s, "MPI_THREAD_MULTIPLE"s };
    // sanity check
    ASSERT_EQ(enum_vec.size(), string_vec.size());

    // conversion via operator<< should work as expected
    std::stringstream ss;
    for (std::size_t i = 0; i < enum_vec.size(); ++i) {
        SCOPED_TRACE(i);
        ss << enum_vec[i];
        EXPECT_EQ(ss.str(), string_vec[i]);
        ss.str(std::string());
        ss.clear();
    }
}

TEST(StartupTest, ToEnumClass) {
    // conversion from string to enum value should work as expected
    EXPECT_EQ(mpicxx::thread_support_from_string("MPI_THREAD_SINGLE"),     mpicxx::thread_support::single);
    EXPECT_EQ(mpicxx::thread_support_from_string("MPI_THREAD_FUNNELED"),   mpicxx::thread_support::funneled);
    EXPECT_EQ(mpicxx::thread_support_from_string("MPI_THREAD_SERIALIZED"), mpicxx::thread_support::serialized);
    EXPECT_EQ(mpicxx::thread_support_from_string("MPI_THREAD_MULTIPLE"),   mpicxx::thread_support::multiple);

}

TEST(StartupTest, ToEnumClassInvalidString) {
    // try to convert an illegal string value
    [[maybe_unused]] mpicxx::thread_support ts;
    EXPECT_THROW_WHAT(
            ts = mpicxx::thread_support_from_string("INVALID_VALUE"),
            std::invalid_argument,
            "Can't convert \"INVALID_VALUE\" to mpicxx::thread_support!"
    );
}

TEST(StartupTest, ToEnumClassViaStreamExtractionOperator) {
    // create vector with valid values
    std::vector<mpicxx::thread_support> enum_vec = { mpicxx::thread_support::single, mpicxx::thread_support::funneled,
                                                     mpicxx::thread_support::serialized, mpicxx::thread_support::multiple };
    // conversion via operator>> should work as expected
    std::stringstream ss("MPI_THREAD_SINGLE MPI_THREAD_FUNNELED MPI_THREAD_SERIALIZED MPI_THREAD_MULTIPLE"s);
    for (std::size_t i = 0; i < enum_vec.size(); ++i) {
        SCOPED_TRACE(i);
        mpicxx::thread_support ts;
        ss >> ts;
        EXPECT_EQ(ts, enum_vec[i]);
    }
}

TEST(StartupTest, ToEnumClassViaStreamExtractionOperatorInvalidString) {
    // try to convert an illegal string value
    std::stringstream ss("INVALID_VALUE");
    mpicxx::thread_support ts;
    ss >> ts;
    EXPECT_TRUE(ss.fail());
}