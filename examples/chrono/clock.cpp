/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-16
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Examples for the @ref mpicxx::chrono::clock implementation.
 */

//! [mwe]
#include <chrono>
#include <iostream>

#include <mpicxx/chrono/clock.hpp>

int main() {
    auto start_time = mpicxx::clock::now();

    // user code

    auto end_time = mpicxx::clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " ms" << std::endl;

    return 0;
}
//! [mwe]