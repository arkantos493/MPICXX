/**
 * @file 
 * @author Marcel Breyer
 * @date 2020-07-19
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Implements a save way to setup and teardown the MPI environment, e.g. without the possibility to forget a call to
 *        [*MPI_Init*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node225.htm) or
 *        [*MPI_Finalize*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node225.htm).
 */

#ifndef MPICXX_MPICXX_MAIN_HPP
#define MPICXX_MPICXX_MAIN_HPP

#include <mpicxx/detail/concepts.hpp>
#include <mpicxx/startup/finalize.hpp>
#include <mpicxx/startup/init.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>

namespace mpicxx {

    /// @name automatic initialization and finalization of the MPI environment
    ///@{
    /**
     * @brief Correctly setup and teardown the MPI environment while executing the code given by @p func.
     * @details This function performs the following tasks in the given order:
     *          1. call @ref mpicxx::init()
     *          2. invoke the function represented by @p func (forwarding all additional parameters)
     *          3. call @ref mpicxx::finalize()
     *
     *          Example:
     *          @snippet examples/startup/init_and_finalize.cpp normal version without args and thread support
     *          is the same as:
     *          @snippet examples/startup/mpicxx_main.cpp mpicxx_main version without args and thread support
     * @tparam FuncPtr must meet the @ref mpicxx::detail::is_main_pointer requirements
     * @tparam Args a parameter pack representing the additional (optional) parameters
     * @param[in] func a callable holding the main code of the application
     * @param[in] args the additional parameters forwarded to the user defined callable @p func
     * @return the result of the invocation of @p func
     *
     * @pre The MPI environment **must not** be initialized.
     * @pre The MPI environment **must not** be finalized.
     * @invariant **Any** mpicxx function an be called inside the provided @p func callable
     *            (except initialization or finalization functions).
     * @post The MPI environment has been finalized, i.e. it's illegal to call any mpicxx function.
     *       The only exceptions are:
     *         - @ref mpicxx::version::mpi_version(), @ref mpicxx::version::mpi_version_major(), @ref mpicxx::version::mpi_version_minor()
     *         - @ref mpicxx::version::mpi_library_version(), @ref mpicxx::version::mpi_library_name()
     *         - @ref mpicxx::initialized(), @ref mpicxx::active(), @ref mpicxx::finalized()
     *         - other mpicxx functions that don't wrap MPI calls
     *
     * @assert_precondition{ If the MPI environment has already been initialized. \n
     *                       If the MPI environment has already been finalized. }
     *
     * @calls{
     * int MPI_Init(int *argc, char ***argv);    // exactly once
     * int MPI_Finalize();                       // exactly once
     * }
     */
    template <typename FuncPtr, typename... Args>
    inline int main(FuncPtr func, Args&&... args) requires detail::is_main_pointer<FuncPtr, Args...> {
        init();

        int ret = std::invoke(func, std::forward<Args>(args)...);

        finalize();
        return ret;
    }
    /**
     * @brief Correctly setup and teardown the MPI environment while executing the code given by @p func.
     * @details This function performs the following tasks in the given order:
     *          1. call @ref mpicxx::init(int& argc, char** argv)
     *          2. invoke the function represented by @p func (forwarding all additional parameters)
     *          3. call @ref mpicxx::finalize()
     *
     *          Example:
     *          @snippet examples/startup/init_and_finalize.cpp normal version with args and without thread support
     *          is the same as:
     *          @snippet examples/startup/mpicxx_main.cpp mpicxx_main version with args and without thread support
     * @tparam FuncPtr must meet the @ref mpicxx::detail::is_main_args_pointer requirements
     * @tparam Args a parameter pack representing the additional (optional) parameters
     * @param[in] func a callable holding the main code of the application
     * @param[inout] argc the number of command line parameters
     * @param[inout] argv the command line parameters
     * @param[in] args the additional parameters forwarded to the user defined callable @p func
     * @return the result of the invocation of @p func
     *
     * @pre The MPI environment **must not** be initialized.
     * @pre The MPI environment **must not** be finalized.
     * @invariant **Any** mpicxx function an be called inside the provided @p func callable
     *            (except initialization or finalization functions).
     * @post The MPI environment has been finalized, i.e. it's illegal to call any mpicxx function.
     *       The only exceptions are:
     *         - @ref mpicxx::version::mpi_version(), @ref mpicxx::version::mpi_version_major(), @ref mpicxx::version::mpi_version_minor()
     *         - @ref mpicxx::version::mpi_library_version(), @ref mpicxx::version::mpi_library_name()
     *         - @ref mpicxx::initialized(), @ref mpicxx::active(), @ref mpicxx::finalized()
     *         - other mpicxx functions that don't wrap MPI calls
     *
     * @assert_precondition{ If the MPI environment has already been initialized. \n
     *                       If the MPI environment has already been finalized. }
     *
     * @calls{
     * int MPI_Init(int *argc, char ***argv);    // exactly once
     * int MPI_Finalize();                       // exactly once
     * }
     */
    template <typename FuncPtr, typename... Args>
    inline int main(FuncPtr func, int& argc, char** argv, Args&&... args) requires detail::is_main_args_pointer<FuncPtr, Args...> {
        init(argc, argv);

        int ret = std::invoke(func, argc, argv, std::forward<Args>(args)...);

        finalize();
        return ret;
    }

