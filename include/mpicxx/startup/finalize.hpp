/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-16
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Implements wrapper around the [MPI finalization functions](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node225.htm).
 */

#ifndef MPICXX_FINALIZATION_HPP
#define MPICXX_FINALIZATION_HPP

#include <mpicxx/detail/assert.hpp>
#include <mpicxx/error/error.hpp>

#include <mpi.h>

namespace mpicxx {

    /// @name finalization of the MPI environment
    ///@{
    /**
     * @brief Checks whether @ref mpicxx::finalize() has completed.
     * @details It is valid to call @ref mpicxx::finalized() before @ref mpicxx::init() and after @ref mpicxx::finalize().
     *
     *    This function is thread safe as required by the [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf).
     * @return `true` if @ref mpicxx::finalize() has completed, otherwise `false`
     * @nodiscard
     *
     * @calls{ int MPI_Finalized(int *flag);    // exactly once }
     */
    [[nodiscard("Did you mean 'finalize()'?")]]
    inline bool finalized() {
        int flag;
        MPI_Finalized(&flag);
        return static_cast<bool>(flag);
    }

    /**
     * @brief Clean up the MPI state.
     * @details If a MPI program terminates normally (i.e., not due to a call to @ref mpicxx::abort() or an unrecoverable error) then each
     *          process must call @ref mpicxx::finalize() before it exits. Before an MPI process invokes @ref mpicxx::finalize(), the
     *          process must perform all MPI calls needed to complete its involvement in MPI communications.
     *
     * @pre The MPI environment **must not** be finalized.
     * @post The MPI environment has been finalized, i.e. it's illegal to call any mpicxx function.
     *       The only exceptions are:
     *         - @ref mpicxx::version::mpi_version(), @ref mpicxx::version::mpi_version_major(), @ref mpicxx::version::mpi_version_minor()
     *         - @ref mpicxx::version::mpi_library_version(), @ref mpicxx::version::mpi_library_name()
     *         - @ref mpicxx::initialized(), @ref mpicxx::active(), @ref mpicxx::finalized()
     *         - other mpicxx functions that don't wrap MPI calls
     *
     * @assert_precondition{ If the MPI environment has already been finalized. }
     *
     * @calls{ int MPI_Finalize(void);    // exactly once }
     */
    inline void finalize() {
        MPICXX_ASSERT_PRECONDITION(!finalized(), "MPI environment already finalized!");

        MPI_Finalize();
    }

    // TODO 2020-02-26 17:49 marcel: change to the mpicxx equivalent
    /**
     * @brief Attempts to abort all tasks in the communication group of @p comm.
     * @details An MPI implementation is **not** required to be able to abort only a subset of processes of
     *          [*MPI_COMM_WORLD*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm).
     * @param[in] error_code the error code (not necessarily returned from the executable)
     * @param[in] comm the communicator whom's tasks to abort
     *
     * @calls{ int MPI_Abort(MPI_Comm comm, int errorcode);    // exactly once }
     */
    inline void abort(const error_code error_code = error_code::success, MPI_Comm comm = MPI_COMM_WORLD) {
        MPI_Abort(comm, error_code.value());
    }

    namespace detail {
        // callback functions type
        using atfinalize_callback_t = void (*)(void);
        // registered callback functions
        inline std::array<atfinalize_callback_t, MPICXX_MAX_NUMBER_OF_ATFINALIZE_CALLBACKS> atfinalize_lookup_callbacks;
        // number of registered callback functions
        inline std::size_t atfinalize_idx = 0;

        /*
         * @brief Calls a specific callback function (previously registered).
         * @param[in] comm not used
         * @param[in] comm_key_val not used
         * @param[in] attribute_val not used
         * @param[in] extra_state not used
         * @return always `0`
         */
        inline int atfinalize_delete_fn([[maybe_unused]] MPI_Comm comm, [[maybe_unused]] int comm_key_val,
                [[maybe_unused]] void* attribute_val, [[maybe_unused]] void* extra_state)
        {
            // invoke handler function
            std::invoke(atfinalize_lookup_callbacks[--atfinalize_idx]);
            return 0;
        }
    }
    /**
     * @brief Registers the callback function @p func (of the type `void (*)void`) to be called directly before
     *        [*MPI_Finalize*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node225.htm).
     * @details Calls all registered functions in reverse order in which they were set. This happens before any other parts of MPI are
     *          affected, i.e. @ref mpicxx::finalized() will return `false` in any of these callback functions.
     *
     *    The maximum number of registrable callback functions can be set via the [`CMake`](https://cmake.org/) configuration flag
     *          *MPICXX_MAX_NUMBER_OF_ATFINALIZE_CALLBACKS* (default: `32`).
     * @param[in] func pointer to a function to be called on normal MPI finalization
     * @return `0` if the registration succeeded, `1` otherwise
     *
     * @pre The callback function pointer @p func **must not** be the `nullptr`.
     * @pre The total number of added callback functions **must not** be greater or equal than *MPICXX_MAX_NUMBER_OF_ATFINALIZE_CALLBACKS*.
     *
     * @assert_precondition{ If @p func is the `nullptr`. \n
     *                       If adding @p func would exceed the size limit. }
     *
     * @calls{
     * int MPI_Comm_create_keyval(MPI_Comm_copy_attr_function *comm_copy_attr_fn, MPI_Comm_delete_attr_function *comm_delete_attr_fn, int *comm_keyval, void *extra_state);    // exactly once
     * int MPI_Comm_set_attr(MPI_Comm comm, int comm_keyval, void *attribute_val);    // exactly once
     * }
     */
    inline int atfinalize(detail::atfinalize_callback_t func) {
        MPICXX_ASSERT_PRECONDITION(func != nullptr, "The callback function cannot be nullptr!");
        MPICXX_ASSERT_PRECONDITION(detail::atfinalize_idx < MPICXX_MAX_NUMBER_OF_ATFINALIZE_CALLBACKS,
                "Maximum number of callback functions ({}) already registered!", MPICXX_MAX_NUMBER_OF_ATFINALIZE_CALLBACKS);

        int comm_keyval;

        // return if the number of registered callbacks exceeds the limit
        if (detail::atfinalize_idx >= MPICXX_MAX_NUMBER_OF_ATFINALIZE_CALLBACKS) return 1;

        // register function
        detail::atfinalize_lookup_callbacks[detail::atfinalize_idx] = func;
        MPI_Comm_create_keyval(MPI_COMM_NULL_COPY_FN, &detail::atfinalize_delete_fn, &comm_keyval, nullptr);
        MPI_Comm_set_attr(MPI_COMM_SELF, comm_keyval, nullptr);
        ++detail::atfinalize_idx;

        return 0;
    }
    ///@}

}

#endif // MPICXX_FINALIZATION_HPP