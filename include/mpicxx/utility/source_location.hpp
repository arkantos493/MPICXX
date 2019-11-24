/**
 * @file source_location.hpp
 * @author Marcel Breyer
 * @date 2019-11-24
 *
 * @brief Provides the ``std::source_location`` implementation.
 *
 * Uses the ``std::source_location`` (or ``std::experimental::source_location``) implementation
 * if available else a custom implementation is provided.
 */

#ifndef MPICXX_SOURCE_LOCATION_HPP
#define MPICXX_SOURCE_LOCATION_HPP

#if __has_include(<source_location>)

/// use standard source_location if available
#include <source_location>
namespace mpicxx::utility {
    using source_location = std::source_location;
}

#elif __has_include(<experimental/source_location>)

/// use experimental source_location if available
#include <experimental/source_location>
namespace mpicxx::utility {
    using source_location = std::experimental::source_location;
}

#else

/// use custom implementation if no other is available
namespace mpicxx::utility {
    /**
     * Implementation of the ``std::source_location`` class.
     */
    class source_location {
    public:
        /**
         * Constructs a new source_location with respective information about the current call side.
         * @param file the file name (path)
         * @param func the function name
         * @param line the line number
         * @param column the column number (**ALWAYS** initialized to 0)
         * @return the source_location holding the call side location information
         */
        static source_location current(
                const char* file = __builtin_FILE(),
                const char* func = __builtin_FUNCITON(),
                const int line = __builtin_LINE(),
                const int column = 0
                ) noexcept {
            source_location loc;
            loc.file = file;
            loc.func = func;
            loc.line = line;
            loc.column = column;
            return loc;
        }

        /**
         * Returns the absolute path name of this file.
         * @return the file name
         */
        constexpr const char* file_name() const noexcept { return file_; }
        /**
         * Returns the function name without additional signature information (i.e. return type or parameters).
         * @return the function name
         */
        constexpr const char* function_name() const noexcept { return func_; }
        /**
         * Returns the line number.
         * @return the line number
         */
        constexpr int line() const noexcept { return line_; }
        /**
         * Returns the column number. **NOT** usefully set in @ref source_location::current().
         * @return the column number
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
