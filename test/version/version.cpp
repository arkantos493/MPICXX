/**
 * @file test/version/version.cpp
 * @author Marcel Breyer
 * @date 2020-02-21
 *
 * @brief Test cases for the version constants and function.
 * @details Testsuite: *VersionTest*
 * | test case name | test case description                        |
 * |:---------------|:---------------------------------------------|
 * | MPICXXVersion  | check the mpicxx version constants           |
 * | MPIVersion     | check the mpi version constants and function |
 */

#include <gtest/gtest.h>

#include <mpicxx/version/version.hpp>

#include <charconv>
#include <vector>


std::vector<std::string_view> split(const std::string_view& version, const char delim = '.') {
    std::vector<std::string_view> splitted;
    std::size_t last_pos = 0;
    for (std::size_t i = 0; i < version.size(); ++i) {
        if (version[i] == delim) {
            splitted.emplace_back(version.data() + last_pos, i - last_pos);
            last_pos = i + 1;
        }
    }
    splitted.emplace_back(version.data() + last_pos, version.size() - last_pos);
    return splitted;
}


TEST(VersionTest, MPICXXVersion) {
    // check library name
    EXPECT_STREQ(mpicxx::version::name.data(), "mpicxx");

    // split version number
    std::vector<std::string_view> version_splitted = split(mpicxx::version::version);

    // correct splitted version numbers
    std::vector<int> version_splitted_int =
            { mpicxx::version::version_major,
              mpicxx::version::version_minor,
              mpicxx::version::version_patch };

    ASSERT_EQ(version_splitted.size(), version_splitted_int.size());
    for (std::size_t i = 0; i < version_splitted.size(); ++i) {
        SCOPED_TRACE(i);
        int ver;
        auto result = std::from_chars(version_splitted[i].data(), version_splitted[i].data() + version_splitted[i].size(), ver);
        ASSERT_EQ(result.ec, std::errc());
        EXPECT_EQ(ver, version_splitted_int[i]);
    }

}

TEST(VersionTest, MPIVersion) {
    // split version number
    std::vector<std::string_view> version_splitted = split(mpicxx::version::mpi_version);

    // correct splitted version numbers
    std::vector<int> version_splitted_int = { mpicxx::version::mpi_version_major, mpicxx::version::mpi_version_minor };

    ASSERT_EQ(version_splitted.size(), version_splitted_int.size());
    for (std::size_t i = 0; i < version_splitted.size(); ++i) {
        SCOPED_TRACE(i);
        int ver;
        auto result = std::from_chars(version_splitted[i].data(), version_splitted[i].data() + version_splitted[i].size(), ver);
        ASSERT_EQ(result.ec, std::errc());
        EXPECT_EQ(ver, version_splitted_int[i]);
    }
}