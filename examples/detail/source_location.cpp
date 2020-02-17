/**
 * @file examples/detail/source_location.cpp
 * @author Marcel Breyer
 * @date 2020-02-08
 *
 * @brief Examples for the @ref mpicxx::detail::source_location implementation.
 */

#include <iostream>
#include <string_view>

#include <mpicxx/detail/source_location>

void log(std::string_view message,
        const mpicxx::detail::source_location& loc = mpicxx::detail::source_location::current()) {
    std::cout << "info:"
              << location.file_name() << "\n"
              << loc.function_name() << "\n"
              << loc.line() << "\n"
              << message << std::endl;
}

int main() {
    // normal usage
    log("Hello, world!");

    // better function name
    log("Hello, world!", mpicxx::detail::source_location::current(PRETTY_FUNC_NAME__));

    return 0;
}