    /**
     * @brief Correctly setup and teardown the MPI environment with the required level of thread support while executing the code given by
     *        @p func. \n
     *        If the required level of thread support couldn't be satisfied, the function returns immediately with return code
     *        [`EXIT_FAILURE`](https://en.cppreference.com/w/cpp/utility/program/EXIT_status).
     * @details This function performs the following tasks in the given order:
     *          1. call @ref mpicxx::init(const thread_support)
     *          2. invoke the function represented by @p func (forwarding all additional parameters)
     *          3. call @ref mpicxx::finalize()
     *
     *          Example:
     *          @snippet examples/startup/init_and_finalize.cpp normal version without args and with thread support
     *          is nearly the same as (except for the return value):
     *          @snippet examples/startup/mpicxx_main.cpp mpicxx_main version without args and with thread support
     * @tparam FuncPtr must meet the @ref mpicxx::detail::is_main_pointer requirements
     * @tparam Args a parameter pack representing the additional (optional) parameters
     * @param[in] func a callable holding the main code of the application
     * @param[in] required the required level of thread support
     * @param[in] args the additional parameters forwarded to the user defined callable @p func
     * @return the result of the invocation of @p func or [`EXIT_FAILURE`](https://en.cppreference.com/w/cpp/utility/program/EXIT_status)
     *         if the required level of thread support couldn't be satisfied
     *
     * @pre The MPI environment **must not** be initialized.
     * @pre The MPI environment **must not** be finalized.
     * @invariant **Any** mpicxx function an be called inside the provided @p func callable
     *            (except initialization or finalization functions).
     * @post The MPI environment has been finalized, i.e. it's illegal to call any mpicxx function.
     *       The only exceptions are:
     *         - @ref mpicxx::version::mpi_version(), @ref mpicxx::version::mpi_version_major(), @ref mpicxx::version::mpi_version_minor()
     *         - @ref mpicxx::version::mpi_library_version(), @ref mpicxx::version::mpi_library_name()
     *         - @ref mpicxx::initialized(), @ref mpicxx::active(), @ref mpicxx::finalized()
     *         - other mpicxx functions that don't wrap MPI calls
     *
     * @assert_precondition{ If the MPI environment has already been initialized. \n
     *                       If the MPI environment has already been finalized. }
     *
     * @calls{
     * int MPI_Init_thread(int *argc, char ***argv, int required, int *provided);    // exactly once
     * int MPI_Finalize();                                                           // exactly once
     * }
     */
    template <typename FuncPtr, typename... Args>
    inline int main(FuncPtr func, const thread_support required, Args&&... args) requires detail::is_main_pointer<FuncPtr, Args...> {
        int ret = EXIT_FAILURE;
        try {
            init(required);
            ret = std::invoke(func, std::forward<Args>(args)...);
        } catch (const mpicxx::thread_support_not_satisfied& e) {
            std::cerr << e.what() << std::endl;
        }

        finalize();
        return ret;
    }
    /**
     * @brief Correctly setup and teardown the MPI environment with the required level of thread support while executing the code given by
     *        @p func. \n
     *        If the required level of thread support couldn't be satisfied, the function returns immediately with return code
     *        [`EXIT_FAILURE`](https://en.cppreference.com/w/cpp/utility/program/EXIT_status).
     * @details This function performs the following tasks in the given order:
     *          1. call @ref mpicxx::init(int& argc, char** argv, const thread_support)
     *          2. invoke the function represented by @p func (forwarding all additional parameters)
     *          3. call @ref mpicxx::finalize()
     *
     *          Example:
     *          @snippet examples/startup/init_and_finalize.cpp normal version with args and thread support
     *          is nearly the same as (except for the return value):
     *          @snippet examples/startup/mpicxx_main.cpp mpicxx_main version with args and thread support
     * @tparam FuncPtr must meet the @ref mpicxx::detail::is_main_args_pointer requirements
     * @tparam Args a parameter pack representing the additional (optional) parameters
     * @param[in] func a callable holding the main code of the application
     * @param[inout] argc the number of command line parameters
     * @param[inout] argv the command line parameters
     * @param[in] required the required level of thread support
     * @param[in] args the additional parameters forwarded to the user defined callable @p func
     * @return the result of the invocation of @p func or [`EXIT_FAILURE`](https://en.cppreference.com/w/cpp/utility/program/EXIT_status)
     *         if the required level of thread support couldn't be satisfied
     *
     * @pre The MPI environment **must not** be initialized.
     * @pre The MPI environment **must not** be finalized.
     * @invariant **Any** mpicxx function an be called inside the provided @p func callable
     *            (except initialization or finalization functions).
     * @post The MPI environment has been finalized, i.e. it's illegal to call any mpicxx function.
     *       The only exceptions are:
     *         - @ref mpicxx::version::mpi_version(), @ref mpicxx::version::mpi_version_major(), @ref mpicxx::version::mpi_version_minor()
     *         - @ref mpicxx::version::mpi_library_version(), @ref mpicxx::version::mpi_library_name()
     *         - @ref mpicxx::initialized(), @ref mpicxx::active(), @ref mpicxx::finalized()
     *         - other mpicxx functions that don't wrap MPI calls
     *
     * @assert_precondition{ If the MPI environment has already been initialized. \n
     *                       If the MPI environment has already been finalized. }
     *
     * @calls{
     * int MPI_Init_thread(int *argc, char ***argv, int required, int *provided);    // exactly once
     * int MPI_Finalize();                                                           // exactly once
     * }
     */
    template <typename FuncPtr, typename... Args>
    inline int main(FuncPtr func, int& argc, char** argv, const thread_support required, Args&&... args) requires detail::is_main_args_pointer<FuncPtr, Args...> {
        int ret = EXIT_FAILURE;
        try {
            init(argc, argv, required);
            ret = std::invoke(func, argc, argv, std::forward<Args>(args)...);
        } catch (const mpicxx::thread_support_not_satisfied& e) {
            std::cerr << e.what() << std::endl;
        }

        finalize();
        return ret;
    }
    ///@}

}

#endif // MPICXX_MPICXX_MAIN_HPP