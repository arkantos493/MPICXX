/**
 * @file include/mpicxx/startup/init.hpp
 * @author Marcel Breyer
 * @date 2020-06-16
 *
 * @brief Implements wrapper around the MPI initialization functions.
 */

#ifndef MPICXX_INITIALIZATION_HPP
#define MPICXX_INITIALIZATION_HPP

#include <type_traits>

#include <mpi.h>

#include <mpicxx/detail/assert.hpp>
#include <mpicxx/exception/thread_support_exception.hpp>
#include <mpicxx/startup/thread_support.hpp>


namespace mpicxx {

    /// @name initialization of the MPI environment
    ///@{
    /**
     * @brief Checks whether @ref mpicxx::init() has completed.
     * @details It is valid to call @ref mpicxx::initialized() before @ref mpicxx::init() and after @ref mpicxx::finalize().
     *
     *          This function is thread safe as required by the [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf).
     * @return `true` if @ref mpicxx::init() has completed, otherwise `false` (`[[nodiscard]]`)
     *
     * @calls{ int MPI_Initialized(int *flag);    // exactly once }
     */
    [[nodiscard("Did you mean 'init()'?")]]
    inline bool initialized() {
        int flag;
        MPI_Initialized(&flag);
        return static_cast<bool>(flag);
    }

    /**
     * @brief Checks whether the MPI environment is currently active, i.e. @ref mpicxx::initialized() returns `true` and
     *        @ref mpicxx::finalized() returns `false`.
     * @details It is valid to call any mpicxx function while this function returns `true`.
     *
     *          This function is thread safe as required by the [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf).
     * @return `true` if currently the MPI environment is active, otherwise `false` (`[[nodiscard]]`)
     *
     * @calls{ int MPI_Initialized(int *flag);    // exactly once
     *         int MPI_Finalized(int *flag);      // exactly once }
     */
    [[nodiscard]]
    inline bool active() {
        int flag_init, flag_final;
        MPI_Initialized(&flag_init);
        MPI_Finalized(&flag_final);
        return static_cast<bool>(flag_init) && !static_cast<bool>(flag_final);
    }

    /**
     * @brief Initialize the MPI environment.
     * @details All MPI programs must contain exactly one call to an MPI initialization routine. Subsequent calls to any initialization
     *          routines are erroneous.
     *
     * @assert_precondition{ If the MPI environment has already been initialized. }
     *
     * @calls{ int MPI_Init(int *argc, char ***argv);       // exactly once  }
     */
    inline void init() {
        MPICXX_ASSERT_PRECONDITION(!initialized(), "MPI environment already initialized!");

        MPI_Init(nullptr, nullptr);
    }
    /**
     * @brief Initialize the MPI environment.
     * @details All MPI programs must contain exactly one call to an MPI initialization routine. Subsequent calls to any initialization
     *          routines are erroneous.
     * @param[inout] argc number of command line arguments
     * @param[inout] argv command line arguments
     *
     * @assert_precondition{ If the MPI environment has already been initialized. }
     *
     * @calls{ int MPI_Init(int *argc, char ***argv);       // exactly once }
     */
    inline void init(int& argc, char** argv) {
        MPICXX_ASSERT_PRECONDITION(!initialized(), "MPI environment already initialized!");

        MPI_Init(&argc, &argv);
    }

    /**
     * @brief Initialize the MPI environment with the required level of thread support (or higher).
     * @details All MPI programs must contain exactly one call to an MPI initialization routine. Subsequent calls to any initialization
     *          routines are erroneous.
     *
     *          A MPI implementation is not required to return the level of thread support requested by @p required if it can provide a
     *          higher level of thread support. For example if the requested level of thread support is `mpicxx::thread_support::single`
     *          (*MPI_THREAD_SINGLE*) an implementation could return `mpicxx::thread_support::multiple` (*MPI_THREAD_MULTIPLE*).
     * @param[in] required the required level of thread support
     * @return the provided level of thread support
     *
     * @assert_precondition{ If the MPI environment has already been initialized. }
     *
     * @throws mpicxx::thread_support_not_satisfied if the requested level of thread support cannot be satisfied
     *
     * @calls{ int MPI_Init_thread(int *argc, char ***argv, int required, int *provided);       // exactly once }
     */
    inline thread_support init(const thread_support required) {
        MPICXX_ASSERT_PRECONDITION(!initialized(), "MPI environment already initialized!");

        int provided_in;
        MPI_Init_thread(nullptr, nullptr, static_cast<int>(required), &provided_in);

        // throw an exception if the required level of thread support can't be satisfied
        thread_support provided = static_cast<thread_support>(provided_in);
        if (required > provided) {
            throw thread_support_not_satisfied(required, provided);
        }
        return provided;
    }
    /**
     * Initialize the MPI environment with the required level of thread support (or higher).
     * @details All MPI programs must contain exactly one call to an MPI initialization routine. Subsequent calls to any initialization
     *          routines are erroneous.
     *
     *          A MPI implementation is not required to return the level of thread support requested by @p required if it can provide a
     *          higher level of thread support. For example if the requested level of thread support is `mpicxx::thread_support::single`
     *          (*MPI_THREAD_SINGLE*) an implementation could return `mpicxx::thread_support::multiple` (*MPI_THREAD_MULTIPLE*).
     * @param[inout] argc number of command line arguments
     * @param[inout] argv command line arguments
     * @param[in] required the requested level of thread support
     * @return the provided level of thread support
     *
     * @assert_precondition{ If the MPI environment has already been initialized. }
     *
     * @throws mpicxx::thread_support_not_satisfied if the requested level of thread support cannot be satisfied
     *
     * @calls{ int MPI_Init_thread(int *argc, char ***argv, int required, int *provided);       // exactly once }
     */
    inline thread_support init(int& argc, char** argv, const thread_support required) {
        MPICXX_ASSERT_PRECONDITION(!initialized(), "MPI environment already initialized!");

        int provided_in;
        MPI_Init_thread(&argc, &argv, static_cast<int>(required), &provided_in);

        // throw an exception if the required level of thread support can't be satisfied
        thread_support provided = static_cast<thread_support>(provided_in);
        if (required > provided) {
            throw thread_support_not_satisfied(required, provided);
        }
        return provided;
    }

    /**
     * @brief Query the provided level of thread support.
     * @details Note that the provided level of thread support must **not** be equal to the requested level of thread support but could be
     *          higher. For example if the requested level of thread support is `mpicxx::thread_support::single` (*MPI_THREAD_SINGLE*) an
     *          implementation could return `mpicxx::thread_support::multiple` (*MPI_THREAD_MULTIPLE*).
     *
     *          This function is thread safe as required by the [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf).
     * @return the provided level of thread support (`[[nodiscard]]`)
     *
     * @calls{ int MPI_Query_thread(int *provided);     // exactly once }
     */
    [[nodiscard]]
    inline thread_support provided_thread_support() {
        int provided;
        MPI_Query_thread(&provided);
        return static_cast<thread_support>(provided);
    }

    /**
     * @brief Returns `true` if this thread is the main thread, i.e. the thread that called @ref mpicxx::init().
     * @details This function is thread safe as required by the [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf).
     * @return `true` if this is the main thread, otherwise `false` (`[[nodiscard]]`)
     *
     * @calls{ int MPI_Is_thread_main(int *flag);       // exactly once }
     */
    [[nodiscard]]
    inline bool is_main_thread() {
        int flag;
        MPI_Is_thread_main(&flag);
        return static_cast<bool>(flag);
    }
    ///@}

}

#endif // MPICXX_INITIALIZATION_HPP
