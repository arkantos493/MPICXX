/**
 * @file include/mpicxx/startup/finalize.hpp
 * @author Marcel Breyer
 * @date 2020-03-15
 *
 * @brief Implements wrapper around the MPI finalization functions.
 */

#ifndef MPICXX_FINALIZATION_HPP
#define MPICXX_FINALIZATION_HPP

#include <mpi.h>

#include <mpicxx/detail/assert.hpp>


// TODO 2020-03-15 18:36 marcel: CALLS docu
namespace mpicxx {

    /**
     * @brief Checks whether @ref mpicxx::finalize() has completed.
     * @details It is valid to call @ref mpicxx::finalized() before @ref mpicxx::init() and after @ref mpicxx::finalize().
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
     * Once @ref mpicxx::finalize() returns, no MPI routine (not even @ref mpicxx::init()) may be called, except for
     * @ref mpicxx::mpi_library_version(), @ref mpicxx::initialized(), @ref mpicxx::finalized(), and any MPI Tool
     * function.
     *
     * @assert_precondition{ If the MPI environment has already been finalized. }
     */
    inline void finalize() {
        MPICXX_ASSERT_PRECONDITION(!finalized(), "MPI environment already finalized!");

        MPI_Finalize();
    }

    // TODO 2020-02-26 17:49 marcel: change to the mpicxx equivalent
    /**
     * @brief Attempts to abort all tasks in the communication group of @p comm.
     * @param error_code the returned error code (not necessarily returned from the executable)
     * @param comm the communicator whom's tasks to abort
     */
    inline void abort(int error_code = -1, MPI_Comm comm = MPI_COMM_WORLD) {
        MPI_Abort(comm, error_code);
    }

    namespace detail {
        // callback functions type
        using atfinalize_callback_t = void (*)(void);
        // registered callback functions
        std::array<atfinalize_callback_t, MAX_NUMBER_OF_ATFINALIZE_CALLBACKS> atfinalize_lookup_callbacks;
        // number of registered callback functions
        std::size_t atfinalize_idx = 0;

        /*
         * @brief Calls a specific callback function (previously registered).
         * @param comm not used
         * @param comm_key_val not used
         * @param attribute_val not used
         * @param extra_state not used
         * @return always `0`
         */
        int atfinalize_delete_fn([[maybe_unused]] MPI_Comm comm, [[maybe_unused]] int comm_key_val,
                [[maybe_unused]] void* attribute_val, [[maybe_unused]] void* extra_state) {
            // invoke handler function
            std::invoke(atfinalize_lookup_callbacks[--atfinalize_idx]);
            return 0;
        }
    }
    /**
     * @brief Registers the callback function @p func to be called directly before *MPI_Finalize*.
     * @details Calls all registered functions in reversed order in which they were set. This occurs before before any other parts of MPI
     * are affected, i.e. @ref mpicxx::finalized() will return `false` in any of these handler callback functions.
     *
     * At most `32` callback functions can be registered.
     * @param func pointer to a function to be called on normal MPI finalization
     * @return `0` if the registration succeeded, `1` otherwise
     *
     * @assert_precondition{ If @p func is the `nullptr`. }
     */
    int atfinalize(detail::atfinalize_callback_t func) {
        MPICXX_ASSERT_PRECONDITION(func != nullptr, "The callback function cannot be nullptr!");

        int comm_keyval;

        // return if the number of registered callbacks exceeds the limit
        if (detail::atfinalize_idx >= MAX_NUMBER_OF_ATFINALIZE_CALLBACKS) return 1;

        // register function
        detail::atfinalize_lookup_callbacks[detail::atfinalize_idx] = func;
        MPI_Comm_create_keyval(MPI_COMM_NULL_COPY_FN, &detail::atfinalize_delete_fn, &comm_keyval, nullptr);
        MPI_Comm_set_attr(MPI_COMM_SELF, comm_keyval, nullptr);
        ++detail::atfinalize_idx;

        return 0;
    }

}

#endif // MPICXX_FINALIZATION_HPP
