/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-22
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Defines the error handler class which can be attached to MPI communicators, files or windows.
 */

#ifndef MPICXX_ERROR_HANDLER_HPP
#define MPICXX_ERROR_HANDLER_HPP

#include <mpicxx/detail/utility.hpp>
#include <mpicxx/error/error.hpp>
#include <mpicxx/error/error_handler_type.hpp>
#include <mpicxx/exception/exception.hpp>
#include <mpicxx/exception/error_handler_type_exception.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <mpi.h>

#include <array>
#include <functional>
#include <stdexcept>
#include <type_traits>


#define MPICXX_DETAIL_CREATE_ERROR_HANDLER_WRAPPER_FUNCTION(name, type, mpicxx_type)                            \
template <auto Func>                                                                                            \
void wrap_##name##_error_handler_function( type* handler_type, int* errcode, ...) {                             \
    if constexpr (std::is_invocable_v<decltype(Func), mpicxx_type, mpicxx::error_code>) {                       \
        std::invoke(Func, *handler_type, *errcode);                                                             \
    } else if constexpr (std::is_invocable_v<decltype(Func), mpicxx_type>) {                                    \
        std::invoke(Func, *handler_type);                                                                       \
    } else if constexpr (std::is_invocable_v<decltype(Func), mpicxx::error_code>) {                             \
        std::invoke(Func, *errcode);                                                                            \
    } else if constexpr (std::is_invocable_v<decltype(Func)>) {                                                 \
        std::invoke(Func);                                                                                      \
    } else {                                                                                                    \
        static_assert(!std::is_same_v<decltype(Func), decltype(Func)>, "Illegal callback function signature!"); \
    }                                                                                                           \
}                                                                                                               \

namespace mpicxx {

    namespace detail {
        // create wrapping functions
        MPICXX_DETAIL_CREATE_ERROR_HANDLER_WRAPPER_FUNCTION(comm, MPI_Comm, MPI_Comm)
        MPICXX_DETAIL_CREATE_ERROR_HANDLER_WRAPPER_FUNCTION(file, MPI_File, MPI_File)
        MPICXX_DETAIL_CREATE_ERROR_HANDLER_WRAPPER_FUNCTION(win,  MPI_Win,  MPI_Win)
    }

    class error_handler {
        // befriend factory functions
        template <auto Func>
        friend error_handler make_error_handler(const error_handler_type);
        friend error_handler make_error_handler(MPI_Comm_errhandler_function);
        friend error_handler make_error_handler(MPI_File_errhandler_function);
        friend error_handler make_error_handler(MPI_Win_errhandler_function);

    public:
        error_handler(const error_handler&) = delete;
        error_handler(error_handler&& other) noexcept : handler_(std::move(other.handler_)), type_(std::move(other.type_)) {
            other.type_ = static_cast<error_handler_type>(0);
        }
        error_handler& operator=(const error_handler&) = delete;
        error_handler& operator=(error_handler&& rhs) noexcept {
            // destruct this
            this->delete_mpi_errhandlers();

            // transfer ownership
            handler_ = std::move(rhs.handler_);
            type_ = std::move(rhs.type_);
            // set rhs to the moved from state
            rhs.type_ = static_cast<error_handler_type>(0);
            return *this;
        }
        ~error_handler() {
            this->delete_mpi_errhandlers();
        }



    private:
        error_handler() : type_(static_cast<error_handler_type>(0)) { }

        void delete_mpi_errhandlers() {
            if (detail::is_type_set(type_, error_handler_type::comm)) {
                MPI_Errhandler_free(&handler_[0]);
            }
            if (detail::is_type_set(type_, error_handler_type::file)) {
                MPI_Errhandler_free(&handler_[1]);
            }
            if (detail::is_type_set(type_, error_handler_type::win)) {
                MPI_Errhandler_free(&handler_[2]);
            }
        }


        void add_comm_error_handler(MPI_Comm_errhandler_function func) {
            type_ |= error_handler_type::comm;
            MPI_Comm_create_errhandler(func, &handler_[0]);
        }
        void add_file_error_handler(MPI_File_errhandler_function func) {
            type_ |= error_handler_type::file;
            MPI_File_create_errhandler(func, &handler_[1]);
        }
        void add_win_error_handler(MPI_Win_errhandler_function func) {
            type_ |= error_handler_type::win;
            MPI_Win_create_errhandler(func, &handler_[2]);
        }


        std::array<MPI_Errhandler, 3> handler_;
        error_handler_type type_;
    };
    

    /// @name mpicxx::error_handler factory functions
    ///@{
    template <auto Func>
    [[nodiscard]]
    error_handler make_error_handler(const error_handler_type type) {
        error_handler handler;
        if (detail::is_type_set(type, error_handler_type::comm)) {
            handler.add_comm_error_handler(detail::wrap_comm_error_handler_function<Func>);
        }
        if (detail::is_type_set(type, error_handler_type::file)) {
            handler.add_file_error_handler(detail::wrap_file_error_handler_function<Func>);
        }
        if (detail::is_type_set(type, error_handler_type::win)) {
            handler.add_win_error_handler(detail::wrap_win_error_handler_function<Func>);
        }
        return handler;
    }
    [[nodiscard]]
    error_handler make_error_handler(MPI_Comm_errhandler_function func) {
        error_handler handler;
        handler.add_comm_error_handler(func);
        return handler;
    }
    [[nodiscard]]
    error_handler make_error_handler(MPI_File_errhandler_function func) {
        error_handler handler;
        handler.add_file_error_handler(func);
        return handler;
    }
    [[nodiscard]]
    error_handler make_error_handler(MPI_Win_errhandler_function func) {
        error_handler handler;
        handler.add_win_error_handler(func);
        return handler;
    }
    ///@}

}

#endif // MPICXX_ERROR_HANDLER_HPP
