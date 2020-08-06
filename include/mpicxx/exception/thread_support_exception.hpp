/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-21
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Implements the exception which gets thrown if the required level of thread support can't be satisfied.
 */

#ifndef MPICXX_THREAD_SUPPORT_EXCEPTION_HPP
#define MPICXX_THREAD_SUPPORT_EXCEPTION_HPP

#include <mpicxx/detail/source_location.hpp>
#include <mpicxx/exception/exception.hpp>
#include <mpicxx/startup/thread_support.hpp>

#include <fmt/color.h>
#include <fmt/format.h>

namespace mpicxx {

    /**
     * @brief An exception which is thrown if the required level of thread support can't be satisfied.
     */
    class thread_support_not_satisfied final : public exception {
    public:
        /**
         * @brief Construct a new exception, i.e. tries to create a detailed exception message about the level of thread support and tries
         *        to add this message to the base class's @ref mpicxx::detail::source_location message.
         * @details If an exception is thrown during construction, no additional message will be prepended.
         * @param[in] required the requested level of thread support
         * @param[in] provided the provided level of thread support
         * @param[in] loc the exception's source location
         */
        thread_support_not_satisfied(const thread_support required, const thread_support provided,
                                     const detail::source_location& loc = detail::source_location::current())
                : exception(loc), required_(required), provided_(provided)
        {
            try {
                // try to create a detailed error message and add it to the base class message
                this->prepend_to_what_message(fmt::format(fmt::emphasis::bold | fmt::fg(fmt::color::red),
                        "Couldn't satisfy required level of thread support: {}\nHighest supported level of thread support:         {}\n\n",
                        required, provided));
            } catch (...) {
                // do nothing if the message couldn't be created
            }
        }

        /**
         * @brief Returns the required level of thread support.
         * @return the required level of thread support
         */
        [[nodiscard]]
        thread_support required() const noexcept { return required_; }
        /**
         * @brief Returns the provided level of thread support.
         * @return the provided level of thread support
         */
        [[nodiscard]]
        thread_support provided() const noexcept { return provided_; }

    private:
        const thread_support required_;
        const thread_support provided_;
    };

}

#endif // MPICXX_THREAD_SUPPORT_EXCEPTION_HPP