/**
 * @file include/mpicxx/exception/exception.hpp
 * @author Marcel Breyer
 * @date 2020-06-21
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

#include <fmt/format.h>

#include <mpicxx/detail/concepts.hpp>
#include <mpicxx/detail/source_location.hpp>


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
         * @details If the location message couldn't be constructed, the respective exception gets directly caught to prevent a call to
         *          [`std::terminate`](https://en.cppreference.com/w/cpp/error/terminate) during stack unwinding.
         *
         *          Prints a detailed stack trace if *ENABLE_STACK_TRACE* has been enabled during the [`CMake`](https://cmake.org/)
         *          configuration step.
         * @param[in] loc the source location information
         */
        exception(const detail::source_location& loc = detail::source_location::current()) : loc_(loc) {
            try {
                using namespace std::string_literals;
                // try to create a detailed error message containing the source location of the thrown exception
                fmt::memory_buffer buf;

                // format exception message
                fmt::format_to(buf, "Exception thrown {} \n",
                        (loc.rank().has_value() ?
                            fmt::format("on MPI_COMM_WORLD rank {}", loc.rank().value()) :
                            "without a running MPI environment"s));

                // format source location
                fmt::format_to(buf, "  in file {}\n  inf function {}\n  @ line {}\n\n", loc.file_name(), loc.function_name(), loc.line());

                // format stack trace
                fmt::format_to(buf, loc.stack_trace());

                using std::to_string;
                msg_ptr_ = std::make_shared<std::string>(to_string(buf));
            } catch (...) {
                // unable to create source location message
                msg_ptr_ = nullptr;
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
            return msg_ptr_ != nullptr ? msg_ptr_->c_str() : error_what;
        }

        /**
         * @brief Returns the created @ref detail::source_location information.
         * @return the @ref detail::source_location information
         */
        [[nodiscard]]
        const detail::source_location& location() const noexcept { return loc_; }

    protected:
        /**
         * @brief Tries to prepend @p msg to the current message.
         * @details There are two possible cases:
         *          -# the initial message (containing the @ref detail::source_location message) was created successfully:
         *             try to prepend @p msg; if an exception is thrown, this function has no effect
         *          -# the initial message (containing the @ref detail::source_location message) was **not** created successfully:
         *             try to store @p msg; if an exception is thrown, this function has no effect
         * @tparam T must meet the @ref mpicxx::detail::is_string requirements.
         * @param[in] msg the message to prepend
         */
        template <detail::is_string T>
        void prepend_to_what_message(T&& msg) {
            try {
                if (msg_ptr_ != nullptr) {
                    // successfully created base class message
                    // -> try to prepend the derived class message (msg)
                    msg_ptr_->insert(0, std::forward<T>(msg));
                } else {
                    // exception thrown while creating base class message
                    // -> try to only create the derived class message (msg)
                    msg_ptr_ = std::make_shared<std::string>(std::forward<T>(msg));
                }
            } catch (...) {
                // do nothing if the derived class message couldn't be prepended to the message
            }
        }
        /**
         * @brief Tries to append @p msg to the current message.
         * @details There are two possible cases:
         *          -# the initial message (containing the @ref detail::source_location message) was created successfully:
         *             try to append @p msg; if an exception is thrown, this function has no effect
         *          -# the initial message (containing the @ref detail::source_location message) was **not** created successfully:
         *             try to store @p msg; if an exception is thrown, this function has no effect
         * @tparam T must meet the @ref mpicxx::detail::is_string requirements.
         * @param[in] msg the message to append
         */
        template <detail::is_string T>
        void append_to_what_message(T&& msg) {
            try {
                if (msg_ptr_ != nullptr) {
                    // successfully created base class message
                    // -> try to append the derived class message (msg)
                    msg_ptr_->append(std::forward<T>(msg));
                } else {
                    // exception thrown while creating base class message
                    // -> try to only create the derived class message (msg)
                    msg_ptr_ = std::make_shared<std::string>(std::forward<T>(msg));
                }
            } catch (...) {
                // do nothing if the derived class message couldn't be appended to the message
            }
        }

    private:
        std::shared_ptr<std::string> msg_ptr_;
        const detail::source_location loc_;
    };


/**
 * @brief Macro to be able to easily use @ref MPICXX_PRETTY_FUNC_NAME__ for a better source location message.
 */
#define MPICXX_THROW_EXCEPTION(except, ...) \
    throw except(__VA_ARGS__ __VA_OPT__(,) mpicxx::detail::source_location::current(MPICXX_PRETTY_FUNC_NAME__));

}

#endif // MPICXX_EXCEPTION_HPP
