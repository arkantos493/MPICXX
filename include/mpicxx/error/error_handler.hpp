/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-19
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Defines the rror handler class which can be attached to MPI communicators, files or windows.
 */

#ifndef MPICXX_ERROR_HANDLER_HPP
#define MPICXX_ERROR_HANDLER_HPP

#include <mpicxx/error/error.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <mpi.h>

#include <array>
#include <functional>
#include <istream>
#include <ostream>
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
    public:
        /**
         * @brief Enum class for the different types of error handlers provided by MPI.
         */
        enum class type {
            /** error handler type for communicators */
            comm = 1 << 0,
            /** error handler type for files*/
            file = 1 << 1,
            /** error handler type for windows */
            win  = 1 << 2
        };

    private:

        // befriend factory functions
        template <auto Func>
        friend error_handler make_error_handler(const error_handler::type);
        friend error_handler make_error_handler(MPI_Comm_errhandler_function);
        friend error_handler make_error_handler(MPI_File_errhandler_function);
        friend error_handler make_error_handler(MPI_Win_errhandler_function);

    public:
        /// @name mpicxx::error_handler::type bitwise operations
        friend error_handler::type operator~(const error_handler::type eht) {
            return static_cast<error_handler::type>( ~static_cast<std::underlying_type_t<error_handler::type>>(eht) );
        }
        friend error_handler::type operator|(const error_handler::type lhs, const error_handler::type rhs) {
            using type = std::underlying_type_t<error_handler::type>;
            return static_cast<error_handler::type>( static_cast<type>(lhs) | static_cast<type>(rhs) );
        }
        friend error_handler::type operator&(const error_handler::type lhs, const error_handler::type rhs) {
            using type = std::underlying_type_t<error_handler::type>;
            return static_cast<error_handler::type>( static_cast<type>(lhs) & static_cast<type>(rhs) );
        }
        friend error_handler::type operator^(const error_handler::type lhs, const error_handler::type rhs) {
            using type = std::underlying_type_t<error_handler::type>;
            return static_cast<error_handler::type>( static_cast<type>(lhs) ^ static_cast<type>(rhs) );
        }
        friend error_handler::type& operator|=(error_handler::type& lhs, const error_handler::type rhs) {
            using type = std::underlying_type_t<error_handler::type>;
            lhs = static_cast<error_handler::type>( static_cast<type>(lhs) | static_cast<type>(rhs) );
            return lhs;
        }
        friend error_handler::type& operator&=(error_handler::type& lhs, const error_handler::type rhs) {
            using type = std::underlying_type_t<error_handler::type>;
            lhs = static_cast<error_handler::type>( static_cast<type>(lhs) & static_cast<type>(rhs) );
            return lhs;
        }
        friend error_handler::type& operator^=(error_handler::type& lhs, const error_handler::type rhs) {
            using type = std::underlying_type_t<error_handler::type>;
            lhs = static_cast<error_handler::type>( static_cast<type>(lhs) ^ static_cast<type>(rhs) );
            return lhs;
        }
        ///@}

        error_handler(const error_handler&) = delete;
        error_handler(error_handler&& other) noexcept : handler_(std::move(other.handler_)), type_(std::move(other.type_)) {
            other.type_ = static_cast<error_handler::type>(0);
        }
        error_handler& operator=(const error_handler&) = delete;
        error_handler& operator=(error_handler&& rhs) noexcept {
            // destruct this
            this->delete_mpi_errhandlers();

            // transfer ownership
            handler_ = std::move(rhs.handler_);
            type_ = std::move(rhs.type_);
            // set rhs to the moved from state
            rhs.type_ = static_cast<error_handler::type>(0);
            return *this;
        }

        ~error_handler() {
            this->delete_mpi_errhandlers();
        }


