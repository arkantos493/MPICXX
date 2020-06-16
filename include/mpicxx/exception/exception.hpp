/**
 * @file include/mpicxx/exception/exception.hpp
 * @author Marcel Breyer
 * @date 2020-06-17
 *
 * @brief Contains the base class for all custom exception in the mpicxx namespace.
 * @details This base class is fully standard conformant, i.e. it provides a public **noexcept** copy constructor and copy assignment
 *          operator.
 */

#ifndef MPICXX_EXCEPTION_HPP
#define MPICXX_EXCEPTION_HPP

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <mpicxx/detail/source_location.hpp>
#include <mpicxx/detail/concepts.hpp>


namespace mpicxx {

    /**
     * @brief The base class of all exceptions in the mpicxx namespace.
     * @details The @ref mpicxx::detail::source_location class is used to provide more information in case of an exceptional case.
     *
     *          It uses a [`std::shared_pointer<std::string>`](https://en.cppreference.com/w/cpp/memory/shared_ptr) for the what message
     *          to be able to provide a **noexcept** copy constructor and copy assignment operator (as requested by the C++ standard).
     */
    class exception : public std::exception {
    public:
        /**
         * @brief Construct an exception, i.e. construct the exception location message.
         * @details If the location message couldn't be constructed, the respective exception gets directly catched to prevent a call to
         *          [`std::terminate`](https://en.cppreference.com/w/cpp/error/terminate) during stack unwinding.
         * @param[in] loc the source location information
         * @param[in] use_stack_trace if `true` a detailed stack trace is included in the exception message (if supported, see
         *                            @ref mpicxx::detail::source_location::stack_trace(std::ostream&, const int))
         */
        exception(const detail::source_location& loc = detail::source_location::current(), const bool use_stack_trace = true) : loc_(loc) {
            try {
                // try to create a detailed error message containing the source location of the thrown exception
                std::stringstream ss;
                ss << "Exception thrown";
                // if we are in an active MPI environment, print current rank
                if (loc.rank().has_value()) {
                    ss << " on rank " << loc.rank().value();
                }
                ss << "\n"
                   << "  in file " << loc.file_name() << "\n"
                   << "  in function '" << loc.function_name() << "'\n"
                   << "  @ line " << loc.line() << "\n\n";

                // write stacktrace into the string stream if requested (default = true)
                if (use_stack_trace) {
                    loc.stack_trace(ss);
                }

                msg_ = std::make_shared<std::string>(std::move(ss.str()));
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
         *          [`std::string` constructor](https://en.cppreference.com/w/cpp/string/basic_string/basic_string)) a static message
         *          will be returned.
         * @return the exception's what message
         */
        [[nodiscard]]
        virtual const char* what() const noexcept override {
            // check whether any specific error message has been created
            static constexpr char error_what[] = "Couldn't create exception message!";
            return msg_ != nullptr ? msg_->c_str() : error_what;
        }

        /**
         * @brief Returns the created @ref detail::source_location information.
         * @return the @ref detail::source_location information
         */
        [[nodiscard]]
        const detail::source_location& location() const noexcept { return loc_; }

    protected:
        /**
         * @brief Tries to prepend @p msg to the current @ref detail::source_location information.
         * @details There are two possible cases:
         *          -# the @ref detail::source_location message was successfully created:
         *             try to prepend @p msg; if an exception is thrown, this function has no effect and only the
         *             @ref detail::source_location  message is stored
         *          -# the @ref detail::source_location message was **not** successfully created:
         *             try to store @p msg; if an exception is thrown, this function has no effect and nothing is stored
         * @tparam T must meet the @ref detail::is_string requirements.
         * @param[in] msg_to_prepend the message to prepend
         */
        template <detail::is_string T>
        void prepend_to_what_message(T&& msg_to_prepend) {
            try {
                if (msg_ != nullptr) {
                    // successfully created source location message
                    // -> try to prepend the derived class message (msg_to_prepend)
                    msg_->insert(0, std::forward<T>(msg_to_prepend));
                } else {
                    // exception thrown while creating source location message
                    // try to only create the derived class message (msg_to_prepend)
                    msg_ = std::make_shared<std::string>(std::forward<T>(msg_to_prepend));
                }
            } catch (...) {
                // do nothing if the derived class message couldn't be prepended to the message
            }
        }
        /**
         * @brief Tries to append @p msg to the current @ref detail::source_location information.
         * @details There are two possible cases:
         *          -# the @ref detail::source_location message was successfully created:
         *             try to append @p msg; if an exception is thrown, this function has no effect and only the
         *             @ref detail::source_location is stored
         *          -# the @ref detail::source_location message was **not** successfully created:
         *             try to store @p msg; if an exception is thrown, this function has no effect and nothing is stored
         * @tparam T must meet the @ref detail::is_string requirements.
         * @param[in] msg_to_append the message to append
         */
        template <detail::is_string T>
        void append_to_what_message(T&& msg_to_append) {
            try {
                if (msg_ != nullptr) {
                    // successfully created source location message
                    // -> try to append the derived class message (msg_to_append)
                    msg_->append(std::forward<T>(msg_to_append));
                } else {
                    // exception thrown while creating source location message
                    // try to only create the derived class message (msg_to_append)
                    msg_ = std::make_shared<std::string>(std::forward<T>(msg_to_append));
                }
            } catch (...) {
                // do nothing if the derived class message couldn't be appended to the message
            }
        }

    private:
        std::shared_ptr<std::string> msg_;
        const detail::source_location loc_;
    };


/**
 * @brief Macro to be able to easily use @ref PRETTY_FUNC_NAME__ for a better source location message.
 */
#define MPICXX_THROW_WITH_PRETTY_LOCATION(except, ...) \
    throw except(__VA_ARGS__ __VA_OPT__(,) mpicxx::detail::source_location::current(PRETTY_FUNC_NAME__));

}

#endif // MPICXX_EXCEPTION_HPP
