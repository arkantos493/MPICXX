/**
 * @file include/mpicxx/startup/init.hpp
 * @author Marcel Breyer
 * @date 2020-03-01
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

    /**
     * @brief Checks whether @ref mpicxx::init() has completed.
     * @details It is valid to call @ref mpicxx::initialized() before @ref mpicxx::init() and after @ref mpicxx::finalize().
     * @return `true` if @ref mpicxx::init() has completed, otherwise `false`
     */
    [[nodiscard("Did you mean 'init()'?")]] inline bool initialized() {
        int flag;
        MPI_Initialized(&flag);
        return static_cast<bool>(flag);
    }

    /**
     * @brief Checks whether currently the MPI environment is active, i.e. @ref mpicxx::initialized() returns `true` and
     * @ref mpicxx::finalized() returns `false`.
     * @details It is valid to call any mpicxx function while this function returns `true`.
     * @return `true` if currently the MPI environment is active, otherwise `false`
     */
    [[nodiscard]] inline bool running() {
        int flag_init, flag_final;
        MPI_Initialized(&flag_init);
        MPI_Finalized(&flag_final);
        return static_cast<bool>(flag_init) && !static_cast<bool>(flag_final);
    }

    /**
     * @brief Initialize the MPI state.
     * @details All MPI programs must contain exactly one call to an MPI initialization routine. Subsequent calls to any initialization
     * routines are erroneous.
     *
     * The only MPI functions that may be invoked before the MPI initialization routines are called are, @ref mpicxx::mpi_library_version(),
     * @ref mpicxx::initialized(), @ref mpicxx::finalized(), and any MPI Tool function.
     *
     * @assert_precondition{ If the MPI environment has already been initialized. }
     */
    inline void init() {
        MPICXX_ASSERT_PRECONDITION(!initialized(), "MPI environment already initialized!");

        MPI_Init(nullptr, nullptr);
    }
    /**
     * @brief Initialize the MPI state.
     * @details All MPI programs must contain exactly one call to an MPI initialization routine. Subsequent calls to any initialization
     * routines are erroneous.
     *
     * The only MPI functions that may be invoked before the MPI initialization routines are called are, @ref mpicxx::mpi_library_version(),
     * @ref mpicxx::initialized(), @ref mpicxx::finalized(), and any MPI Tool function.
     * @param argc number of command line arguments
     * @param argv command line arguments
     *
     * @assert_precondition{ If the MPI environment has already been initialized. }
     */
    inline void init(int argc, char** argv) {
        MPICXX_ASSERT_PRECONDITION(!initialized(), "MPI environment already initialized!");

        MPI_Init(&argc, &argv);
    }

    /**
     * @brief Initialize the MPI state with the required level of thread support (or higher).
     * @details All MPI programs must contain exactly one call to an MPI initialization routine. Subsequent calls to any initialization
     * routines are erroneous.
     *
     * The only MPI functions that may be invoked before the MPI initialization routines are called are, @ref mpicxx::mpi_library_version(),
     * @ref mpicxx::initialized(), @ref mpicxx::finalized(), and any MPI Tool function.
     * @param required the required level of thread support
     * @return the provided level of thread support
     *
     * @assert_precondition{ If the MPI environment has already been initialized. }
     *
     * @throws mpicxx::thread_support_not_satisfied if the requested level of thread support can't be satisfied
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
     * Initialize the MPI state with the required level of thread support (or higher).
     * @details All MPI programs must contain exactly one call to an MPI initialization routine. Subsequent calls to any initialization
     * routines are erroneous.
     *
     * The only MPI functions that may be invoked before the MPI initialization routines are called are, @ref mpicxx::mpi_library_version(),
     * @ref mpicxx::initialized(), @ref mpicxx::finalized(), and any MPI Tool function.
     * @param argc number of command line arguments
     * @param argv command line arguments
     * @param required the requested level of thread support
     * @return the provided level of thread support
     *
     * @assert_precondition{ If the MPI environment has already been initialized. }
     *
     * @throws mpicxx::thread_support_not_satisfied if the requested level of thread support can't be satisfied
     */
    inline thread_support init(int argc, char** argv, const thread_support required) {
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
     * @brief Query the proved level of thread support.
     * @return the provided level of thread support
     */
    [[nodiscard]] inline thread_support provided_thread_support() {
        int provided;
        MPI_Query_thread(&provided);
        return static_cast<thread_support>(provided);
    }

    /**
     * @brief Returns `true` if this thread is the main thread, i.e. this thread that called @ref mpicxx::init().
     * @return `true` if this is the main thread, otherwise `false`
     */
    [[nodiscard]] inline bool is_main_thread() {
        int flag;
        MPI_Is_thread_main(&flag);
        return static_cast<bool>(flag);
    }

}

#endif // MPICXX_INITIALIZATION_HPP
