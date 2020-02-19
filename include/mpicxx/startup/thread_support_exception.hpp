#ifndef MPICXX_THREAD_SUPPORT_EXCEPTION_HPP
#define MPICXX_THREAD_SUPPORT_EXCEPTION_HPP

#include <stdexcept>
#include <string>

#include <fmt/format.h>

#include <mpicxx/startup/thread_support.hpp>


namespace mpicxx {

    class thread_support_not_satisfied : public std::exception {
    public:
        thread_support_not_satisfied(const thread_support required, const thread_support provided)
                : required_(required), provided_(provided) {
            msg_ =  fmt::format("Couldn't satisfy required level of thread support: {}\n", required);
            msg_ += fmt::format("Highest supported level of thread support:         {}"  , provided);
        }

        [[nodiscard]] virtual const char* what() const noexcept { return msg_.c_str(); }

        [[nodiscard]] thread_support required() const noexcept { return required_; }
        [[nodiscard]] thread_support provided() const noexcept { return provided_; }

    private:
        std::string msg_;
        const thread_support required_;
        const thread_support provided_;
    };

}

#endif // MPICXX_THREAD_SUPPORT_EXCEPTION_HPP
