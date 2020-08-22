/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-22
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Implements the exception which gets thrown if the provided @ref mpicxx::error_handler_type isn't legal.
 */

#ifndef MPICXX_ERROR_HANDLER_TYPE_EXCEPTION_HPP
#define MPICXX_ERROR_HANDLER_TYPE_EXCEPTION_HPP

#include <mpicxx/detail/source_location.hpp>
#include <mpicxx/error/error_handler_type.hpp>
#include <mpicxx/exception/exception.hpp>

#include <fmt/color.h>
#include <fmt/format.h>

namespace mpicxx {
    // TODO 2020-08-22 20:42 breyerml: name
    /**
     * @brief An exception which is thrown if the provided @ref mpicxx::error_handler_type isn't legal.
     */
    class unset_error_handler_type final : public exception {
    public:
        /**
         * @brief Construct a new exception, i.e. tries to create a detailed exception message about the @ref mpicxx::error_handler_type
         *        and tries to add this message to the base class's @ref mpicxx::detail::source_location message.
         * @details If an exception is thrown during construction, no additional message will be prepended.
         * @param[in] type the provided @ref mpicxx::error_handler_type
         * @param[in] loc the exception's source location
         */
        unset_error_handler_type(const error_handler_type requested, const error_handler_type set,
                                 const detail::source_location& loc = detail::source_location::current())
                : exception(loc), requested_(requested), set_(set)
        {
            try {
                // try to create a detailed error message and add it to the base class message
                this->prepend_to_what_message(fmt::format(fmt::emphasis::bold | fmt::fg(fmt::color::red),
                        "The requested error handler type ({}) hasn't been set for this error handler! Set error handler types are: {}\n\n",
                        requested, set));
            } catch (...) {
                // do nothing if the message couldn't be created
            }
        }

        /**
         * @brief Returns the requested @ref mpicxx::error_handler_type.
         * @return the requested @ref mpicxx::error_handler_type
         */
        [[nodiscard]]
        error_handler_type requested_type() const noexcept { return requested_; }
        /**
         * @brief Returns the set @ref mpicxx::error_handler_type.
         * @return the set @ref mpicxx::error_handler_type
         */
         [[nodiscard]]
         error_handler_type set_type() const noexcept { return set_; }

    private:
        const error_handler_type requested_;
        const error_handler_type set_;
    };

}

#endif // MPICXX_ERROR_HANDLER_TYPE_EXCEPTION_HPP