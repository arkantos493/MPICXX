/**
 * @file include/mpicxx/startup/mpicxx_main.hpp
 * @author Marcel Breyer
 * @date 2020-03-15
 *
 * @brief Implements a save way to setup and teardown the MPI environment, e.g. without the possibility to forget a call to *MPI_Init* or
 * *MPI_Finalize*.
 */


#ifndef MPICXX_MPICXX_MAIN_HPP
#define MPICXX_MPICXX_MAIN_HPP

#include <mpicxx/detail/concepts.hpp>
#include <mpicxx/startup/finalize.hpp>
#include <mpicxx/startup/init.hpp>

#include <functional>
#include <iostream>


namespace mpicxx {

    /**
     * @brief Correctly setup and teardown the MPI environment while executing the code given by @p func.
     * @details This function performs the following tasks in the given order:
     * 1. call @ref mpicxx::init()
     * 2. invoke the function represented by @p func (forwarding all additional parameters)
     * 3. call @ref mpicxx::finalize()
     *
     * Example:
     * @snippet examples/startup/init_and_finalize.cpp normal version without args and thread support
     * is the same as:
     * @snippet examples/startup/mpicxx_main.cpp mpicxx_main version without args and thread support
     * @tparam FuncPtr a callable fulfilling the @ref detail::main_pointer requirements
     * @tparam Args a parameter pack for the additional arguments
     * @param[in] func any callable holding the main code of the application
     * @param[in] args the additional arguments forwarded to the user defined function @p func
     * @return the result of the invocation of @p FuncPtr
     */
    template <typename FuncPtr, typename... Args>
    requires detail::main_pointer<FuncPtr, Args...>
    int main(FuncPtr func, Args&&... args) {
        init();

        int ret = std::invoke(func, std::forward<Args>(args)...);

        finalize();
        return ret;
    }
    /**
     * @brief Correctly setup and teardown the MPI environment while executing the code given by @p func.
     * @details This function performs the following tasks in the given order:
     * 1. call @ref mpicxx::init(int argc, char** argv)
     * 2. invoke the function represented by @p func (forwarding all additional parameters)
     * 3. call @ref mpicxx::finalize()
     *
     * Example:
     * @snippet examples/startup/init_and_finalize.cpp normal version with args and without thread support
     * is the same as:
     * @snippet examples/startup/mpicxx_main.cpp mpicxx_main version with args and without thread support
     * @tparam FuncPtr a callable fulfilling the @ref detail::main_args_pointer requirements
     * @tparam Args a parameter pack for the additional arguments
     * @param[in] func any callable holding the main code of the application
     * @param[inout] argc the number of command line parameters
     * @param[inout] argv the command line parameters
     * @param[in] args the additional arguments forwarded to the user defined function @p func
     * @return the result of the invocation of @p FuncPtr
     */
    template <typename FuncPtr, typename... Args>
    requires detail::main_args_pointer<FuncPtr, Args...>
    int main(FuncPtr func, int argc, char** argv, Args&&... args) {
        init(argc, argv);

        int ret = std::invoke(func, argc, argv, std::forward<Args>(args)...);

        finalize();
        return ret;
    }

    /**
     * @brief Correctly setup and teardown the MPI environment with the required level of thread support while executing the code given by
     * @p func. If the required level of thread support couldn't be satisfied, the function returns immediately with return code -1.
     * @details This function performs the following tasks in the given order:
     * 1. call @ref mpicxx::init(const thread_support)
     * 2. invoke the function represented by @p func (forwarding all additional parameters)
     * 3. call @ref mpicxx::finalize()
     *
     * Example:
     * @snippet examples/startup/init_and_finalize.cpp normal version without args and with thread support
     * is nearly the same as (except for the return value):
     * @snippet examples/startup/mpicxx_main.cpp mpicxx_main version without args and with thread support
     * @tparam FuncPtr a callable fulfilling the @ref detail::main_pointer requirements
     * @tparam Args a parameter pack for the additional arguments
     * @param[in] func any callable holding the main code of the application
     * @param[in] required the required level of thread support
     * @param[in] args the additional arguments forwarded to the user defined function @p func
     * @return the result of the invocation of @p FuncPtr or -1 if the required level level of thread support couldn't be satisfied
     */
    template <typename FuncPtr, typename... Args>
    requires detail::main_pointer<FuncPtr, Args...>
    int main(FuncPtr func, const thread_support required, Args&&... args) {
        int ret = -1;
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
     * @p func. If the required level of thread support couldn't be satisfied, the function returns immediately with return code -1.
     * @details This function performs the following tasks in the given order:
     * 1. call @ref mpicxx::init(const thread_support)
     * 2. invoke the function represented by @p func (forwarding all additional parameters)
     * 3. call @ref mpicxx::finalize()
     *
     * Example:
     * @snippet examples/startup/init_and_finalize.cpp normal version with args and thread support
     * is nearly the same as (except for the return value):
     * @snippet examples/startup/mpicxx_main.cpp mpicxx_main version with args and thread support
     * @tparam FuncPtr a callable fulfilling the @ref detail::main_pointer requirements
     * @tparam Args a parameter pack for the additional arguments
     * @param[in] func any callable holding the main code of the application
     * @param[inout] argc the number of command line parameters
     * @param[inout] argv the command line parameters
     * @param[in] required the required level of thread support
     * @param[in] args the additional arguments forwarded to the user defined function @p func
     * @return the result of the invocation of @p FuncPtr or -1 if the required level level of thread support couldn't be satisfied
     */
    template <typename FuncPtr, typename... Args>
    requires detail::main_args_pointer<FuncPtr, Args...>
    int main(FuncPtr func, int argc, char** argv, const thread_support required, Args&&... args) {
        int ret = -1;
        try {
            init(argc, argv, required);
            ret = std::invoke(func, argc, argv, std::forward<Args>(args)...);
        } catch (const mpicxx::thread_support_not_satisfied& e) {
            std::cerr << e.what() << std::endl;
        }

        finalize();
        return ret;
    }

}


#endif // MPICXX_MPICXX_MAIN_HPP