//    private:
        error_handler() : type_(static_cast<error_handler::type>(0)) { }

        void delete_mpi_errhandlers() {
            if ((type_ & error_handler::type::comm) == error_handler::type::comm) {
                MPI_Errhandler_free(&handler_[0]);
            }
            if ((type_ & error_handler::type::file) == error_handler::type::file) {
                MPI_Errhandler_free(&handler_[1]);
            }
            if ((type_ & error_handler::type::win)  == error_handler::type::win) {
                MPI_Errhandler_free(&handler_[2]);
            }
        }

        void add_comm_error_handler(MPI_Comm_errhandler_function func) {
            type_ |= error_handler::type::comm;
            MPI_Comm_create_errhandler(func, &handler_[0]);
        }
        void add_file_error_handler(MPI_File_errhandler_function func) {
            type_ |= error_handler::type::file;
            MPI_File_create_errhandler(func, &handler_[1]);
        }
        void add_win_error_handler(MPI_Win_errhandler_function func) {
            type_ |= error_handler::type::win;
            MPI_Win_create_errhandler(func, &handler_[2]);
        }

        std::array<MPI_Errhandler, 3> handler_;
        error_handler::type type_;
    };


    /// @name mpicxx::error_handler::type conversion functions
    ///@{
    /**
     * @brief Stream-insertion operator overload for the @ref mpicxx::error_handler::type enum class.
     * @param[inout] out an output stream
     * @param[in] eht the enum class value
     * @return the output stream
     */
    inline std::ostream& operator<<(std::ostream& out, const error_handler::type eht) {
        switch (eht) {
            case error_handler::type::comm:
                out << "COMM";
                break;
            case error_handler::type::file:
                out << "FILE";
                break;
            case error_handler::type::win:
                out << "WIN";
                break;
        }
        return out;
    }
    /**
     * @brief Overload of the @ref mpicxx::to_string(T&&) function for the @ref mpicxx::error_handler::type enum class.
     * @param[in] eht the enum class value
     * @return the [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) representation of the enum class value
     * @nodiscard
     */
    [[nodiscard]]
    inline std::string to_string(const error_handler::type eht) {
        return fmt::format("{}", eht);
    }

    /**
     * @brief Converts the given string to the respective @ref mpicxx::error_handler::type value.
     * @details Expects the string value to be one of `"COMM"`, `"FILE"` or `"WIN"`.
     * @param[in] sv the enum value represented as a string
     * @return the @ref mpicxx::error_handler::type representation of @p sv
     * @nodiscard
     *
     * @throws std::invalid_argument if the given value can't be converted to a @ref mpicxx::error_handler::type value
     */
    [[nodiscard]]
    inline error_handler::type error_handler_type_from_string(const std::string_view sv) {
        if (sv.compare("COMM") == 0) {
            return error_handler::type::comm;
        } else if (sv.compare("FILE") == 0) {
            return error_handler::type::file;
        } else if (sv.compare("WIN") == 0) {
            return error_handler::type::win;
        } else {
            throw std::invalid_argument(fmt::format("Can't convert \"{}\" to mpicxx::error_handler::type!", sv));
        }
    }
    /**
     * @brief Stream-extraction operator overload for the @ref mpicxx::error_handler::type enum class.
     * @details Sets the [`std::ios::failbit`](https://en.cppreference.com/w/cpp/io/ios_base/iostate) if the given value can't be converted
     *          to a @ref mpicxx::error_handler::type value.
     * @param[inout] in an input stream
     * @param[out] eht the enum class
     * @return the input stream
     */
    inline std::istream& operator>>(std::istream& in, error_handler::type& eht) {
        try {
            std::string str;
            in >> str;
            eht = error_handler_type_from_string(str);
        } catch (const std::exception&) {
            in.setstate(std::ios::failbit);
        }
        return in;
    }
    ///@}


    /// @name mpicxx::error_handler factory functions
    ///@{
    template <auto Func>
    error_handler make_error_handler(const error_handler::type type) {
        error_handler handler;
        if ((type & error_handler::type::comm) == error_handler::type::comm) {
            handler.add_comm_error_handler(detail::wrap_comm_error_handler_function<Func>);
        }
        if ((type & error_handler::type::file) == error_handler::type::file) {
            handler.add_file_error_handler(detail::wrap_file_error_handler_function<Func>);
        }
        if ((type & error_handler::type::win) == error_handler::type::win) {
            handler.add_win_error_handler(detail::wrap_win_error_handler_function<Func>);
        }
        return handler;
    }
    error_handler make_error_handler(MPI_Comm_errhandler_function func) {
        error_handler handler;
        handler.add_comm_error_handler(func);
        return handler;
    }
    error_handler make_error_handler(MPI_File_errhandler_function func) {
        error_handler handler;
        handler.add_file_error_handler(func);
        return handler;
    }
    error_handler make_error_handler(MPI_Win_errhandler_function func) {
        error_handler handler;
        handler.add_win_error_handler(func);
        return handler;
    }
    ///@}

}

#endif // MPICXX_ERROR_HANDLER_HPP
