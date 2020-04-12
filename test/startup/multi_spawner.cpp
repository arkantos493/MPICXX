/**
 * @file test/startup/multi_spawner.cpp
 * @author Marcel Breyer
 * @date 2020-04-12
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner class wrapping the *MPI_COMM_SPAWN_MULTIPLE* function.
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

#include <mpicxx/startup/thread_support.hpp>

using namespace std::string_literals;


