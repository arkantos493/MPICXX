/**
 * @file examples/chrono/clock.cpp
 * @author Marcel Breyer
 * @date 2020-07-08
 *
 * @brief Examples for the @ref mpicxx::chrono::clock implementation.
 */

//! [mwe]
#include <chrono>
#include <iostream>

#include <mpicxx/chrono/clock>


int main() {

    auto start_time = mpicxx::clock::now();

    // ...

    auto end_time = mpicxx::clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " ms" << std::endl;

    return 0;
}
//! [mwe]