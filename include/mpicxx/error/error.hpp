/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-14
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Defines error codes and error classes including standard once.
 */

#ifndef MPICXX_ERROR_HPP
#define MPICXX_ERROR_HPP

#include <mpicxx/detail/assert.hpp>
#include <mpicxx/detail/concepts.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <mpi.h>

#include <functional>
#include <iterator>
#include <optional>
#include <string_view>
#include <vector>

namespace mpicxx {

    // forward declaration of the error_category class
    class error_category;

    /**
     * @nosubgrouping
     * @brief This class represents an error code returned by calls to various MPI functions.
     */
    class error_code {
    public:
        // ---------------------------------------------------------------------------------------------------------- //
        //                                                constructor                                                 //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name constructor
        ///@{
        /**
         * @brief Constructs a new error code with the value given by @p code.
         * @param[in] code the error code value (default: [*MPI_SUCCESS*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node222.htm))
         *
         * @pre @p code **must** not be less than 0 or greater than the last used error code (@ref mpicxx::error_code::last_used_value()).
         *
         * @assert_sanity{ If @p code isn't a valid error code value. }
         */
        error_code(const int code = MPI_SUCCESS) noexcept : code_(code) {
            MPICXX_ASSERT_SANITY(this->valid_error_code(code),
                    "Attempt to create an error code with invalid value ({})! "
                    "Valid error code values must be in the interval [{}, {}].",
                    code, MPI_SUCCESS, error_code::last_used_value().value_or(std::numeric_limits<int>::max()));
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                 modifiers                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name modify error code
        ///@{
        /**
         * @brief Assign the new error code value @p code to the current one.
         * @param[in] code the new error code value
         *
         * @pre @p code **must** not be less than 0 or greater than the last used error code (@ref mpicxx::error_code::last_used_value()).
         *
         * @assert_sanity{ If @p code isn't a valid error code value. }
         */
        void assign(const int code) noexcept {
            MPICXX_ASSERT_SANITY(this->valid_error_code(code),
                    "Attempt to assign an error code with invalid value ({})! "
                    "Valid error code values must be in the interval [{}, {}].",
                    code, MPI_SUCCESS, error_code::last_used_value().value_or(std::numeric_limits<int>::max()));

            code_ = code;
        }
        /**
         * @brief Replaces the error code with the default value
         *        [*MPI_SUCCESS*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node222.htm).
         */
        void clear() noexcept { code_ = MPI_SUCCESS; }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  observer                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name observe error code
        ///@{
        /**
         * @brief Returns the value of the error code.
         * @return the error code value
         * @nodiscard
         */
        [[nodiscard]]
        constexpr int value() const noexcept { return code_; }
        /**
         * @brief Returns the value of the last used error code.
         * @details The returned value will **not** change unless a function to add an error class or an error category is called.
         * @return the last error code value
         * @nodiscard
         *
         * @note One can **not** assume that **all** values below the returned value are valid.
         *
         * @calls{ int MPI_Comm_get_attr(MPI_Comm comm, int comm_keyval, void *attribute_val, int *flag);    // exactly once }
         */
        [[nodiscard]]
        static const std::optional<int> last_used_value() {
            void* ptr;
            int flag;
            MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_LASTUSEDCODE, &ptr, &flag);
            if (static_cast<bool>(flag)) {
                return std::make_optional(*reinterpret_cast<int*>(ptr));
            } else {
                return std::nullopt;
            }
        }
        /**
         * @brief Returns the @ref mpicxx::error_category of the error code value.
         * @return the @ref mpicxx::error_category
         * @nodiscard
         *
         * @pre The current error code value **must** not be less than 0 or greater than the last used error code
         *      (@ref mpicxx::error_code::last_used_value()).
         *
         * @assert_precondition{ If the current error code value isn't a valid value. }
         *
         * @calls{ int MPI_Error_class(int errorcode, int *errorclass);    // exactly once }
         */
        [[nodiscard]]
        error_category category() const;
        /**
         * @brief Returns the error string associated with the error code value.
         * @return the error string
         * @nodiscard
         * 
         * @pre The current error code value **must** not be less than 0 or greater than the last used error code
         *      (@ref mpicxx::error_code::last_used_value()).
         *
         * @assert_precondition{ If the current error code value isn't a valid value. }
         *
         * @calls{ int MPI_Error_string(int errorcode, char *string, int *resultlen);    // exactly once }
         */
        [[nodiscard]]
        std::string message() const {
            MPICXX_ASSERT_PRECONDITION(this->valid_error_code(code_),
                    "Attempt to retrieve the error strong of an error code with invalid value ({})! "
                    "Valid error code values must be in the interval [{}, {}].",
                    code_, MPI_SUCCESS, error_code::last_used_value().value_or(std::numeric_limits<int>::max()));

            char error_string[MPI_MAX_ERROR_STRING];
            int resultlen;
            MPI_Error_string(code_, error_string, &resultlen);
            return std::string(error_string, resultlen);
        }
        /**
         * @brief Returns the maximum possible error string size.
         * @return the maximum error string size
         *         (= [*MPI_MAX_ERROR_STRING*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node221.htm))
         * @nodiscard
         */
        [[nodiscard]]
        static constexpr std::size_t max_message_size() { return static_cast<std::size_t>(MPI_MAX_ERROR_STRING); }
        /**
         * @brief Check if the error code value is valid, i.e. non-zero.
         * @return `false` if `value() == MPI_SUCCESS`, `true` otherwise
         * @nodiscard
         */
        [[nodiscard]]
        operator bool() const noexcept { return code_ != MPI_SUCCESS; }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            non-member functions                                            //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name non-member functions
        ///@{
        /**
         * @brief [Three-way comparison operator](https://en.cppreference.com/w/cpp/language/default_comparisons) for two
         *        @ref mpicxx::error_code @p lhs and @p rhs.
         * @details Automatically generates `%mpicxx::error_code::operator<`, `%mpicxx::error_code::operator<=`,
         *          `%mpicxx::error_code::operator>` and `%mpicxx::error_code::operator>=`.
         * @param[in] lhs an @ref mpicxx::error_code
         * @param[in] rhs an @ref mpicxx::error_code
         * @return the [`std::strong_ordering`](https://en.cppreference.com/w/cpp/utility/compare/strong_ordering) result
         * @nodiscard
         */
        [[nodiscard]]
        friend std::strong_ordering operator<=>(const error_code lhs, const error_code rhs) = default;
        /**
         * @brief Stream-insertion operator overload for the @ref mpicxx::error_code class.
         * @details Outputs the error code value **and** the associated error string.
         * @param[inout] out an output stream
         * @param[in] ec the @ref mpicxx::error_code
         * @return the output stream
         *
         * @pre The current error code value **must** not be less than 0 or greater than the last used error code
         *      (@ref mpicxx::error_code::last_used_value()).
         *
         * @assert_precondition{ If the current error code value isn't a valid value. }
         *
         * @calls{ int MPI_Error_string(int errorcode, char *string, int *resultlen);    // exactly once }
         */
        friend std::ostream& operator<<(std::ostream& out, const error_code ec) {
            return out << ec.value() << ": " << ec.message();
        }
        ///@}


