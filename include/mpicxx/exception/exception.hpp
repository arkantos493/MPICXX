/**
 * @file include/mpicxx/exception/exception.hpp
 * @author Marcel Breyer
 * @date 2020-03-01
 *
 * @brief Contains the base class for all custom exception in the mpicxx namespace.
 * @details This base class is fully standard conformant, i.e. it provides a public **noexcept** copy constructor and copy assignment
 * operator.
 */

#ifndef MPICXX_EXCEPTION_HPP
#define MPICXX_EXCEPTION_HPP

#include <memory>
#include <stdexcept>
#include <string>

#include <fmt/format.h>

#include <mpicxx/detail/source_location.hpp>
#include <mpicxx/detail/concepts.hpp>


namespace mpicxx {

    /**
     * @brief The base class of all exceptions in the mpicxx namespace.
     * @details The @ref mpicxx::detail::source_location class is used to provide more information in case of an exceptional case.
     *
     * It uses a [`std::shared_pointer<std::string>`](https://en.cppreference.com/w/cpp/memory/shared_ptr) for the what message to be able
     * to provide a **noexcept** copy constructor and copy assignment operator (as requested by the C++ standard).
     */
    class exception : public std::exception {
    public:
        /**
         * @brief Construct an exception, i.e. construct the exception location message.
         * @details If the location message couldn't be constructed, the respective exception gets directly catched to prevent a call to
         * [`std::terminate`](https://en.cppreference.com/w/cpp/error/terminate) during stack unwinding.
         * @param loc the source location information
         */
        exception(const detail::source_location& loc = detail::source_location::current()) : loc_(loc) {
            try {
                // try to create a detailed error message containing the source location if the thrown exception
                std::string rank = loc.rank().has_value() ? fmt::format(" on rank {}", loc.rank().value()) : "";
                msg_ = std::make_shared<std::string>(
                        fmt::format("Exception thrown{}:\n  in file {}\n  in function {}\n  @ line {}",
                                rank, loc.file_name(), loc.function_name(), loc.line()));
                // // TODO 2020-03-01 19:13 marcel: stack trace ?
            } catch (...) {
                // unable to create source location message
                msg_ = nullptr;
            }
        }

        /**
         * @brief Virtual destructor to be able to correctly derive from this class.
         */
        virtual ~exception() = default;

        /**
         * @brief Returns the exception message.
         * @details If no exception message could be created (e.g. due to exceptions in the
         * [`std::string` constructor](https://en.cppreference.com/w/cpp/string/basic_string/basic_string)) a static message will be
         * returned.
         * @return the exception's what message
         */
        [[nodiscard]] virtual const char* what() const noexcept override {
            // check whether any specific error message has been created
            static constexpr char error_what[] = "Couldn't create exception message!";
            return msg_ != nullptr ? msg_->c_str() : error_what;
        }

        /**
         * @brief Returns the created source location information.
         * @return the source location information
         */
        [[nodiscard]] const detail::source_location& location() const noexcept { return loc_; }

    protected:
        /**
         * @brief Tries to prepend @p msg to the current source location information.
         * @details There are two possible cases:
         * 1. the source location message was successfully created:
         * try to prepend @p msg; if an exception is thrown, this function has no effect and only the source location message is stored
         * 2. the source location message was **not** successfully created:
         * try to store @p msg; if an exception is thrown, this function has no effect and nothing is stored
         * @param msg the message to prepend
         */
        void prepend_to_what_message(detail::string auto&& msg) {
            try {
                if (msg_ != nullptr) {
                    // successfully created source location message
                    // -> try to prepend the derived class message (msg)
                    msg_->insert(0, std::forward<decltype(msg)>(msg));
                } else {
                    // exception thrown while creating source location message
                    // try to only create the derived class message (msg)
                    msg_ = std::make_shared<std::string>(std::forward<decltype(msg)>(msg));
                }
            } catch (...) {
                // do nothing if the derived class message couldn't be prepended to the message
            }
        }
        /**
         * @brief Tries to append @p msg to the current source location information.
         * @details There are two possible cases:
         * 1. the source location message was successfully created:
         * try to append @p msg; if an exception is thrown, this function has no effect and only the source location message is stored
         * 2. the source location message was **not** successfully created:
         * try to store @p msg; if an exception is thrown, this function has no effect and nothing is stored
         * @param msg the message to append
         */
        void append_to_what_message(detail::string auto&& msg) {
            try {
                if (msg_ != nullptr) {
                    // successfully created source location message
                    // -> try to append the derived class message (msg)
                    msg_->append(std::forward<decltype(msg)>(msg));
                } else {
                    // exception thrown while creating source location message
                    // try to only create the derived class message (msg)
                    msg_ = std::make_shared<std::string>(std::forward<decltype(msg)>(msg));
                }
            } catch (...) {
                // do nothing if the derived class message couldn't be appended to the message
            }
        }

    private:
        std::shared_ptr<std::string> msg_;
        const detail::source_location loc_;
    };

}

#endif // MPICXX_EXCEPTION_HPP
