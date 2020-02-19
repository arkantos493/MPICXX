/**
 * @file include/mpicxx/startup/initialization.hpp
 * @author Marcel Breyer
 * @date 2020-02-19
 *
 * @brief Implements wrapper around the MPI initialization functions.
 */

#ifndef MPICXX_INITIALIZATION_HPP
#define MPICXX_INITIALIZATION_HPP

#include <type_traits>

#include <mpi.h>

#include <mpicxx/startup/thread_support.hpp>
#include <mpicxx/startup/thread_support_exception.hpp>


namespace mpicxx {

    inline void initialize() {
        MPI_Init(nullptr, nullptr);
    }

    inline void initialize(int& argc, char** argv) {
        MPI_Init(&argc, &argv);
    }

    inline thread_support initialize(const thread_support required) {
        using thread_support_ut = std::underlying_type_t<thread_support>;

        thread_support_ut provided_ut;
        MPI_Init_thread(nullptr, nullptr, static_cast<thread_support_ut>(required), &provided_ut);

        thread_support provided = static_cast<thread_support>(provided_ut);
        if (required > provided) {
            throw thread_support_not_satisfied(required, provided);
        }
        return provided;
    }

    inline thread_support initialize(int& argc, char** argv, const thread_support required) {
        using thread_support_ut = std::underlying_type_t<thread_support>;

        thread_support_ut provided_ut;
        MPI_Init_thread(&argc, &argv, static_cast<thread_support_ut>(required), &provided_ut);

        thread_support provided = static_cast<thread_support>(provided_ut);
        if (required > provided) {
            throw thread_support_not_satisfied(required, provided);
        }
        return provided;
    }

    [[nodiscard]] inline thread_support provided_thread_support() {
        using thread_support_ut = std::underlying_type_t<thread_support>;

        thread_support_ut provided;
        MPI_Query_thread(&provided);
        return static_cast<thread_support>(provided);
    }

    [[nodiscard]] inline bool initialized() {
        int flag;
        MPI_Initialized(&flag);
        return static_cast<bool>(flag);
    }

    // TODO 2020-02-19 19:33 marcel: name
    [[nodiscard]] inline bool running() {
        int flag_init, flag_final;
        MPI_Initialized(&flag_init);
        MPI_Finalized(&flag_final);
        return static_cast<bool>(flag_init) && !static_cast<bool>(flag_final);
    }

}

#endif // MPICXX_INITIALIZATION_HPP