    private:
#if MPICXX_ASSERTION_LEVEL > 0
        /*
         * @brief Checks whether @p code is a valid error code value, i.e. @p code is not less than 0 and not greater than the last used
         *        error code value (mpicxx::error_code::last_used_value()).
         * @param[in] code the error code value to check
         * @return `true` if @p code is a valid error code value, otherwise `false`
         */
        bool valid_error_code(const int code) const {
            return 0 <= code && code <= error_code::last_used_value().value_or(std::numeric_limits<int>::max());
        }
#endif

        int code_;
    };


    /**
     * @nosubgrouping
     * @brief This class represents an error category containing possibly multiple @ref error_code.
     * @details This class is marked `[[nodiscard]]` since the default constructor creates a new error category.
     *          Discarding this newly constructed @ref mpicxx::error_category wouldn't allow adding of new @ref mpicxx::error_code which
     *          would render the respective @ref mpicxx::error_category useless.
     */
    class [[nodiscard]] error_category {
        // Befriend error_code class to be able to create an error_category in mpicxx::error_code::category
        friend class error_code;

        /*
         * @brief Constructs an error category with the value given by @p category.
         * @default **Doesn't** create a new error category, but refers to an already existing one.
         *
         *    This constructor is `private` and is only used in the `mpicxx::error_code::category()` function.
         * @param[in] category the value of the error category
         */
        explicit constexpr error_category(const int category) noexcept : category_(category) { }

