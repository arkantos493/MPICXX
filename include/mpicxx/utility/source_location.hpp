/**
 * @file source_location.hpp
 * @author Marcel Breyer
 * @date 2019-11-24
 *
 * @brief Provides the `std::source_location` implementation.
 *
 * Uses the `std::source_location` (or `std::experimental::source_location`) implementation
 * if available else a custom implementation is provided.
 */

#ifndef MPICXX_SOURCE_LOCATION_HPP
#define MPICXX_SOURCE_LOCATION_HPP

#if __has_include(<source_location>)

// use standard source_location if available
#include <source_location>
namespace mpicxx::utility {
    using source_location = std::source_location;
}

#elif __has_include(<experimental/source_location>)

// use experimental source_location if available
#include <experimental/source_location>
namespace mpicxx::utility {
    using source_location = std::experimental::source_location;
}

#else

// use custom implementation if no other is available
namespace mpicxx::utility {
    // TODO 2019-12-18 20:42 marcel: test! and maybe __PRETTY_FUNC__ ?
    /**
     * @brief Implementation of the
     * <a href="https://en.cppreference.com/w/cpp/utility/source_location">std::source_location</a> class.
     */
    class source_location {
    public:
        /**
         * @brief Constructs a new source_location with respective information about the current call side.
         * @param file the file name (path)
         * @param func the function name
         * @param line the line number
         * @param column the column number
         * @return the source_location holding the call side location information
         *
         * @attention @p column is always (independent of the call side position) default initialized to 0 (= unknown)
         */
        static source_location current(
                const char* file = __builtin_FILE(),
                const char* func = __builtin_FUNCTION(),
                const int line = __builtin_LINE(),
                const int column = 0
                ) noexcept {
            source_location loc;
            loc.file_ = file;
            loc.func_ = func;
            loc.line_ = line;
            loc.column_ = column;
            return loc;
        }

        /**
         * @brief Returns the absolute path name of this file.
         * @return the file name
         */
        constexpr const char* file_name() const noexcept { return file_; }
        /**
         * @brief Returns the function name without additional signature information (i.e. return type or parameters).
         * @return the function name
         */
        constexpr const char* function_name() const noexcept { return func_; }
        /**
         * @brief Returns the line number.
         * @return the line number
         */
        constexpr int line() const noexcept { return line_; }
        /**
         * @brief Returns the column number.
         * @return the column number
         *
         * @attention default value in @ref source_location::current() always 0
         */
        constexpr int column() const noexcept { return column_; }

    private:
        const char* file_ = "unknown";
        const char* func_ = "unknown";
        int line_ = 0;
        int column_ = 0;
    };
}

#endif

#endif // MPICXX_SOURCE_LOCATION_HPP
