/**
 * @file include/mpicxx/startup/thread_support_exception.hpp
 * @author Marcel Breyer
 * @date 2020-02-20
 *
 * @brief Contains an exception which is thrown in @ref mpicxx::initialize(const thread_support) and
 * @ref mpicxx::initialize(int&, char**, const thread_support) if the required level of thread support can't be satisfied.
 */

#ifndef MPICXX_THREAD_SUPPORT_EXCEPTION_HPP
#define MPICXX_THREAD_SUPPORT_EXCEPTION_HPP

#include <stdexcept>
#include <string>

#include <fmt/format.h>

#include <mpicxx/startup/thread_support.hpp>


namespace mpicxx {

    /**
     * @brief An exception which is thrown in @ref mpicxx::initialize(const thread_support) and
     * @ref mpicxx::initialize(int&, char**, const thread_support) if the required level of thread support can't be satisfied.
     */
    class thread_support_not_satisfied : public std::exception {
    public:
        /**
         * @brief Construct a new exception.
         * @brief Constructs the error message.
         * @param required level of thread support (but not satisfied)
         * @param provided level of thread support
         */
        thread_support_not_satisfied(const thread_support required, const thread_support provided)
                : required_(required), provided_(provided) {
            msg_ =  fmt::format("Couldn't satisfy required level of thread support: {}\n", required);
            msg_ += fmt::format("Highest supported level of thread support:         {}"  , provided);
        }

        /**
         * @brief Returns the error message of this exception.
         * @return the exception message
         */
        [[nodiscard]] virtual const char* what() const noexcept { return msg_.c_str(); }

        /**
         * @brief Returns the required, but not satisfied, thread support.
         * @return required thread_support
         */
        [[nodiscard]] thread_support required() const noexcept { return required_; }
        /**
         * @brief Returns the provided thread support.
         * @return provided provided
         */
        [[nodiscard]] thread_support provided() const noexcept { return provided_; }

    private:
        std::string msg_;
        const thread_support required_;
        const thread_support provided_;
    };

}

#endif // MPICXX_THREAD_SUPPORT_EXCEPTION_HPP
