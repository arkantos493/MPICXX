/**
 * @file test/startup/multi_spawner.cpp
 * @author Marcel Breyer
 * @date 2020-03-24
 *
 * @brief Test cases for the @ref mpicxx::multi_spawner class wrapping the *MPI_COMM_SPAWN* function.
 * @details Testsuite: *StartupTest*
 * | test case name           | test case description                                                                             |
 * |:-------------------------|:--------------------------------------------------------------------------------------------------|
 * |                   |                                               |
 */

#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <mpicxx/startup/spawn.hpp>
#include <mpicxx/startup/thread_support.hpp>

using namespace std::string_literals;


