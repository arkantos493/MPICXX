/**
 * @file include/mpicxx/chrono/clock.hpp
 * @author Marcel Breyer
 * @date 2020-03-16
 *
 * @brief Implements a wrapper class around the MPI timer functions.
 * @details These functions include *MPI_Wtime*, *MPI_Wtick* and the attribute *MPI_WTIME_IS_GLOBAL*.
 */

#ifndef MPICXX_CLOCK_HPP
#define MPICXX_CLOCK_HPP

#include <chrono>

#include <mpi.h>


namespace mpicxx {

    /**
     * @brief A clock wrapper for *MPI_Wtime* and *MPI_Wtick* which supports [`std::chrono`](https://en.cppreference.com/w/cpp/chrono).
     * @details Example usage:
     * @include examples/chrono/clock.cpp
     */
    struct clock {
        /**
         * @brief Duration, a [`std::chrono::duration`](https://en.cppreference.com/w/cpp/chrono/duration) type used to measure the time
         * since epoch.
         */
        using duration = std::chrono::duration<double>;
        /// An arithmetic type representing the number of ticks.
        using rep = duration::rep;
        /**
         * @brief `typename Period::type`, a [`std::ratio`](https://en.cppreference.com/w/cpp/numeric/ratio/ratio) representing the tick
         * period (i.e. the number of seconds per tick).
         */
        using period = duration::period;
        /// Represents a point in time associated with this custom clock.
        using time_point = std::chrono::time_point<clock>;
        /// *MPI_Wtime* does **not** make any guarantees to be steady.
        static const bool is_steady = true;

        /**
         * @brief Returns a floating-point number of seconds, representing elapsed wall-clock time since some time in the past.
         * @return the elapsed wall-clock time
         *
         * @calls{
         * MPI_Wtime();      // exactly once
         * }
         */
        [[nodiscard]] static time_point now() noexcept {
            return time_point(duration(MPI_Wtime()));
        }

        /**
         * @brief Returns the resolution of @ref mpicxx::clock::now() in seconds.
         * @details Example: if the clock is incremented every millisecond, this function would return \f$10^{-3}\f$.
         * @return a double precision value (the number of seconds between successive clock ticks)
         *
         * @calls{
         * double MPI_Wtick();      // exactly once
         * }
         */
        [[nodiscard]] static double resolution() noexcept {
            return MPI_Wtick();
        }

        // TODO 2020-02-18 19:39 marcel: change to the mpicxx equivalent
        /**
         * @brief Returns whether the clock is synchronized in the given communicator group (default: *MPI_COMM_WORLD*).
         * @details The global variable *MPI_WTIME_IS_GLOBAL* is set to 1 if clocks at all processes in *MPI_COMM_WORLD* are synchronized,
         * 0 otherwise. Because this variable need not be present when the clocks are not synchronized, the attribute key to
         * *MPI_Comm_get_attr* is used, which is always valid.
         * @param comm the communicator for which the synchronization should be checked
         * @return boolean variable that indicates whether clocks are synchronized
         *
         * @calls{
         * int MPI_Comm_get_attr(MPI_Comm comm, int comm_keyval, void *attribute_val, int *flag);      // exactly once
         * }
         */
        [[nodiscard]] static bool synchronized(MPI_Comm comm = MPI_COMM_WORLD) {
            void* ptr;
            int flag;
            MPI_Comm_get_attr(comm, MPI_WTIME_IS_GLOBAL, &ptr, &flag);
            if (static_cast<bool>(flag)) {
                return static_cast<bool>(*reinterpret_cast<int*>(ptr));
            } else {
                return false;
            }
        }
    };

}

#endif // MPICXX_CLOCK_HPP