    public:
        // ---------------------------------------------------------------------------------------------------------- //
        //                                                 constructor                                                //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name constructor
        ///@{
        /**
         * @brief Constructs a new error category.
         */
//        error_category() noexcept { // TODO 2020-08-13 23:24 breyerml:
//            MPI_Add_error_class(&category_);
//        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                 modifiers                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name modify error category
        ///@{
        /**
         * @brief Constructs a new @ref mpicxx::error_code with @p error_string as error description associated with this
         *        @ref mpicxx::error_category.
         * @param[in] error_string the description of the new error code
         * @return the newly created @ref mpicxx::error_code
         */
        error_code add_error_code(const std::string_view error_string) const {
            int new_error_code;
            MPI_Add_error_code(category_, &new_error_code);
            MPI_Add_error_string(new_error_code, error_string.data());
            return error_code(new_error_code);
        }
        /**
         * @brief Constructs [`std::distance(first, last)`](https://en.cppreference.com/w/cpp/iterator/distance) new @ref mpicxx::error_code
         *        with the respective error description in the range [@p first, @p last) associated with this @ref mpicxx::error_category.
         * @param[in] first iterator to the first error description in the range
         * @param[in] last iterator one-past the last error description in the range
         * @return all newly created @ref mpicxx::error_code
         */
        template <std::input_iterator InputIt>
        std::vector<error_code> add_error_code(InputIt first, InputIt last) const requires (!detail::is_c_string<InputIt>) {
            std::vector<error_code> new_error_codes;
            new_error_codes.reserve(std::distance(first, last));
            for (; first != last; ++first) {
                new_error_codes.emplace_back(this->add_error_code(*first));
            }
            return new_error_codes;
        }
        /**
         * @brief Constructs `ilist.begin()` new @ref mpicxx::error_code with the respective error description in the
         *        [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) @p ilist associated with this
         *        @ref mpicxx::error_category.
         * @param ilist [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) containing the error
         *              descriptions
         * @return all newly created @ref mpicxx::error_code
         */
        template <detail::is_string T>
        std::vector<error_code> add_error_code(std::initializer_list<T> ilist) const {
            return this->add_error_code(ilist.begin(), ilist.end());
        }
        /**
         * @brief Constructs `sizeof...(T)` new @ref mpicxx::error_code with the respective error description in the
         *        parameter pack @p args associated with this @ref mpicxx::error_category.
         * @tparam T must meed the @ref mpicxx::detail::is_string requirements and must not greater than 1
         * @param args an arbitrary number (but at least 2) of error descriptions
         * @return all newly created @ref mpicxx::error_code
         */
        template <detail::is_string... T>
        std::vector<error_code> add_error_code(T&&... args) const requires (sizeof...(T) > 1) {
            std::vector<error_code> new_error_codes;
            new_error_codes.reserve(sizeof...(T));
            ([&](auto&& arg) {
                new_error_codes.emplace_back(this->add_error_code(std::forward<decltype(arg)>(arg)));
            }(std::forward<T>(args)), ...);
            return new_error_codes;
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  observer                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name observe error category
        ///@{
        /**
         * @brief Returns the value of the error category.
         * @return the error category value
         * @nodiscard
         */
        [[nodiscard]]
        constexpr int value() const noexcept { return category_; }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            non-member functions                                            //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name non-member functions
        ///@{
        /**
         * @brief [Three-way comparison operator](https://en.cppreference.com/w/cpp/language/default_comparisons) for two
         *        @ref mpicxx::error_category @p lhs and @p rhs.
         * @details Automatically generates mpicxx::error_category::operator<, mpicxx::error_category::operator<=,
         *          mpicxx::error_category::operator> and mpicxx::error_category::operator>=.
         * @param[in] lhs an @ref mpicxx::error_category
         * @param[in] rhs an @ref mpicxx::error_category
         * @return the [`std::strong_ordering`](https://en.cppreference.com/w/cpp/utility/compare/strong_ordering) result
         * @nodiscard
         */
        friend std::strong_ordering operator<=>(const error_category lhs, const error_category rhs) = default;
        /**
         * @brief Stream-insertion operator overload for the @ref mpicxx::error_category class.
         * @details Outputs the error category value.
         * @param[inout] out an output stream
         * @param[in] ec the @ref mpicxx::error_category
         * @return the output stream
         */
        friend std::ostream& operator<<(std::ostream& out, const error_category ec) {
            return out << ec.category_;
        }
        ///@}


    private:
        int category_;
    };


    [[nodiscard]]
    inline error_category error_code::category() const {
        MPICXX_ASSERT_PRECONDITION(this->valid_error_code(code_),
                "Attempt to retrieve the error string of an error code with invalid value ({})! "
                "Valid error code values must be in the interval [{}, {}].",
                code_, MPI_SUCCESS, error_code::last_used_value().value_or(std::numeric_limits<int>::max()));

        int category;
        MPI_Error_class(code_, &category);
        return error_category(category);
    }

}

namespace std {

    /**
     * @brief Specializes the [`std::hash`](https://en.cppreference.com/w/cpp/utility/hash) template to be able to hash a
     *        @ref mpicxx::error_code and therefore to be able to directly use a @ref mpicxx::error_code in a e.g. a
     *        [`std::unordered_set`](https://en.cppreference.com/w/cpp/container/unordered_set) or
     *        [std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map).
     * @details It hashes the error code value using the default [`std::hash<int>`](https://en.cppreference.com/w/cpp/utility/hash).
     * @return the hash value
     */
    template <>
    struct hash<mpicxx::error_code> {
        std::size_t operator()(mpicxx::error_code er) const noexcept {
            return std::hash<int>{}(er.value());
        }
    };

}

#endif // MPICXX_ERROR_HPP