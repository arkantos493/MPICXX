/**
 * @file include/mpicxx/chrono/clock.hpp
 * @author Marcel Breyer
 * @date 2020-02-17
 *
 * @brief Implements a wrapper class around the MPI timer functions.
 * @details These functions include *MPI_Wtime*, *MPI_Wtick* and the attribute *MPI_WTIME_IS_GLOBAL*.
 */

#ifndef MPICXX_TIME_HPP
#define MPICXX_TIME_HPP

#include <chrono>

#include <mpi.h>


namespace mpicxx {

    /**
     * @brief A clock wrapper for *MPI_Wtime* and *MPI_Wtick* which supports [`std::chrono`](https://en.cppreference.com/w/cpp/chrono).
     */
    struct clock {
        ///
        using duration = std::chrono::duration<double>;
        ///
        using rep = duration::rep;
        ///
        using period = duration::period;
        ///
        using time_point = std::chrono::time_point<clock>;
        ///
        static const bool is_steady = false;

        /**
         * @brief
         * @return
         */
        [[nodiscard]] static time_point now() noexcept {
            return time_point(duration(MPI_Wtime()));
        }

        /**
         * @brief
         * @return
         */
        [[nodiscard]] static double resolution() noexcept {
            return MPI_Wtick();
        }

        /**
         * @brief
         * @return
         */
        [[nodiscard]] static bool global_synchronized() {
            void* ptr;
            int flag;
            MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_WTIME_IS_GLOBAL, &ptr, &flag);
            if (static_cast<bool>(flag)) {
                return static_cast<bool>(*reinterpret_cast<int*>(ptr));
            } else {
                return false;
            }
        }
    };

}

#endif // MPICXX_TIME_HPP
