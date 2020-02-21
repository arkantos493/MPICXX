/**
 * @file include/mpicxx/startup/initialization.hpp
 * @author Marcel Breyer
 * @date 2020-02-20
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

    /**
     * @brief Initialize the MPI state.
     * @details All MPI programs must contain exactly one call to an MPI initialization routine. Subsequent calls to any initialization
     * routines are erroneous.
     *
     * The only MPI functions that may be invoked before the MPI initialization routines are called are, @ref mpicxx::mpi_library_version(),
     * @ref mpicxx::initialized(), @ref mpicxx::finalized(), and any MPI Tool function.
     */
    inline void initialize() {
        MPI_Init(nullptr, nullptr);
    }
    /**
     * @copydoc mpicxx::initialize()
     * @param argc number of command line arguments
     * @param argv command line arguments
     */
    inline void initialize(int& argc, char** argv) {
        MPI_Init(&argc, &argv);
    }

    /**
     * @brief Initialize the MPI state with the required level of thread support (or higher).
     * @copydetails mpicxx::initialize()
     * @param required the required level of thread support
     * @return the provided level of thread support
     *
     * @throws mpicxx::thread_support_not_satisfied if the requested level of thread support can't be satisfied
     */
    inline thread_support initialize(const thread_support required) {
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
     * @param argc number of command line arguments
     * @param argv command line arguments
     * @param required the requested level of thread support
     * @return the provided level of thread support
     *
     * @throws mpicxx::thread_support_not_satisfied if the requested level of thread support can't be satisfied
     */
    inline thread_support initialize(int& argc, char** argv, const thread_support required) {
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
     * @brief Checks whether @ref mpicxx::initialize() has completed.
     * @details It is valid to call @ref mpicxx::initialized() before @ref mpicxx::initialize() and after @ref mpicxx::finalize().
     * @return `true` if @ref mpicxx::initialize() has completed, otherwise `false`
     */
    [[nodiscard("Did you mean 'initialize()'?")]] inline bool initialized() {
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

}

#endif // MPICXX_INITIALIZATION_HPP
