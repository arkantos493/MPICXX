/**
 * @file include/mpicxx/startup/finalization.hpp
 * @author Marcel Breyer
 * @date 2020-02-25
 *
 * @brief Implements wrapper around the MPI finalization functions.
 */

#ifndef MPICXX_FINALIZATION_HPP
#define MPICXX_FINALIZATION_HPP

#include <mpi.h>

#include <mpicxx/detail/assert.hpp>


namespace mpicxx {

    /**
     * @brief Checks whether @ref mpicxx::finalize() has completed.
     * @details It is valid to call @ref mpicxx::finalized() before @ref mpicxx::initialize() and after @ref mpicxx::finalize().
     * @return `true` if @ref mpicxx::finalize() has completed, otherwise `false`
     */
    [[nodiscard("Did you mean 'finalize()'?")]] inline bool finalized() {
        int flag;
        MPI_Finalized(&flag);
        return static_cast<bool>(flag);
    }

    /**
     * @brief Clean up the MPI state.
     * @details If an MPI program terminates normally (i.e., not due to a call to @ref mpicxx::abort() or an unrecoverable error) then each
     * process  must call @ref mpicxx::finalize() before it exits. Before an MPI process invokes @ref mpicxx::finalize(), the process must
     * perform all MPI calls needed to complete its involvement in MPI communications.
     *
     * Once @ref mpicxx::finalize() returns, no MPI routine (not even @ref mpicxx::initialize()) may be called, except for
     * @ref mpicxx::mpi_library_version(), @ref mpicxx::initialized(), @ref mpicxx::finalized(), and any MPI Tool
     * function.
     *
     * @assert_precondition{ If the MPI environment has already been finalized. }
     */
    inline void finalize() {
        MPICXX_ASSERT_PRECONDITION(!finalized(), "MPI environment already finalized!");

        MPI_Finalize();
    }

}

#endif // MPICXX_FINALIZATION_HPP
