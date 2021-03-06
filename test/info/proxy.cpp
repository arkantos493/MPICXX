/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-04
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::proxy class that is used to distinguish between read and write access of a [key, value]-pair
 *        of a @ref mpicxx::info object.
 * @details Testsuite: *InfoProxyTest*
 * | test case name             | test case description                                   |
 * |:---------------------------|:--------------------------------------------------------|
 * | ProxyWriteAccessValid      | write access the proxy                                  |
 * | ProxyWriteAccessInvalid    | invalid write access the proxy (death test)             |
 * | ProxyReadAccessValid       | read access the proxy                                   |
 * | ProxyReadAccessInvalid     | invalid read access the proxy (death test)              |
 * | ProxyOutputOperatorValid   | stream-insertion operator overload                      |
 * | ProxyOutputOperatorInvalid | invalid stream-insertion operator overload (death test) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

#include <sstream>

TEST(InfoProxyTest, ProxyWriteAccessValid) {
    // create info object
    mpicxx::info info;

    // create proxy
    auto p = info["key"];

    // attempt write access
    p = "value";

    // check if info contains new [key, value]-pair
    char value[MPI_MAX_INFO_VAL];
    int flag;
    MPI_Info_get(info.get(), "key", 5, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value");
}

TEST(InfoProxyDeathTest, ProxyWriteAccessInvalid) {
    // create info object
    mpicxx::info info;

    // create proxy
    auto p = info["key"];

    // attempt write access with illegal value size
    EXPECT_DEATH( p = "" , "");
    std::string value(MPI_MAX_INFO_VAL, ' ');
    EXPECT_DEATH( p = value , "");

    // attempt write access on a proxy that refers to an info object referring to MPI_INFO_NULL
    info = mpicxx::info(MPI_INFO_NULL, false);
    EXPECT_DEATH( p = "value" , "");
}


TEST(InfoProxyTest, ProxyReadAccessValid) {
    // create info object
    mpicxx::info info;

    // create proxy
    auto p1 = info["key"];

    // attempt read access
    std::string value = p1;

    // check for the correct value
    EXPECT_STREQ(value.c_str(), " ");

    // add [key, value]-pair to info object
    MPI_Info_set(info.get(), "key2", "value2");
    auto p2 = info["key2"];

    // attempt read access
    value = p2;

    // check for the correct value
    EXPECT_STREQ(value.c_str(), "value2");
}

TEST(InfoProxyDeathTest, ProxyReadAccessInvalid) {
    // create info object
    mpicxx::info info;

    // create proxy
    auto p = info["key"];

    // attempt read access on a proxy that refers to an info object referring to MPI_INFO_NULL
    info = mpicxx::info(MPI_INFO_NULL, false);
    [[maybe_unused]] std::string str;
    EXPECT_DEATH( str = static_cast<std::string>(p) , "");
}


TEST(InfoProxyTest, ProxyOutputOperatorValid) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key", "value");

    // create proxy
    auto p = info["key"];

    // write proxy to output stream
    std::stringstream ss;
    ss << p;

    // check for the correct value
    EXPECT_STREQ(ss.str().c_str(), "value");
}

TEST(InfoProxyDeathTest, ProxyOutputOperatorInvalid) {
    // create info object
    mpicxx::info info;

    // create proxy
    auto p = info["key"];

    // attempt read access on a proxy that refers to an info object referring to MPI_INFO_NULL
    info = mpicxx::info(MPI_INFO_NULL, false);
    std::stringstream ss;
    EXPECT_DEATH( ss << p; , "");
}