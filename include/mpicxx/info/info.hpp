/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-04
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Implements a wrapper class around the [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object.
 * @details The @ref mpicxx::info class interface is inspired by the
 *          [`std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map) and
 *          [`std::map`](https://en.cppreference.com/w/cpp/container/map) interface.
 */

#ifndef MPICXX_INFO_HPP
#define MPICXX_INFO_HPP

#include <mpicxx/detail/assert.hpp>
#include <mpicxx/detail/concepts.hpp>
#include <mpicxx/detail/conversion.hpp>

#include <fmt/format.h>
#include <mpi.h>

#include <cstddef>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <limits>
#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace mpicxx {
    /**
     * @nosubgrouping
     * @brief This class is a wrapper to the [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object providing
     *        a interface inspired by [`std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map) and
     *        [`std::map`](https://en.cppreference.com/w/cpp/container/map).
     * @details Example usage:
     *          @snippet examples/info/info.cpp mwe
     */
    class info {

        // ---------------------------------------------------------------------------------------------------------- //
        //                                                 proxy class                                                //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief A proxy class for @ref mpicxx::info::at(T&&), @ref mpicxx::info::at(const std::string_view) const and
         *        @ref mpicxx::info::operator[](T&&) to distinguish between read and write accesses.
         * @details Calls @ref operator std::string() const on a write access and @ref operator=(const std::string_view) on a read access.
         *
         *    Can be printed through an @ref operator<<(std::ostream&, const proxy&) overload.
         */
        class proxy {
            /// Pointer type to the referred to info object.
            using MPI_Info_ptr = MPI_Info*;
            /// Reference type to the referred to info object.
            using MPI_Info_ref = MPI_Info&;
        public:
            /**
             * @brief Construct a new proxy object.
             * @tparam T must meet the @ref mpicxx::detail::is_string requirements
             * @param[in] info the referred to [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object
             * @param[in] key the provided @p key
             *
             * @pre @p info **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
             * @pre The @p key's length **must** be greater than 0 and less than
             *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
             *
             * @assert_sanity{ If @p info refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                 If @p key exceeds its size limit. }
             */
            template <detail::is_string T>
            proxy(MPI_Info_ref info, T&& key) : info_(std::addressof(info)), key_(std::forward<T>(key)) {
                MPICXX_ASSERT_SANITY(!this->info_refers_to_mpi_info_null(),
                        "Attempt to create a proxy from an info object referring to 'MPI_INFO_NULL'!");
                MPICXX_ASSERT_SANITY(this->legal_string_size(key_, MPI_MAX_INFO_KEY),
                        "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key_.size(), MPI_MAX_INFO_KEY);
            }

            /**
             * @brief On write access, add the provided @p value and saved key to the info object.
             * @details Creates a new [key, value]-pair if the key doesn't already exist, otherwise overwrites the existing @p value.
             * @param[in] value the @p value associated with key
             *
             * @pre @p value **must** include the null-terminator.
             * @pre The @p value's length **must** be greater than 0 and less than
             *      [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
             *
             * @assert_precondition{ If `*this` refers to an info object referring to
             *                       [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                       If @p value exceeds its size limit. }
             *
             * @calls{ int MPI_Info_set(MPI_Info info, const char *key, const char *value);    // exactly once }
             */
            void operator=(const std::string_view value) {
                MPICXX_ASSERT_PRECONDITION(!this->info_refers_to_mpi_info_null(),
                        "Attempt to access a [key, value]-pair of an info object referring to 'MPI_INFO_NULL'!");
                MPICXX_ASSERT_PRECONDITION(this->legal_string_size(value, MPI_MAX_INFO_VAL),
                        "Illegal info value: 0 < {} < {} (MPI_MAX_INFO_VAL)", value.size(), MPI_MAX_INFO_VAL);

                MPI_Info_set(*info_, key_.data(), value.data());
            }

            /**
             * @brief On read access, return the value associated with the saved key.
             * @details If the key doesn't exist yet, it will be inserted with a string consisting only of one whitespace as value,
             *          also returning a [`std::string(" ")`](https://en.cppreference.com/w/cpp/string/basic_string).
             * @return the value associated with key
             * @nodiscard
             *
             * @attention This function returns the associated value *by-value*, i.e. changing the returned
             *            [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) **won't** alter the info object's
             *            internal value!
             * @attention Because inserting an empty string `""` is not allowed, a `" "` string is inserted instead, if the key does not
             *            already exist.
             *
             * @assert_precondition{ If `*this` refers to an info object referring to
             *                       [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
             *
             * @calls{
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);       // exactly once
             * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                       // at most once
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);    // at most once
             * }
             */
            [[nodiscard]]
            operator std::string() const {
                MPICXX_ASSERT_PRECONDITION(!this->info_refers_to_mpi_info_null(),
                        "Attempt to access a [key, value]-pair of an info object referring to 'MPI_INFO_NULL'!");

                // get the length of the value
                int valuelen, flag;
                MPI_Info_get_valuelen(*info_, key_.data(), &valuelen, &flag);

                if (!static_cast<bool>(flag)) {
                    // the key doesn't exist yet
                    // -> add a new [key, value]-pair and return a std::string consisting of only one whitespace
                    std::string value(" ");
                    MPI_Info_set(*info_, key_.data(), value.data());
                    return value;
                }

                // key exists -> get the associated value
                std::string value(valuelen, ' ');
                MPI_Info_get(*info_, key_.data(), valuelen, value.data(), &flag);
                return value;
            }

            /**
             * @brief Convenience overload to be able to directly print a proxy object.
             * @details Calls @ref operator std::string() const to get the value that should be printed,
             *          i.e. if key doesn't exist yet, a new [key, value]-pair will be inserted into the info object
             * @param[inout] out the output stream to write on
             * @param[in] rhs the proxy object
             * @return the output stream
             *
             * @assert_precondition{ If @p rhs refers to an info object referring to
             *                       [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
             *
             * @calls{
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);       // exactly once
             * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                       // at most once
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);    // at most once
             * }
             */
            friend std::ostream& operator<<(std::ostream& out, const proxy& rhs) {
                MPICXX_ASSERT_PRECONDITION(!rhs.info_refers_to_mpi_info_null(),
                        "Attempt to access a [key, value]-pair of an info object referring to 'MPI_INFO_NULL'!");

                out << static_cast<std::string>(rhs.operator std::string());
                return out;
            }

        private:
#if MPICXX_ASSERTION_LEVEL > 0
            /*
             * @brief Check whether `*this` refers to an info object referring to
             *        [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
             * @return `true` if `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm),
             *         otherwise `false`
             */
            bool info_refers_to_mpi_info_null() const {
                return *info_ == MPI_INFO_NULL;
            }
            /*
             * @brief Check whether @p val has a legal size.
             * @details @p val has a legal size if it is greater than 0 and less then @p max_size.
             * @param[in] val the string to check
             * @param[in] max_size the maximum legal size
             * @return `true` if the size is legal, otherwise `false`
             */
            bool legal_string_size(const std::string_view val, const int max_size) const {
                return 0 < val.size() && static_cast<int>(val.size()) < max_size;
            }
#endif

            MPI_Info_ptr info_;
            const std::string key_;
        };


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  iterators                                                 //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @nosubgrouping
         * @brief Provides iterator and const_iterator for an info object.
         * @details The standard reverse_iterator and const_reverse_iterator are provided
         *          in terms of [`std::reverse_iterator<iterator>`](https://en.cppreference.com/w/cpp/iterator/reverse_iterator) and
         *          [`std::reverse_iterator<const_iterator>`](https://en.cppreference.com/w/cpp/iterator/reverse_iterator) respectively.
         *
         *          All necessary functions for a
         *          [*random access iterator*](https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator) are implemented.
         *
         *          Each iterator can be in one of three possible states:
         *          1. singular iterator (the iterator is currently not attached to any info object)
         *          2. iterator referring to an info object referring to *MPI_INFO_NULL*
         *          3. valid
         *
         *          The presence of 1\. and 2\. can be checked using assertions if the assertion categories *SANITY* and *PRECONDITION* are
         *          activated during [`CMake`](https://cmake.org/)'s configuration step.
         *
         * @tparam is_const if `true` a const_iterator is instantiated, otherwise a non-const iterator
         */
        template <bool is_const>
        class iterator_impl {
            // be able to construct a const_iterator from an iterator
            template <bool>
            friend class iterator_impl;
            // info class can now directly access the pos member
            friend class info;

            /**
             * @brief Helper class returned by @ref mpicxx::info::iterator_impl::operator->() (since simply returning a pointer to a
             *        temporary object would result in a dangling pointer).
             * @tparam T the type of the wrapped value
             */
            template <typename T>
            class pointer_impl {
            public:
                /**
                 * @brief Construct a new @ref pointer_impl wrapping @p val.
                 * @param val the value to wrap
                 */
                explicit pointer_impl(T&& val) : val_(std::move(val)) { }
                /**
                 * @brief Overload for the [`operator->()`](https://en.cppreference.com/w/cpp/language/operator_member_access).
                 * @return A pointer to the underlying value.
                 */
                T* operator->() { return &val_; }
            private:
                T val_;
            };

            // pointer type to the referred to info object (pointer to const if `is_const` is `true`)
            using MPI_Info_ptr = std::conditional_t<is_const, const MPI_Info*, MPI_Info*>;
            // reference type to the referred to info object (reference to const if `is_const` is `true`)
            using MPI_Info_ref = std::conditional_t<is_const, const MPI_Info&, MPI_Info&>;
        public:
            // ---------------------------------------------------------------------------------------------------------- //
            //                                         iterator_traits definitions                                        //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) difference type to identify the
             *        distance between two iterators.
             */
            using difference_type = std::ptrdiff_t;
            /**
             * @brief [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) value type that can be obtained
             *        by dereferencing the iterator.
             * @details In case of a non-const iterator, the value will be returned by a @ref mpicxx::info::proxy object to allow changing
             *          its value.
             *
             *          In case of a const_iterator, the value will directly be returned as
             *          [`const std::string`](https://en.cppreference.com/w/cpp/string/basic_string) because changing the value is
             *          impossible by definition.
             */
            using value_type = std::conditional_t<is_const,
                                                  std::pair<const std::string, const std::string>,
                                                  std::pair<const std::string, proxy>>;
            /**
             * @brief [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) pointer type to the
             *        [key, value]-pair iterated over.
             * @details Because it is not possible to simply return value_type* (dangling pointer to a local object),
             *          it is necessary to wrap value_type in a @ref mpicxx::info::iterator_impl::pointer_impl object
             *          (which in turn overloads [`operator->()`](https://en.cppreference.com/w/cpp/language/operator_member_access)).
             */
            using pointer = pointer_impl<value_type>;
            /**
             * @brief [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) reference type
             *        (**not** meaningful because
             *        [`mpicxx::info::iterator_impl::operator*()`](https://en.cppreference.com/w/cpp/language/operator_member_access) and
             *        [`mpicxx::info::iterator_impl::operator->()`](https://en.cppreference.com/w/cpp/language/operator_member_access) \n
             *        has to return **by-value** (using a proxy for write access)).
             */
            using reference = value_type;
            /// [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) iterator category.
            using iterator_category = std::random_access_iterator_tag;
            /// [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) iterator concept (for C++20 concepts)
            using iterator_concept = std::random_access_iterator_tag;

            // ---------------------------------------------------------------------------------------------------------- //
            //                                                constructors                                                //
            // ---------------------------------------------------------------------------------------------------------- //
            /// @name constructors and destructor
            ///@{
            /**
             * @brief Default construct a new iterator.
             *
             * @post `*this` is **singular**.
             */
            iterator_impl() : info_(nullptr), pos_(0) { }
            /**
             * @brief Construct a new iterator referring to @p info at position @p pos.
             * @param[inout] info the referred to [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object
             * @param[in] pos the iterator's current position
             *
             * @post `*this` is not singular if and only if @p info is not the
             *       [`nullptr`](https://en.cppreference.com/w/cpp/language/nullptr).
             *
             * @assert_sanity{ If @p info is a [`nullptr`](https://en.cppreference.com/w/cpp/language/nullptr). \n
             *                 If @p info refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                 If @p pos falls outside the valid range. }
             */
            iterator_impl(MPI_Info_ref info, const difference_type pos) : info_(std::addressof(info)), pos_(pos) {
                MPICXX_ASSERT_SANITY(!this->singular(),
                        "Attempt to explicitly create a singular iterator!");
                MPICXX_ASSERT_SANITY(!this->info_refers_to_mpi_info_null(),
                        "Attempt to create an iterator from an info object referring to 'MPI_INFO_NULL'!");
                MPICXX_ASSERT_SANITY(pos_ >= 0 && pos <= this->info_size(),
                        "Attempt to create an iterator referring to {}, which falls outside its valid range!!", pos);
            }
            /**
             * @brief Copy constructor. Constructs the info object with a copy of the contents of @p other.
             * @param[in] other iterator to use as data source
             *
             * @post `*this` is not singular if and only if @p other is not singular.
             *
             * @assert_sanity{ If @p other is a singular iterator. \n
             *                 If @p other refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
             */
            iterator_impl(const iterator_impl& other) : info_(other.info_), pos_(other.pos_) {
                MPICXX_ASSERT_SANITY(!other.singular() && !other.info_refers_to_mpi_info_null(),
                        "Attempt to create an iterator from a {} iterator{}!",
                        other.state(), other.info_state());
            }
            /**
             * @brief Special copy constructor. Convert a non-const iterator to a const_iterator.
             * @tparam other_const
             * @param[in] other the copied iterator
             *
             * @post `*this` is not singular if and only if @p other is not singular.
             *
             * @assert_sanity{ If @p other is a singular iterator. \n
             *                 If @p other refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
             */
            template <bool other_const>
            iterator_impl(const iterator_impl<other_const>& other) : info_(other.info_), pos_(other.pos_) {
                static_assert(is_const || !other_const, "Attempt to assign a const_iterator to a non-const iterator!");

                MPICXX_ASSERT_SANITY(!other.singular() && !other.info_refers_to_mpi_info_null(),
                        "Attempt to create an iterator from a {} iterator{}!",
                        other.state(), other.info_state());
            }
            ///@}


            // ---------------------------------------------------------------------------------------------------------- //
            //                                            assignment operators                                            //
            // ---------------------------------------------------------------------------------------------------------- //
            /// @name assignment operators
            ///@{
            /**
             * @brief Copy assignment operator. Replace the contents with a copy of the contents of @p other.
             * @param[in] rhs another iterator to use as data source
             * @return `*this`
             *
             * @post `*this` is not singular if and only if @p rhs is not singular.
             *
             * @assert_sanity{ If @p rhs is a singular iterator. \n
             *                 If @p rhs refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
             */
            iterator_impl& operator=(const iterator_impl& rhs) {
                MPICXX_ASSERT_SANITY(!rhs.singular() && !rhs.info_refers_to_mpi_info_null(),
                        "Attempt to assign a {} iterator{} to a {} iterator{}!",
                        rhs.state(), rhs.info_state(), this->state(), this->info_state());

                info_ = rhs.info_;
                pos_ = rhs.pos_;
                return *this;
            }
            /**
             * @brief Special copy assignment operator. Assign a non-const iterator to a const_iterator.
             * @tparam rhs_const
             * @param[in] rhs another iterator to use as data source
             * @return `*this`
             *
             * @post `*this` is not singular if and only if @p rhs is not singular.
             *
             * @assert_sanity{ If @p rhs is a singular iterator. \n
             *                 If @p rhs refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
             */
            template <bool rhs_const>
            iterator_impl& operator=(const iterator_impl<rhs_const>& rhs) {
                static_assert(is_const || !rhs_const, "Attempt to assign a const_iterator to a non-const iterator!");

                MPICXX_ASSERT_SANITY(!rhs.singular() && !rhs.info_refers_to_mpi_info_null(),
                        "Attempt to assign a {} iterator{} to a {} iterator{}!",
                        rhs.state(), rhs.info_state(), this->state(), this->info_state());

                info_ = rhs.info_;
                pos_ = rhs.pos_;
                return *this;
            }
            ///@}


            // ---------------------------------------------------------------------------------------------------------- //
            //                                            relational operators                                            //
            // ---------------------------------------------------------------------------------------------------------- //
            /// @name relational operators
            ///@{
            /**
             * @brief Compares `*this` and @p rhs for equality.
             * @details Automatically generates mpicxx::info::iterator_impl::operator!=.
             *
             *    The iterators `*this` and @p rhs **may not** necessarily have the same constness.
             * @tparam rhs_const
             * @param[in] rhs the other iterator
             * @return `true` if both are equal, otherwise `false`
             * @nodiscard
             *
             * @assert_sanity{ If `*this` or @p rhs is a singular iterator. \n
             *                 If `*this` or @p rhs refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                 If `*this` and @p rhs don't refer to the same info object. }
             */
            template <bool rhs_const>
            [[nodiscard]]
            bool operator==(const iterator_impl<rhs_const>& rhs) const {
                MPICXX_ASSERT_SANITY(!this->singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
                        this->state(), rhs.state());
                MPICXX_ASSERT_SANITY(!this->info_refers_to_mpi_info_null() && !rhs.info_refers_to_mpi_info_null(),
                        "Attempt to compare a {} iterator{} to a {} iterator{}!",
                        this->state(), this->info_state(), rhs.state(), rhs.info_state());
                MPICXX_ASSERT_SANITY(this->comparable(rhs), "Attempt to compare iterators from different sequences!");

                return info_ == rhs.info_ && pos_ == rhs.pos_;
            }
            /**
             * @brief [Three-way comparison operator](https://en.cppreference.com/w/cpp/language/default_comparisons) for `*this` and
             *        @p rhs.
             * @details Automatically generates mpicxx::info::iterator_impl::operator<, mpicxx::info::iterator_impl::operator<=,
             *          mpicxx::info::iterator_impl::operator> and mpicxx::info::iterator_impl::operator>=.
             *          
             *          The iterators `*this` and @p rhs **may not** necessarily have the same constness.
             * @tparam rhs_const
             * @param[in] rhs the other iterator
             * @return the [`std::partial_ordering`](https://en.cppreference.com/w/cpp/utility/compare/partial_ordering) result
             * @nodiscard
             *
             * @assert_sanity{ If `*this` or @p rhs is a singular iterator. \n
             *                 If `*this` or @p rhs refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                 If `*this` and @p rhs don't refer to the same info object. }
             */
            template <bool rhs_const>
            [[nodiscard]]
            std::partial_ordering operator<=>(const iterator_impl<rhs_const>& rhs) const {
                MPICXX_ASSERT_SANITY(!this->singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
                        this->state(), rhs.state());
                MPICXX_ASSERT_SANITY(!this->info_refers_to_mpi_info_null() && !rhs.info_refers_to_mpi_info_null(),
                        "Attempt to compare a {} iterator{} to a {} iterator{}!",
                        this->state(), this->info_state(), rhs.state(), rhs.info_state());
                MPICXX_ASSERT_SANITY(this->comparable(rhs), "Attempt to compare iterators from different sequences!");

                if (auto cmp = info_ <=> rhs.info_; cmp != 0) return std::partial_ordering::unordered;
                return pos_ <=> rhs.pos_;
            }
            ///@}


            // ---------------------------------------------------------------------------------------------------------- //
            //                                                  modifiers                                                 //
            // ---------------------------------------------------------------------------------------------------------- //
            /// @name modifiers
            ///@{
            /**
             * @brief Move the iterator one position forward.
             * @return modified iterator pointing to the new position
             *
             * @assert_sanity{ If `*this` is a singular iterator. \n
             *                 If `*this` refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                 If `*this` is a past-the-end iterator. }
             */
            iterator_impl& operator++() {
                MPICXX_ASSERT_SANITY(this->incrementable(), "Attempt to increment a {} iterator{}!", this->state(), this->info_state());

                ++pos_;
                return *this;
            }
            /**
             * @brief Move the iterator one position forward and return the old iterator.
             * @return iterator pointing to the old position
             *
             * @assert_sanity{ If `*this` is a singular iterator. \n
             *                 If `*this` refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                 If `*this` is a past-the-end iterator. }
             */
            iterator_impl operator++(int) {
                MPICXX_ASSERT_SANITY(this->incrementable(), "Attempt to increment a {} iterator{}!", this->state(), this->info_state());

                iterator_impl tmp{*this};
                operator++();
                return tmp;
            }
            /**
             * @brief Move this iterator @p inc steps forward.
             * @param[in] inc number of steps (@p inc may be negative)
             * @return modified iterator pointing to the new position
             *
             * @assert_sanity{ If `*this` is a singular iterator. \n
             *                 If `*this` refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                 If `*this + inc` falls outside the valid range. }
             */
            iterator_impl& operator+=(const difference_type inc) {
                MPICXX_ASSERT_SANITY(this->advanceable(inc),
                        "Attempt to advance a {} iterator{} {} steps, which falls outside its valid range!",
                        this->state(), this->info_state(), inc);

                pos_ += inc;
                return *this;
            }
            /**
             * @brief Move the iterator @p inc steps forward.
             * @param[in] it the iterator to increment
             * @param[in] inc number of steps (@p inc may be negative)
             * @return new iterator pointing to the new position
             * @nodiscard
             *
             * @assert_sanity{ If `it` is a singular iterator. \n
             *                 If `it` refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                 If `it + inc` falls outside the valid range. }
             */
            [[nodiscard("Did you mean 'operator+='?")]]
            friend iterator_impl operator+(iterator_impl it, const difference_type inc) {
                MPICXX_ASSERT_SANITY(it.advanceable(inc),
                        "Attempt to advance a {} iterator{} {} steps, which falls outside its valid range!",
                        it.state(), it.info_state(), inc);

                it.pos_ += inc;
                return it;
            }
            /**
             * @copydoc operator+(iterator_impl, const difference_type)
             */
            [[nodiscard("Did you mean 'operator+='?")]]
            friend iterator_impl operator+(const difference_type inc, iterator_impl it) {
                MPICXX_ASSERT_SANITY(it.advanceable(inc),
                        "Attempt to advance a {} iterator{} {} steps, which falls outside its valid range!",
                        it.state(), it.info_state(), inc);

                return it + inc;
            }
            /**
             * @brief Move the iterator one position backward.
             * @return modified iterator pointing to the new position
             *
             * @assert_sanity{ If `*this` is a singular iterator. \n
             *                 If `*this` refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                 If `*this` is a start-of-sequence iterator. }
             */
            iterator_impl& operator--() {
                MPICXX_ASSERT_SANITY(this->decrementable(), "Attempt to decrement a {} iterator{}!", this->state(), this->info_state());

                --pos_;
                return *this;
            }
            /**
             * @brief Move the iterator one position backward and return the old iterator.
             * @return iterator pointing to the old position
             *
             * @assert_sanity{ If `*this` is a singular iterator. \n
             *                 If `*this` refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                 If `*this` is a start-of-sequence iterator. }
             */
            iterator_impl operator--(int) {
                MPICXX_ASSERT_SANITY(this->decrementable(), "Attempt to decrement a {} iterator{}!", this->state(), this->info_state());

                iterator_impl tmp{*this};
                operator--();
                return tmp;
            }
            /**
             * @brief Move the iterator @p inc steps backward.
             * @param[in] inc number of steps (@p inc may be negative)
             * @return modified iterator pointing to the new position
             *
             * @assert_sanity{ If `*this` is a singular iterator. \n
             *                 If `*this` refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                 If `*this` is a start-of-sequence iterator. }
             */
            iterator_impl& operator-=(const difference_type inc) {
                MPICXX_ASSERT_SANITY(this->advanceable(-inc),
                        "Attempt to retreat a {} iterator{} {} steps, which falls outside its valid range!",
                        this->state(), this->info_state(), inc);

                pos_ -= inc;
                return *this;
            }
            /**
             * @brief Move the iterator @p inc steps backward.
             * @param[in] it the iterator to decrement
             * @param[in] inc number of steps (@p inc may be negative)
             * @return new iterator pointing to the new position
             * @nodiscard
             *
             * @assert_sanity{ If `it` is a singular iterator. \n
             *                 If `it` refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                 If `it - inc` falls outside the valid range. }
             */
            [[nodiscard("Did you mean 'operator-='?")]]
            friend iterator_impl operator-(iterator_impl it, const difference_type inc) {
                MPICXX_ASSERT_SANITY(it.advanceable(-inc),
                        "Attempt to retreat a {} iterator{} {} steps, which falls outside its valid range!",
                        it.state(), it.info_state(), inc);

                it.pos_ -= inc;
                return it;
            }
            ///@}


            // ---------------------------------------------------------------------------------------------------------- //
            //                                            distance calculation                                            //
            // ---------------------------------------------------------------------------------------------------------- //
            /// @name distance calculation
            ///@{
            /**
             * @brief Calculate the distance between the iterator and the given @p rhs one.
             * @details The iterators `*this` and @p rhs **may not** necessarily have the same constness.
             *
             *    It holds: `it2 - it1 ==` [`std::distance(it1, it2)`](https://en.cppreference.com/w/cpp/iterator/distance).
             * @tparam rhs_const
             * @param[in] rhs the end iterator
             * @return number of [key, value]-pairs between the iterators `*this` and  @p rhs
             * @nodiscard
             *
             * @assert_sanity{ If `*this` or @p rhs is a singular iterator. \n
             *                 If `*this` or @p rhs refers to an info object referring to
             *                 [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                 If `*this` and @p rhs don't refer to the same info object. }
             */
            template <bool rhs_const>
            [[nodiscard]]
            difference_type operator-(const iterator_impl<rhs_const>& rhs) const {
                MPICXX_ASSERT_SANITY(!this->singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
                        this->state(), rhs.state());
                MPICXX_ASSERT_SANITY(!this->info_refers_to_mpi_info_null() && !rhs.info_refers_to_mpi_info_null(),
                        "Attempt to compare a {} iterator{} to a {} iterator{}!",
                        this->state(), this->info_state(), rhs.state(), rhs.info_state());
                MPICXX_ASSERT_SANITY(this->comparable(rhs), "Attempt to compare iterators from different sequences!");

                return pos_ - rhs.pos_;
            }
            ///@}


            // ---------------------------------------------------------------------------------------------------------- //
            //                                               dereferencing                                                //
            // ---------------------------------------------------------------------------------------------------------- //
            /// @name dereferencing
            ///@{
            /**
             * @brief Get the [key, value]-pair at the current iterator position + @p n.
             * @details If the current iterator is a const_iterator, the returned type is a
             *          [`std::pair<const std::string, const std::string>`](https://en.cppreference.com/w/cpp/utility/pair).
             *
             *          If the current iterator is a non-const iterator, the returned type is a
             *          [`std::pair<const std::string, proxy>`](https://en.cppreference.com/w/cpp/utility/pair), i.e. the
             *          [key, value]-pair's value can be changed through the proxy object.
             * @param[in] n the requested offset of the iterator (@p n may be negative)
             * @return the [key, value]-pair
             * @nodiscard
             *
             * @pre `*this` **must not** be a singular iterator.
             * @pre `*this` **must not** refer to an info object referring to
             *      [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
             * @pre `*this` **must** be dereferenceable, i.e. the position denoted by the current iterator + @p n must be in the
             *      half-open interval [0, nkeys), where `nkeys` ist the size of the referred to info object.
             *
             * @assert_precondition{ If `*this` is a singular iterator. \n
             *                       If `*this` refers to an info object referring to
             *                       [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                       If `*this` can't be dereferenced. }
             *
             * @calls_ref{
             * @code{.cpp}
             * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                  // exactly once
             * @endcode
             * `const_iterator` (alias for an `iterator_impl<true>`): \n
             * @code{.cpp}
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);       // exactly once
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);    // exactly once
             * @endcode
             * `iterator` (alias for an `iterator_impl<false>`): \n
             * For *MPI* functions called while using a proxy see the @ref mpicxx::info::proxy documentation.
             * }
             */
            [[nodiscard]]
            reference operator[](const difference_type n) const {
                MPICXX_ASSERT_PRECONDITION(!this->singular() && !this->info_refers_to_mpi_info_null(),
                        "Attempt to subscript a {} iterator{}!",
                        this->state(), this->info_state());
                MPICXX_ASSERT_PRECONDITION(this->advanceable(n) && this->advanceable(n + 1),
                        "Attempt to subscript a {} iterator {} step from its current position, which falls outside its dereferenceable range.",
                        this->state(), n);

                // get the key (with an offset of n)
                char key[MPI_MAX_INFO_KEY];
                MPI_Info_get_nthkey(*info_, pos_ + n, key);

                if constexpr (is_const) {
                    // this is currently a const_iterator
                    // -> retrieve the value associated with the key

                    // get the length of the value associated with the key
                    int valuelen, flag;
                    MPI_Info_get_valuelen(*info_, key, &valuelen, &flag);

                    // get the value associated with the key
                    std::string value(valuelen, ' ');
                    MPI_Info_get(*info_, key, valuelen, value.data(), &flag);

                    return std::make_pair(std::string(key), std::move(value));
                } else {
                    // this is currently a non-const iterator
                    // -> create a proxy object and return it as value instead of a std::string
                    // (allows changing the value in the info object)
                    return std::make_pair(std::string(key), proxy(*info_, key));
                }
            }
            /**
             * @brief Get the [key, value]-pair at the current iterator position.
             * @details If the current iterator is a const_iterator, the returned type is a
             *          [`std::pair<const std::string, const std::string>`](https://en.cppreference.com/w/cpp/utility/pair).
             *
             *          If the current iterator is a non-const iterator, the returned type is a
             *          [`std::pair<const std::string, proxy>`](https://en.cppreference.com/w/cpp/utility/pair), i.e. the
             *          [key, value]-pair's value can be changed through the proxy object.
             * @return the [key, value]-pair
             * @nodiscard
             *
             * @pre `*this` **must not** be a singular iterator.
             * @pre `*this` **must not** refer to an info object referring to
             *      [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
             * @pre `*this` **must** be dereferenceable, i.e. the position denoted by the current iterator must be in the half-open interval
             *      [0, nkeys), where `nkeys` ist the size of the referred to info object.
             *
             * @assert_precondition{ If `*this` is a singular iterator. \n
             *                       If `*this` refers to an info object referring to
             *                       [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
             *                       If `*this` can't be dereferenced. }
             *
             * @calls_ref{
             * @code{.cpp}
             * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                  // exactly once
             * @endcode
             * `const_iterator` (alias for an `iterator_impl<true>`): \n
             * @code{.cpp}
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);       // exactly once
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);    // exactly once
             * @endcode
             * `iterator` (alias for an `iterator_impl<false>`): \n
             * For *MPI* functions called while using a proxy see the @ref mpicxx::info::proxy documentation.
             * }
             */
            [[nodiscard]]
            reference operator*() const {
                MPICXX_ASSERT_PRECONDITION(!this->singular() && !this->info_refers_to_mpi_info_null() && this->dereferenceable(),
                        "Attempt to dereference a {} iterator{}!", this->state(), this->info_state());

                return this->operator[](0);
            }
            /**
             * @copydoc operator*()
             */
            [[nodiscard]]
            pointer operator->() const {
                MPICXX_ASSERT_PRECONDITION(!this->singular() && !this->info_refers_to_mpi_info_null() && this->dereferenceable(),
                        "Attempt to dereference a {} iterator{}!", this->state(), this->info_state());

                return pointer(this->operator[](0));
            }
            ///@}


        private:
#if MPICXX_ASSERTION_LEVEL > 0
            /*
             * @brief Calculate the size of the referred to info object.
             * @details If `*this` is a singular iterator or the referred to info object refers to
             *          [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm), the size is 0.
             * @return the size of the referred to info object
             */
            difference_type info_size() const {
                if (this->singular() || this->info_refers_to_mpi_info_null()) {
                    return 0;
                }
                int nkeys = 0;
                MPI_Info_get_nkeys(*info_, &nkeys);
                return static_cast<difference_type>(nkeys);
            }
            /*
             * @brief Checks whether `*this` is a singular iterator.
             * @return `true` if `*this` is a singular iterator, otherwise `false`
             */
            bool singular() const {
                return info_ == nullptr;
            }
            /*
             * @brief Check whether `*this` refers to an info object referring to
             *        [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
             * @details A singular iterator **does not** count as iterator referring to an info object referring to
             *          [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
             * @return `true` if `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm),
             *         otherwise `false`
             */
            bool info_refers_to_mpi_info_null() const {
                return info_ != nullptr && *info_ == MPI_INFO_NULL;
            }
            /*
             * @brief Checks whether `*this` and @p rhs can be compared to each other.
             * @details Two iterators are comparable if and only if they are not singular and they refer to the same info object, which
             *          doesn't refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
             * @return `true` if @rhs is comparable, otherwise `false`
             */
            template <bool rhs_const>
            bool comparable(const iterator_impl<rhs_const>& rhs) const {
                return !this->singular() && !rhs.singular() && info_ == rhs.info_;
            }
            /*
             * @brief Checks whether `*this` is a past-the-end iterator (e.g. an iterator obtained by calling @ref mpicxx::info::end()).
             * @return `true` if `*this` is a past-the-end iterator, otherwise `false`
             */
            bool past_the_end() const {
                return pos_ >= this->info_size();
            }
            /*
             * @brief Checks whether `*this` is a start-of-sequnence iterator (e.g. an iterator obtained by calling
             *        @ref mpicxx::info::begin()).
             * @return `true` if `*this` is a start-of-sequence iterator, otherwise `false`
             */
            bool start_of_sequence() const {
                return pos_ == 0;
            }
            /*
             * @brief Checks whether `*this` can be incremented.
             * @details An iterator can be incremented if and only if it is **not**:
             *          - a singular iterator
             *          - referring to an info object referring to
             *            [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm)
             *          - a past-the-end iterator
             * @return `true` if `*this` is incrementable, otherwise `false`
             */
            bool incrementable() const {
                return !this->singular() && !this->info_refers_to_mpi_info_null() && !this->past_the_end();
            }
            /*
             * @brief Checks whether `*this` can be decremented.
             * @details An iterator can be decremented if and only if it is **not**:
             *          - a singular iterator
             *          - referring to an info object referring to
             *            [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm)
             *          - a start-of-sequence iterator
             * @return `true` if `*this` is decrementable, otherwise `false`
             */
            bool decrementable() const {
                return !this->singular() && !this->info_refers_to_mpi_info_null() && !this->start_of_sequence();
            }
            /*
             * @brief Checks whether `*this` can be advanced.
             * @details An iterator can be advanced if and only if:
             *          - it is **not** a singular iterator
             *          - it is **not** referring to an info object referring to
             *            [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm)
             *          - the current position + @p **does not** fall outside its valid range
             * @param[in] n the number of advance steps (@p n may be negative)
             * @return `true` if `*this` is advancable, otherwise `false`
             */
            bool advanceable(const difference_type n) const {
                if (this->singular() || this->info_refers_to_mpi_info_null()) {
                    return false;
                } else if (n > 0) {
                    return pos_ + n <= this->info_size();
                } else {
                    return pos_ + n >= 0;
                }
            }
            /*
             * @brief Checks whether `*this` can be safely dereferenced.
             * @details An iterator can be dereferenced if and only if it is **not**:
             *          - a singular iterator
             *          - referring to an info object referring to
             *            [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm)
             *          - a past-the-end iterator
             *          - a before-begin iterator
             * @return `true` if `*this` is dereferenceable, otherwise `false`
             */
            bool dereferenceable() const {
                return !this->singular() && !this->info_refers_to_mpi_info_null() && !this->past_the_end() && pos_ >= 0;
            }
            /*
             * @brief Returns a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) describing the current state
             *        of `*this`.
             * @return the current state of `*this`
             */
            std::string state() const {
                if (this->singular()) {
                    return std::string("singular");
                } else if (this->past_the_end()) {
                    return std::string("past-the-end");
                } else if (pos_ < 0) {
                    return std::string("before-begin");
                } else if (this->start_of_sequence()) {
                    return std::string("dereferenceable (start-of-sequence)");
                } else {
                    return std::string("dereferenceable");
                }
            }
            /*
             * @brief Returns a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) describing whether the referred to
             *        info object refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
             * @return a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) describing whether `*this` refers to
             *         [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm)
             */
            std::string info_state() const {
                using namespace std::string_literals;
                if (this->info_refers_to_mpi_info_null()) {
                    return std::string(" (referring to an info object refering to 'MPI_INFO_NULL')");
                } else {
                    return std::string();
                }
            }
#endif

            MPI_Info_ptr info_;
            difference_type pos_;
        };


    public:
        // ---------------------------------------------------------------------------------------------------------- //
        //                                                member types                                                //
        // ---------------------------------------------------------------------------------------------------------- //
        /// The type of a key.
        using key_type = std::string;
        /// The type of a value associated with a key.
        using mapped_type = std::string;
        /// The type of a [key, value]-pair.
        using value_type = std::pair<const key_type, mapped_type>;
        /// Unsigned integer type.
        using size_type = std::size_t;
        /// Signed integer type.
        using difference_type = std::ptrdiff_t;
        /// The type of value_type used as a reference.
        using reference = value_type&;
        /// The type of value_type used as a const reference.
        using const_reference = const value_type&;
        /// The type of value_type used as a pointer.
        using pointer = value_type*;
        /// The type of value_type used as a const pointer.
        using const_pointer = const value_type*;
        /// Alias for an iterator using the `iterator_impl` template class with `is_const` set to `false`.
        using iterator = iterator_impl<false>;
        /// Alias for a const_iterator using the `iterator_impl` template class with `is_const` set to `true`.
        using const_iterator = iterator_impl<true>;
        /// Alias for a reverse_iterator using [`std::reverse_iterator`](https://en.cppreference.com/w/cpp/iterator/reverse_iterator).
        using reverse_iterator = std::reverse_iterator<iterator>;
        /// Alias for a const_reverse_iterator using [`std::reverse_iterator`](https://en.cppreference.com/w/cpp/iterator/reverse_iterator).
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;


        // ---------------------------------------------------------------------------------------------------------- //
        //                                             static data member                                             //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Static member that holds all execution environment information defined in
         *        [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node225.htm).
         * @details As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) predefined keys of
         *          [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node225.htm) are:
         *  key         | description
         * :----------- | :--------------------------------------------------------------------------------------------------------------
         * command      | name of program executed
         * argv         | space separated arguments to command
         * maxprocs     | maximum number of MPI processes to start (e.g. `"1024"`)
         * soft         | allowed values for number of processors
         * host         | hostname
         * arch         | architecture name
         * wdir         | working directory of the MPI process
         * file         | value is the name of a file in which additional information is specified
         * thread_level | requested level of thread support, if requested before the program started execution (e.g. `"MPI_THREAD_SINGLE"`) 
         *
         * @note The contents of [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node225.htm) are implementation
         *       defined, i.e. not all of the predefined keys have to be defined.
         *
         * @attention **No** [*MPI_Info_free*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) gets called upon destruction
         *            (doing so would result in a MPI runtime failure).
         */
        static const info env;

        /**
         * @brief Static null object which is mainly used to explicitly indicate that **no** info is provided.
         *
         * @attention **No** [*MPI_Info_free*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) gets called upon destruction
         *            (doing so would result in a MPI runtime failure).
         */
        static const info null;


        // ---------------------------------------------------------------------------------------------------------- //
        //                                        constructors and destructor                                         //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name constructors and destructor
        ///@{
        /**
         * @brief Constructs an empty info object.
         *
         * @post The newly constructed info object is in a valid state.
         *
         * @calls{ int MPI_Info_create(MPI_Info *info);    // exactly once }
         */
        info() : is_freeable_(true) {
            // initialize an empty info object
            MPI_Info_create(&info_);
        }
        /**
         * @brief Copy constructor. Constructs the info object with a copy of the contents of @p other.
         * @details Retains @p other's [key, value]-pair ordering.
         * @param[in] other another info object to be used as source to initialize the [key, value]-pairs of this info object with
         *
         * @post The newly constructed info object is in a valid state iff @p other is in a valid state.
         * @attention Every copied info object (except if `other.get() == MPI_INFO_NULL`) is marked **freeable** independent of the
         *            **freeable** state of make the copied-from info object.
         *
         * @calls{ int MPI_Info_dup(MPI_info info, MPI_info *newinfo);    // at most once }
         */
        info(const info& other) {
            if (other.info_ == MPI_INFO_NULL) {
                // copy an info object which refers to MPI_INFO_NULL
                info_ = MPI_INFO_NULL;
                is_freeable_ = other.is_freeable_;
            } else {
                // copy normal info object
                MPI_Info_dup(other.info_, &info_);
                is_freeable_ = true;
            }
        }
        /**
         * @brief Move constructor. Constructs the info object with the contents of @p other using move semantics.
         * @details Retains @p other's [key, value]-pair ordering.
         * @param[in] other another info object to be used as source to initialize the [key, value]-pairs of this info object with
         *
         * @post The newly constructed info object is in a valid state iff @p other is in a valid state.
         * @post @p other is in the moved-from state, i.e. it refers to
         *       [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post All iterators referring to @p other remain valid, but now refer to `*this`.
         * @attention Only a limited number of member functions can be called on an info object referring to
         *            [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) (aka moved-from state):
         *            - the destructor ~info()
         *            - all assignment operators: @ref mpicxx::info::operator=(const mpicxx::info&),
         *              @ref mpicxx::info::operator=(mpicxx::info&&) and @ref mpicxx::info::operator=(std::initializer_list<value_type>)
         *            - the swap member function: @ref mpicxx::info::swap(mpicxx::info&)
         *            - the relational operators: @ref mpicxx::info::operator==(const mpicxx::info&, const mpicxx::info&) and
         *              mpicxx::info::operator!=(const mpicxx::info&, const mpicxx::info&)
         *            - all static member functions: @ref mpicxx::info::max_size(), @ref mpicxx::info::max_key_size() and
         *              @ref mpicxx::info::max_value_size()
         *            - all getters: @ref mpicxx::info::get(), @ref mpicxx::info::get() const and @ref mpicxx::info::freeable() const
         */
        info(info&& other) noexcept : info_(std::move(other.info_)), is_freeable_(std::move(other.is_freeable_)) {
            // set other to the moved-from state (referring to MPI_INFO_NULL)
            other.info_ = MPI_INFO_NULL;
            other.is_freeable_ = false;
        }
        /**
         * @brief Constructs the info object with the contents of the range [@p first, @p last).
         * @details If multiple [key, value]-pairs in the range share the same key, the **last** occurrence determines the final value.
         *
         *    Example:
         *          @snippet examples/info/constructor.cpp constructor iterator range
         *          Results in the following [key, value]-pairs stored in the info object (not necessarily in this order): \n
         *          `["key1", "value1_override"]`, `["key2", "value2"]` and `["key3", "value3"]`
         * @tparam InputIt must meet the [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) requirements
         * @param[in] first iterator to the first [key, value]-pair in the range
         * @param[in] last iterator one-past the last [key, value]-pair in the range
         *
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid range, i.e. @p first must be less or equal than @p last.
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre The length of **any** value **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post The newly constructed info object is in a valid state.
         *
         * @assert_precondition{ If any key or value exceed their size limit. \n
         *                       If @p first and @p last don't denote a valid range. }
         *
         * @calls{
         * int MPI_Info_create(MPI_Info *info);                                    // exactly once
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);    // exactly 'last - first' times
         * }
         */
        template <std::input_iterator InputIter>
        info(InputIter first, InputIter last) : info() {
            // default construct the info object via the default constructor
            // add all [key, value]-pairs
            this->insert_or_assign(first, last);
        }
        /**
         * @brief Constructs the info object with the contents of the
         *        [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) @p init.
         * @details If multiple [key, value]-pairs in the range share the same key, the **last** occurrence determines the final value.
         *
         *    Example:
         *          @snippet examples/info/constructor.cpp constructor initializer list
         *          Results in the following [key, value]-pairs stored in the info object (not necessarily in this order):\n
         *          `["key1", "value1_override"]`, `["key2", "value2"]` and `["key3", "value3"]`
         * @param[in] init [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) to initialize the
         *                 [key, value]-pairs of the info object with
         *
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre The length of **any** value **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post The newly constructed info object is in a valid state.
         *
         * @assert_precondition{ If any key or value exceed their size limit. }
         *
         * @calls{
         * int MPI_Info_create(MPI_Info *info);                                    // exactly once
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);    // exactly 'init.size()' times
         * }
         */
        info(std::initializer_list<value_type> init) : info() {
            // default construct the info object via the default constructor
            // add all [key, value]-pairs
            this->insert_or_assign(init);
        }
        /**
         * @brief Constructs the info object with the contents of the parameter pack @p args.
         * @details If multiple [key, value]-pairs in the range share the same key, the **last** occurrence determines the final value.
         *
         *    Example:
         *          @snippet examples/info/constructor.cpp constructor parameter pack
         *          Results in the following [key, value]-pairs stored in the info object (not necessarily in this order):\n
         *          `["key1", "value1_override"]`, `["key2", "value2"]` and `["key3", "value3"]`
         * @tparam T must meed the @ref mpicxx::detail::is_pair requirements and must not be empty
         * @param[in] args an arbitrary number (but at least 1) of [key, value]-pairs
         *
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre The length of **any** value **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post The newly constructed info object is in a valid state.
         *
         * @assert_precondition{ If any key or value exceed their size limit. }
         *
         * @calls{
         * int MPI_Info_create(MPI_Info *info);                                    // exactly once
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);    // exactly 'sizeof...(T)' times
         * }
         */
        template <detail::is_pair... T>
        explicit info(T&&... args) requires (sizeof...(T) > 0) : info() {
            // default construct the info object via the default constructor
            // add all [key, value]-pairs
            this->insert_or_assign(std::forward<T>(args)...);
        }
        /**
         * @brief Wrap a [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object in an @ref mpicxx::info object.
         * @param[in] other the raw [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object
         * @param[in] is_freeable mark whether the [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object
         *                        wrapped in this info object should be automatically freed at the end of its lifetime
         *
         * @post The newly constructed info object is in a valid state iff @p other isn't
         *       [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @attention If @p is_freeable is set to `false`, **the user** has to ensure that the
         *            [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object @p other gets properly
         *            freed (via a call to [*MPI_Info_free*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm)) at the end
         *            of its lifetime.
         * @attention Changing the underlying [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object
         *            **does not** change the value of `*this`!:
         *            @snippet examples/info/constructor.cpp constructor MPI_Info
         *
         * @assert_sanity{ If @p other equals to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) or
         *                 [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) **and** @p is_freeable is set
         *                 to `true`. }
         */
        constexpr info(MPI_Info other, const bool is_freeable) noexcept : info_(other), is_freeable_(is_freeable) {
            MPICXX_ASSERT_SANITY(!(other == MPI_INFO_NULL && is_freeable == true), "'MPI_INFO_NULL' shouldn't be marked as freeable!");
            MPICXX_ASSERT_SANITY(!(other == MPI_INFO_ENV && is_freeable == true), "'MPI_INFO_ENV' shouldn't be marked as freeable!");
        }
        /**
         * @brief Destructs the info object.
         * @details Calls [*MPI_Info_free*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) if and only if the info object
         *          is marked freeable. Only objects created through @ref mpicxx::info(MPI_Info, const bool) can be marked as non-freeable
         *          (or info objects which are moved-from such objects). \n
         *          For example @ref mpicxx::info::env is **non-freeable** due to the fact that the MPI runtime system would crash if
         *          [*MPI_Info_free*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) is called with
         *          [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @pre No attempt to automatically free [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) or
         *      [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) **must** be made.
         * @post Invalidates **all** iterators referring to `*this`.
         *
         * @assert_precondition{ If an attempt is made to free
         *                       [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) or
         *                       [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls{ int MPI_Info_free(MPI_info *info);    // at most once }
         */
        ~info() {
            // destroy info object if marked as freeable
            if (is_freeable_) {
                MPICXX_ASSERT_PRECONDITION(info_ != MPI_INFO_NULL, "Attempt to free a 'MPI_INFO_NULL' object!");
                MPICXX_ASSERT_PRECONDITION(info_ != MPI_INFO_ENV, "Attempt to free a 'MPI_INFO_ENV' object!");

                MPI_Info_free(&info_);
            }
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            assignment operators                                            //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name assignment operators
        ///@{
        /**
         * @brief Copy assignment operator. Replaces the contents with a copy of the contents of @p other.
         * @details Retains @p rhs's [key, value]-pair ordering. Gracefully handles self-assignment.
         * @param[in] rhs another info object to use as data source
         * @return `*this`
         *
         * @pre No attempt to automatically free [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) or
         *      [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) as `*this` **must** be made.
         * @post The assigned to info object is in a valid state iff @p other is in a valid state.
         * @attention Every copied info object (except if `other.get() == MPI_INFO_NULL`)  is marked **freeable** independent of the
         *            **freeable** state of the copied-from info object.
         *
         * @assert_precondition{ If an attempt is made to free
         *                       [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) or
         *                       [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) as `*this`. }
         * @assert_sanity{ If `*this` and @p rhs are the same info object. }
         *
         * @calls{
         * int MPI_Info_free(MPI_info *info);                     // at most once
         * int MPI_Info_dup(MPI_info info, MPI_info *newinfo);    // at most once
         * }
         */
        info& operator=(const info& rhs) {
            MPICXX_ASSERT_SANITY(!this->identical(rhs), "Attempt to perform a \"self copy assignment\"!");

            // check against self-assignment
            if (this != std::addressof(rhs)) {
                // delete current MPI_Info object if and only if it is marked as freeable
                if (is_freeable_) {
                    MPICXX_ASSERT_PRECONDITION(info_ != MPI_INFO_NULL, "Attempt to free a 'MPI_INFO_NULL' object!");
                    MPICXX_ASSERT_PRECONDITION(info_ != MPI_INFO_ENV, "Attempt to free a 'MPI_INFO_ENV' object!");

                    MPI_Info_free(&info_);
                }
                // copy rhs info object
                if (rhs.info_ == MPI_INFO_NULL) {
                    // copy an info object which refers to MPI_INFO_NULL
                    info_ = MPI_INFO_NULL;
                    is_freeable_ = rhs.is_freeable_;
                } else {
                    // copy normal info object
                    MPI_Info_dup(rhs.info_, &info_);
                    is_freeable_ = true;
                }
            }
            return *this;
        }
        /**
         * @brief Move assignment operator. Replaces the contents with contents of @p other using move semantics.
         * @details Retains @p rhs's [key, value]-pair ordering. Does **not** handle self-assignment
         *          (as of https://isocpp.org/wiki/faq/assignment-operators).
         * @param[in] rhs another info object to use as data source
         * @return `*this`
         *
         * @pre No attempt to automatically free [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) or
         *      [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) as `*this` **must** be made.
         * @post @p rhs is in the moved-from state, i.e. it refers to
         *       [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post All iterators referring to @p rhs remain valid, but now refer to `*this`.
         * @attention Only a limited number of member functions can be called on an info object referring to
         *            [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) (aka moved-from state):
         *            - the destructor ~info()
         *            - all assignment operators: @ref mpicxx::info::operator=(const mpicxx::info&),
         *              @ref mpicxx::info::operator=(mpicxx::info&&) and @ref mpicxx::info::operator=(std::initializer_list<value_type>)
         *            - the swap member function: @ref mpicxx::info::swap(mpicxx::info&)
         *            - the relational operators: @ref mpicxx::info::operator==(const mpicxx::info&, const mpicxx::info&) and
         *              mpicxx::info::operator!=(const mpicxx::info&, const mpicxx::info&)
         *            - all static member functions: @ref mpicxx::info::max_size(), @ref mpicxx::info::max_key_size() and
         *              @ref mpicxx::info::max_value_size()
         *            - all getters: @ref mpicxx::info::get(), @ref mpicxx::info::get() const and @ref mpicxx::info::freeable() const
         *
         * @assert_precondition{ If an attempt is made to free
         *                       [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) or
         *                       [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) as `*this`. }
         * @assert_sanity{ `*this` and @p rhs are the same info object. }
         *
         * @calls{ int MPI_Info_free(MPI_info *info);    // at most once }
         */
        info& operator=(info&& rhs) {
            MPICXX_ASSERT_SANITY(!this->identical(rhs), "Attempt to perform a \"self move assignment\"!");

            // delete current MPI_Info object if and only if it is marked as freeable
            if (is_freeable_) {
                MPICXX_ASSERT_PRECONDITION(info_ != MPI_INFO_NULL, "Attempt to free a 'MPI_INFO_NULL' object!");
                MPICXX_ASSERT_PRECONDITION(info_ != MPI_INFO_ENV, "Attempt to free a 'MPI_INFO_ENV' object!");

                MPI_Info_free(&info_);
            }
            // transfer ownership
            info_ = std::move(rhs.info_);
            is_freeable_ = std::move(rhs.is_freeable_);
            // set rhs to the moved-from state (referring to MPI_INFO_NULL)
            rhs.info_ = MPI_INFO_NULL;
            rhs.is_freeable_ = false;
            return *this;
        }
        /**
         * @brief Replaces the contents with those identified by the
         *        [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) @p ilist.
         * @details If multiple [key, value]-pairs in the range share the same key, the **last** occurrence determines the final value.
         * @param[in] ilist [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) to initialize the
         *                  [key, value]-pairs of the info object with
         * @return `*this`
         *
         * @pre No attempt to automatically free [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) or
         *      [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) as `*this` **must** be made.
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre The length of **any** value **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If an attempt is made to free
         *                       [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) or
         *                       [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) as `*this`. \n
         *                       If any key or value exceed their size limit. }
         *
         * @calls{
         * int MPI_Info_free(MPI_info *info);                                      // at most once
         * int MPI_Info_create(MPI_Info *info);                                    // exactly once
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);    // exactly 'init.size()' times
         * }
         */
        info& operator=(std::initializer_list<value_type> ilist) {
            // delete current MPI_Info object iff it is marked as freeable and in a valid state
            if (is_freeable_) {
                MPICXX_ASSERT_PRECONDITION(info_ != MPI_INFO_NULL, "Attempt to free a 'MPI_INFO_NULL' object!");
                MPICXX_ASSERT_PRECONDITION(info_ != MPI_INFO_ENV, "Attempt to free a 'MPI_INFO_ENV' object!");

                MPI_Info_free(&info_);
            }
            // recreate the info object
            MPI_Info_create(&info_);
            is_freeable_ = true;
            // add all [key, value]-pairs
            this->insert_or_assign(ilist);
            return *this;
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  iterators                                                 //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name iterators
        ///@{
        /**
         * @brief Returns an @ref mpicxx::info::iterator to the first [key, value]-pair of the info object.
         * @details If the info object is empty, the returned @ref mpicxx::info::iterator will be equal to @ref mpicxx::info::end().
         * @return @ref mpicxx::info::iterator to the first [key, value]-pair
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls_ref{ For *MPI* functions called while using an iterator see the @ref mpicxx::info::iterator_impl documentation. }
         */
        [[nodiscard]]
        iterator begin() {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to create an iterator from an info object referring to 'MPI_INFO_NULL'!");

            return iterator(info_, 0);
        }
        /**
         * @brief Returns an @ref mpicxx::info::iterator to the element following the last [key, value]-pair of the info object.
         * @details This element acts as a placeholder; attempting to access it results in **undefined behavior**.
         * @return @ref mpicxx::info::iterator to the element following the last [key, value]-pair
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls_ref{
         * @code{.cpp} int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys);    // exactly once @endcode
         * For *MPI* functions called while using an iterator see the @ref mpicxx::info::iterator_impl documentation.
         * }
         */
        [[nodiscard]]
        iterator end() {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to create an iterator from an info object referring to 'MPI_INFO_NULL'!");

            return iterator(info_, this->size());
        }
        /**
         * @brief Returns a @ref mpicxx::info::const_iterator to the first [key, value]-pair of the info object.
         * @details If the info object is empty, the returned @ref mpicxx::info::const_iterator will be equal to @ref mpicxx::info::cend().
         * @return @ref mpicxx::info::const_iterator to the first [key, value]-pair
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls_ref{ For *MPI* functions called while using an iterator see the @ref mpicxx::info::iterator_impl documentation. }
         */
        [[nodiscard]]
        const_iterator begin() const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to create a const_iterator from an info object referring to 'MPI_INFO_NULL'!");

            return const_iterator(info_, 0);
        }
        /**
         * @brief Returns a @ref mpicxx::info::const_iterator to the element following the last [key, value]-pair of the info object.
         * @details This element acts as a placeholder; attempting to access it results in **undefined behavior**.
         * @return @ref mpicxx::info::const_iterator to the element following the last [key, value]-pair
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls_ref{
         * @code{.cpp} int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys);    // exactly once @endcode
         * For *MPI* functions called while using an iterator see the @ref mpicxx::info::iterator_impl documentation.
         * }
         */
        [[nodiscard]]
        const_iterator end() const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to create a const_iterator from an info object referring to 'MPI_INFO_NULL'!");

            return const_iterator(info_, this->size());
        }
        /**
         * @copydoc begin() const
         */
        [[nodiscard]]
        const_iterator cbegin() const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to create a const_iterator from an info object referring to 'MPI_INFO_NULL'!");

            return const_iterator(info_, 0);
        }
        /**
         * @copydoc end() const
         */
        [[nodiscard]]
        const_iterator cend() const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to create a const_iterator from an info object referring to 'MPI_INFO_NULL'!");

            return const_iterator(info_, this->size());
        }

        /**
         * @brief Returns a @ref mpicxx::info::reverse_iterator to the first [key, value]-pair of the reversed info object.
         * @details It corresponds to the last [key, value]-pair of the non-reversed info object.
         *          If the info object is empty, the returned @ref mpicxx::info::reverse_iterator will be equal to @ref mpicxx::info::rend().
         * @return @ref mpicxx::info::reverse_iterator to the first [key, value]-pair
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls_ref{
         * @code{.cpp} int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys);    // exactly once @endcode
         * For *MPI* functions called while using an iterator see the @ref mpicxx::info::iterator_impl documentation.
         * }
         */
        [[nodiscard]]
        reverse_iterator rbegin() {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to create a reverse_iterator from an info object referring to 'MPI_INFO_NULL'!");

            return std::make_reverse_iterator(this->end());
        }
        /**
         * @brief Returns a @ref mpicxx::info::reverse_iterator to the element following the last [key, value]-pair of the reversed info
         *        object.
         * @details It corresponds to the element preceding the first [key, value]-pair of the non-reversed info object.
         *          This element acts as a placeholder, attempting to access it results in **undefined behavior**.
         * @return @ref mpicxx::info::reverse_iterator to the element following the last [key, value]-pair
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls_ref{ For *MPI* functions called while using an iterator see the @ref mpicxx::info::iterator_impl documentation. }
         */
        [[nodiscard]]
        reverse_iterator rend() {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to create a reverse_iterator from an info object referring to 'MPI_INFO_NULL'!");

            return std::make_reverse_iterator(this->begin());
        }
        /**
         * @brief Returns a @ref mpicxx::info::const_reverse_iterator to the first [key, value]-pair of the reversed info object.
         * @details It corresponds to the last [key, value]-pair of the non-reversed info object.
         *          If the info object is empty, the returned @ref mpicxx::info::const_reverse_iterator will be equal to
         *          @ref mpicxx::info::crend().
         * @return @ref mpicxx::info::const_reverse_iterator to the first [key, value]-pair
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls_ref{
         * @code{.cpp} int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys);    // exactly once @endcode
         * For *MPI* functions called while using an iterator see the @ref mpicxx::info::iterator_impl documentation.
         * }
         */
        [[nodiscard]]
        const_reverse_iterator rbegin() const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to create a const_reverse_iterator from an info object referring to 'MPI_INFO_NULL!");

            return std::make_reverse_iterator(this->cend());
        }
        /**
         * @brief Returns a @ref mpicxx::info::const_reverse_iterator to the element following the last [key, value]-pair of the reversed
         *        info object.
         * @details It corresponds to the element preceding the first [key, value]-pair of the non-reversed info object.
         *          This element acts as a placeholder, attempting to access it results in **undefined behavior**.
         * @return @ref mpicxx::info::const_reverse_iterator to the element following the last [key, value]-pair
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls_ref{ For *MPI* functions called while using an iterator see the @ref mpicxx::info::iterator_impl documentation. }
         */
        [[nodiscard]]
        const_reverse_iterator rend() const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to create a const_reverse_iterator from an info object referring to 'MPI_INFO_NULL'!");

            return std::make_reverse_iterator(this->cbegin());
        }
        /**
         * @copydoc rbegin() const
         */
        [[nodiscard]]
        const_reverse_iterator crbegin() const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to create a const_reverse_iterator from an info object referring to 'MPI_INFO_NULL'!");

            return std::make_reverse_iterator(this->cend());
        }
        /**
         * @copydoc rend() const
         */
        [[nodiscard]]
        const_reverse_iterator crend() const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to create a const_reverse_iterator from an info object referring to 'MPI_INFO_NULL'!");

            return std::make_reverse_iterator(this->cbegin());
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  capacity                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name capacity
        ///@{
        /**
         * @brief Checks if the info object has no [key, value]-pairs, i.e. whether `begin() == end()`.
         * @return `true` if the info object is empty, `false` otherwise
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls{ int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);    // exactly once }
         */
        [[nodiscard]]
        bool empty() const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

            return this->size() == 0;
        }
        /**
         * @brief Returns the number of [key, value]-pairs in the info object, i.e.
         *        [`std::distance(begin(), end())`](https://en.cppreference.com/w/cpp/iterator/distance).
         * @return the number of [key, value]-pairs in the info object
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls{ int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);    // exactly once }
         */
        [[nodiscard]]
        size_type size() const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

            int nkeys;
            MPI_Info_get_nkeys(info_, &nkeys);
            return static_cast<size_type>(nkeys);
        }
        /**
         * @brief Returns the maximum number of [key, value]-pairs an info object is able to hold due to system or library implementation
         *        limitations, \n
         *        i.e. [`std::distance(begin(), end())`](https://en.cppreference.com/w/cpp/iterator/distance) for the largest info object.
         * @return maximum number of [key, value]-pairs
         * @nodiscard
         *
         * @attention This value typically reflects the theoretical limit on the size of the info object, at most
         *            [`std::numeric_limits<difference_type>::%max()`](https://en.cppreference.com/w/cpp/types/numeric_limits). At runtime,
         *            the size of the info object may be limited to a value smaller than @ref mpicxx::info::max_size() by the amount of RAM
         *            available.
         */
        [[nodiscard]]
        static constexpr size_type max_size() {
            return std::numeric_limits<difference_type>::max();
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  modifiers                                                 //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name modifiers
        ///@{
        /**
         * @brief Access the value associated with the given @p key including bounds checks.
         * @details Returns a proxy class, which is used to distinguish between read and write accesses.
         *
         *    Example:
         *          @snippet examples/info/access.cpp access at
         * @tparam must meet the @ref mpicxx::detail::is_string requirements
         * @param[in] key the @p key of the [key, value]-pair to find
         * @return a proxy object
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre The @p key **must** already exist, otherwise a
         *      [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) exception will be thrown.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p key exceeds its size limit. }
         *
         * @throws std::out_of_range if the info object does not have a [key, value]-pair with the specified @p key
         *
         * @calls_ref{
         * @code{.cpp}
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);    // exactly once
         * @endcode
         * For *MPI* functions called while using a proxy see the @ref mpicxx::info::proxy documentation.
         * }
         */
        template <detail::is_string T>
        proxy at(T&& key) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                    "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)",
                    detail::convert_to_string_size(key), MPI_MAX_INFO_KEY);

            // check whether the key exists
            if (!this->key_exists(key)) {
                // key doesn't exist
                throw std::out_of_range(fmt::format("{} doesn't exist!", std::forward<T>(key)));
            }
            // create proxy object and forward key
            return proxy(info_, std::forward<decltype(key)>(key));
        }
        /**
         * @brief Access the value associated with the given @p key including bounds checks.
         * @details Example:
         *          @snippet examples/info/access.cpp access const at
         * @param[in] key the @p key of the [key, value]-pair to find
         * @return the value associated with @p key
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre The @p key **must** already exist, otherwise a
         *      [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) exception will be thrown.
         * @attention This const overload does **not** return a proxy object but a
         *            [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string)!
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p key exceeds its size limit. }
         *
         * @throws std::out_of_range if the info object does not have a [key, value]-pair with the specified @p key
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);       // exactly once
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);    // at most once
         * }
         */
        std::string at(const std::string_view key) const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                    "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

            // get the length of the value associated with key
            int valuelen, flag;
            MPI_Info_get_valuelen(info_, key.data(), &valuelen, &flag);
            // check whether the key exists
            if (!static_cast<bool>(flag)) {
                // key doesn't exist
                throw std::out_of_range(fmt::format("{} doesn't exist!", std::forward<decltype(key)>(key)));
            }
            // get the value associated with key
            std::string value(valuelen, ' ');
            MPI_Info_get(info_, key.data(), valuelen, value.data(), &flag);
            return value;
        }
        /**
         * @brief Access the value associated with the given @p key.
         * @details Returns a proxy class which is used to distinguish between read and write access. \n
         *          `mpicxx::info::operator[]` is non-const because it inserts the @p key if it doesn't exist.
         *          If this behavior is undesirable or if the container is const, `mpicxx::info::at()` may be used.
         *
         *    Example:
         *          @snippet examples/info/access.cpp access operator overload
         * @tparam T must meet the @ref mpicxx::detail::is_string requirements
         * @param[in] key the @p key of the [key, value]-pair to find
         * @return a proxy object
         *
         * @pre `*this` **may not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this` are
         *       invalidated, if an insertion took place. \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the insertion point remain valid, all
         *       other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p key exceeds its size limit. }
         *
         * @calls_ref{ For *MPI* functions called while using a proxy see the @ref mpicxx::info::proxy documentation. }
         */
        template <detail::is_string T>
        proxy operator[](T&& key) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                    "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)",
                    detail::convert_to_string_size(key), MPI_MAX_INFO_KEY);

            // create proxy object and forward key
            return proxy(info_, std::forward<T>(key));
        }

        /**
         * @brief Insert the given [key, value]-pair if the info object doesn't already contain a [key, value]-pair with an equivalent key.
         * @param[in] key @p key of the [**key**, value]-pair to insert
         * @param[in] value @p value of the [key, **value**]-pair to insert
         * @return a pair consisting of an iterator to the inserted [key, value]-pair (or the one that prevented the insertion) and a `bool`
         *         denoting whether the insertion took place
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre **Both** @p key **and** @p value **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre The @p value's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post As of the [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         *       are invalidated, if an insertion took place (i.e. the returned `bool` is `true`). \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the insertion point remain valid, all
         *       other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p key or @p value exceed their size limit. }
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);    // exactly once
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                    // at most once
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                      // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                               // at most 'this->size()' times
         * }
         */
        std::pair<iterator, bool> insert(const std::string_view key, const std::string_view value) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                    "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(value, MPI_MAX_INFO_VAL),
                    "Illegal info value: 0 < {} < {} (MPI_MAX_INFO_VAL)", value.size(), MPI_MAX_INFO_VAL);

            // check whether the key exists
            const bool key_already_exists = this->key_exists(key);
            if (!key_already_exists) {
                // key doesn't exist -> add new [key, value]-pair
                MPI_Info_set(info_, key.data(), value.data());
            }
            // search position of the key and return an iterator
            return std::make_pair(iterator(info_, this->find_pos(key, this->size())), !key_already_exists);
        }
        /**
         * @brief Inserts all [key, value]-pairs from the range [@p first, @p last) if the info object does not already contain a
         *        [key, value]-pair with an equivalent key.
         * @details If multiple [key, value]-pairs in the range have the same key, the **first** occurrence determines the final value.
         * @tparam InputIt must meet the [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) requirements
         * @param[in] first iterator to the first [key, value]-pair in the range
         * @param[in] last iterator one-past the last [key, value]-pair in the range
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid range, i.e. @p first must be less or equal than @p last.
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre The length of **any** value **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         *       are invalidated, if an insertion took place. \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the first insertion point remain valid,
         *       all other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p first and @p last don't denote a valid range. \n
         *                       If any key or value exceed their size limit. }
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);    // exactly 'last - first' times
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                    // at most 'last - first' times
         * }
         */
        template <std::input_iterator InputIt>
        void insert(InputIt first, InputIt last) requires (!detail::is_c_string<InputIt>) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");

            // try to insert every element in the range [first, last)
            for (; first != last; ++first) {
                // retrieve element
                const value_type& pair = *first;

                MPICXX_ASSERT_PRECONDITION(this->legal_string_size(pair.first, MPI_MAX_INFO_KEY),
                        "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", pair.first.size(), MPI_MAX_INFO_KEY);
                MPICXX_ASSERT_PRECONDITION(this->legal_string_size(pair.second, MPI_MAX_INFO_VAL),
                        "Illegal info value: 0 < {} < {} (MPI_MAX_INFO_VAL)", pair.second.size(), MPI_MAX_INFO_VAL);

                // check whether the key exists
                if (!this->key_exists(pair.first)) {
                    // key doesn't exist -> add new [key, value]-pair
                    MPI_Info_set(info_, pair.first.data(), pair.second.data());
                }
            }
        }
        /**
         * @brief Inserts all [key, value]-pairs from the
         *        [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) @p ilist if the info object does
         *        not already contain a [key, value]-pair with an equivalent key.
         * @details If multiple [key, value]-pairs in the
         *          [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) have the same key, the **first**
         *          occurrence determines the final value.
         * @param[in] ilist [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) to insert the
         *                  [key, value]-pairs from
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre The length of **any** value **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         *       are invalidated, if an insertion took place. \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the first insertion point remain valid,
         *       all other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If any key or value exceed their size limit. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                      // exactly 'ilist.size()' times
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);    // at most 'ilist.size()' times
         * }
         */
        void insert(std::initializer_list<value_type> ilist) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

            this->insert(ilist.begin(), ilist.end());
        }
        /**
         * @brief Inserts all [key, value]-pairs from the parameter pack @p args if the info object does not already contain a
         *        [key, value]-pair with an equivalent key.
         * @details If multiple [key, value]-pairs in the parameter pack have the same key, the **first** occurrence determines the final
         *          value.
         * @tparam T must meed the @ref mpicxx::detail::is_pair requirements and must not be empty
         * @param[in] args an arbitrary number (but at least 1) of [key, value]-pairs
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre The length of **any** value **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         *       are invalidated, if an insertion took place. \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the first insertion point remain valid,
         *       all other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If any key or value exceed their size limit. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                      // exactly 'sizeof...(T)' times
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);    // at most 'sizeof...(T)' times
         * }
         */
        template <detail::is_pair... T>
        void insert(T&&... args) requires (sizeof...(T) > 0) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

            ([&](auto&& pair) {
                MPICXX_ASSERT_PRECONDITION(this->legal_string_size(pair.first, MPI_MAX_INFO_KEY),
                        "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)",
                        detail::convert_to_string_size(pair.first), MPI_MAX_INFO_KEY);
                MPICXX_ASSERT_PRECONDITION(this->legal_string_size(pair.second, MPI_MAX_INFO_VAL),
                        "Illegal info value: 0 < {} < {} (MPI_MAX_INFO_VAL)",
                        detail::convert_to_string_size(pair.second), MPI_MAX_INFO_VAL);

                using pair_t = std::remove_cvref_t<decltype(pair)>;
                // check whether the key exists
                if (!this->key_exists(pair.first)) {
                    // key doesn't exist -> add new [key, value]-pair
                    MPI_Info_set(info_, detail::convert_to_char_pointer(std::forward<pair_t>(pair).first),
                                        detail::convert_to_char_pointer(std::forward<pair_t>(pair).second));
                }
            }(std::forward<T>(args)), ...);
        }

        /**
         * @brief Insert or assign the given [key, value]-pair to the info object.
         * @param[in] key @p key of the [**key**, value]-pair to insert
         * @param[in] value @p value of the [key, **value**]-pair to insert
         * @return a pair consisting of an iterator to the inserted or assigned [key, value]-pair and a `bool`
         *         denoting whether the insertion (`true`) or the assignment (`false`) took place
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre **Both** @p key **and** @p value **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre The @p value's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         *       are invalidated, if an insertion took place (i.e. the returned `bool` is `true`). \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the insertion point remain valid, all
         *       other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p key or @p value exceed their size limit. }
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);    // exactly once
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                    // exactly once
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                      // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                               // at most 'this->size()' times
         * }
         */
        std::pair<iterator, bool> insert_or_assign(const std::string_view key, const std::string_view value) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                    "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(value, MPI_MAX_INFO_VAL),
                    "Illegal info value: 0 < {} < {} (MPI_MAX_INFO_VAL)", value.size(), MPI_MAX_INFO_VAL);

            // check whether an insertion or assignment will take place
            const bool key_already_exists = this->key_exists(key);
            // updated (i.e. insert or assign) the [key, value]-pair
            MPI_Info_set(info_, key.data(), value.data());
            // search position of the key and return an iterator
            return std::make_pair(iterator(info_, this->find_pos(key, this->size())), !key_already_exists);
        }
        /**
         * @brief Inserts or assigns [key, value]-pairs from range [@p first, @p last) to the info object.
         * @details If multiple [key, value]-pairs in the range have the same key, the **last** occurrence determines the final value.
         * @tparam InputIt must meet the [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) requirements
         * @param[in] first iterator to the first [key, value]-pair in the range
         * @param[in] last iterator one-past the last [key, value]-pair in the range
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid range, i.e. @p first must be less or equal than @p last.
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre The length of **any** value **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         *       are invalidated, if an insertion took place. \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the first insertion point remain valid,
         *       all other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p first and @p last don't denote a valid range. \n
         *                       If any key or value exceed their size limit. }
         *
         * @calls{ int MPI_Info_set(MPI_Info info, const char *key, const char *value);    // exactly 'last - first' times }
         */
        template <std::input_iterator InputIt>
        void insert_or_assign(InputIt first, InputIt last) requires (!detail::is_c_string<InputIt>) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");

            // insert or assign every element in the range [first, last)
            for (; first != last; ++first) {
                // retrieve element
                const value_type& pair = *first;
                MPICXX_ASSERT_PRECONDITION(this->legal_string_size(pair.first, MPI_MAX_INFO_KEY),
                        "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", pair.first.size(), MPI_MAX_INFO_KEY);
                MPICXX_ASSERT_PRECONDITION(this->legal_string_size(pair.second, MPI_MAX_INFO_VAL),
                        "Illegal info value: 0 < {} < {} (MPI_MAX_INFO_VAL)", pair.second.size(), MPI_MAX_INFO_VAL);

                // insert or assign [key, value]-pair
                MPI_Info_set(info_, pair.first.data(), pair.second.data());
            }
        }
        /**
         * @brief Inserts or assigns [key, value]-pairs from the
         *        [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) @p ilist to the info object.
         * @details If multiple [key, value]-pairs in the
         *          [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) have the same key, the **last**
         *          occurrence determines the final value.
         * @param[in] ilist [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) to insert or assign the
         *                  [key, value]-pairs from
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre The length of **any** value **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         *       are invalidated, if an insertion took place. \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the first insertion point remain valid,
         *       all other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If any key or value exceed their size limit. }
         *
         * @calls{ int MPI_Info_set(MPI_Info info, const char *key, const char *value);    // exactly 'ilist.size()' times }
         */
        void insert_or_assign(std::initializer_list<value_type> ilist) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

            this->insert_or_assign(ilist.begin(), ilist.end());
        }
        /**
         * @brief Inserts or assigns [key, value]-pairs from the parameter pack @p args to the info object.
         * @details If multiple [key, value]-pairs in the parameter pack have the same key, the **last** occurrence determines the final
         *          value.
         * @tparam T must meed the @ref mpicxx::detail::is_pair requirements and must not be empty
         * @param[in] args an arbitrary number (but at least 1) of [key, value]-pairs
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre The length of **any** value **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         *       are invalidated, if an insertion took place. \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the first insertion point remain valid,
         *       all other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If any key or value exceed their size limit. }
         *
         * @calls{ int MPI_Info_set(MPI_Info info, const char *key, const char *value);    // exactly 'sizeof...(T)' times }
         */
        template <detail::is_pair... T>
        void insert_or_assign(T&&... args) requires (sizeof...(T) > 0) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

            ([&](auto&& pair) {
                MPICXX_ASSERT_PRECONDITION(this->legal_string_size(pair.first, MPI_MAX_INFO_KEY),
                        "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)",
                        detail::convert_to_string_size(pair.first), MPI_MAX_INFO_KEY);
                MPICXX_ASSERT_PRECONDITION(this->legal_string_size(pair.second, MPI_MAX_INFO_VAL),
                        "Illegal info value: 0 < {} < {} (MPI_MAX_INFO_VAL)",
                        detail::convert_to_string_size(pair.second), MPI_MAX_INFO_VAL);

                using pair_t = std::remove_cvref_t<decltype(pair)>;
                MPI_Info_set(info_, detail::convert_to_char_pointer(std::forward<pair_t>(pair).first),
                                    detail::convert_to_char_pointer(std::forward<pair_t>(pair).second));
            }(std::forward<T>(args)), ...);
        }

        /**
         * @brief Erase all [key, value]-pairs from the info object.
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post The info object is empty, i.e. `this->size() == 0` respectively `this->empty() == true`.
         * @post Invalidates **all** iterators referring to `*this`.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);           // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);    // exactly 'this->size()' times
         * int MPI_Info_delete(MPI_Info info, const char *key);         // exactly 'this->size()' times
         * }
         */
        void clear() {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

            const size_type size = this->size();
            char key[MPI_MAX_INFO_KEY];
            // repeat nkeys times and always remove the first element
            for (size_type i = 0; i < size; ++i) {
                MPI_Info_get_nthkey(info_, 0, key);
                MPI_Info_delete(info_, key);
            }
        }

        /**
         * @brief Removes the [key, value]-pair at @p pos.
         * @details The iterator @p pos must be valid and **dereferenceable**. Thus the @ref mpicxx::info::end() iterator (which is valid,
         *          but is not dereferenceable) cannot be used as a value for @p pos.
         * @param[in] pos iterator to the [key, value]-pair to remove
         * @return iterator following the removed [key, value]-pair (= position of @p pos prior to removal); \n
         *         if @p pos refers to the last [key, value]-pair, then the @ref mpicxx::info::end() iterator is returned
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p pos **must** refer to `*this` info object.
         * @pre The position denoted by @p pos **must** be in the half-open interval [0, `this->size()`).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         *       are invalidated. \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the point of erase remain valid, all
         *       other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p pos does not refer to `*this` info object. \n
         *                       If attempting an illegal dereferencing. }
         *
         * @calls{
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);    // exactly once
         * int MPI_Info_delete(MPI_Info info, const char *key);         // exactly once
         * }
         */
        iterator erase(const_iterator pos) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_info_iterator(pos), "Attempt to use an info iterator referring to another info object!");
            MPICXX_ASSERT_PRECONDITION(this->info_iterator_valid(pos), "Attempt to dereference a {} iterator!", pos.state());

            char key[MPI_MAX_INFO_KEY];
            MPI_Info_get_nthkey(info_, pos.pos_, key);
            MPI_Info_delete(info_, key);
            return iterator(info_, pos.pos_);
        }
        /**
         * @brief Removes all [key, value]-pairs in the range [@p first, @p last).
         * @details [@p first, @p last) must be a valid range in `*this`. The iterator @p first does not need to be dereferenceable if
         *          `first == last`: erasing an empty range is a no-op.
         *
         *          The [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) only guarantees that the number of a
         *          given key does not change **as long as** no call to
         *          [*MPI_Info_set*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) or
         *          [*MPI_Info_delete*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) is made.
         *
         *          Therefore (to be compliant with the standard) this function performs two steps:
         *          1. Save all keys of the [key, value]-pairs contained in [@p first, @p last) in a
         *             [`std::vector<std::string>>`](https://en.cppreference.com/w/cpp/container/vector).
         *          2. Delete all [key, value]-pairs in the info object @p c with a key contained in the previously created vector.
         * @param[in] first iterator to the first [key, value]-pair in the range
         * @param[in] last iterator one-past the last [key, value]-pair in the range
         * @return iterator following the last removed [key, value]-pair (= position of @p first prior to any removal); \n
         *         if `last == end()` prior to removal, then the updated @ref mpicxx::info::end() iterator is returned; \n
         *         if [@p first, @p last) is an empty range, then @p last is returned
         *
         * @pre `*this` **may not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p first and @p last **must** refer to `*this` info object
         * @pre The position denoted by @p first **must** be in the interval [0, `this->size()`].
         * @pre The position denoted by @p last **must** be in the interval [0, `this->size()`].
         * @pre @p first **must** be less or equal than @p last.
         * @post As of the [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         *       are invalidated. \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the first point of erase remain valid,
         *       all other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p first or @p last does not refer to `*this` info object. \n
         *                       If attempting an illegal dereferencing. }
         * @assert_sanity{ If @p first is greater than @p last. }
         *
         * @calls{
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);    // exactly 'last - first' times
         * int MPI_Info_delete(MPI_Info info, const char *key);         // exactly 'last - first' times
         * }
         */
        iterator erase(const_iterator first, const_iterator last) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_info_iterator(first),
                    "Attempt to use an info iterator ('first') referring to another info object!");
            MPICXX_ASSERT_PRECONDITION(this->legal_info_iterator(last),
                    "Attempt to use an info iterator ('last') referring to another info object!");
            MPICXX_ASSERT_PRECONDITION(this->info_iterator_valid(first),
                    "Attempt to dereference a {} iterator ('first')!", first.state());
            MPICXX_ASSERT_PRECONDITION(this->info_iterator_valid(last),
                    "Attempt to dereference a {} iterator ('last')!", last.state());
            MPICXX_ASSERT_SANITY(this->legal_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");

            const difference_type count = last - first;
            char key[MPI_MAX_INFO_KEY];
            std::vector<std::string> keys_to_delete(count);

            // save all keys in the range [first, last)
            for (difference_type i = 0; i < count; ++i) {
                MPI_Info_get_nthkey(info_, first.pos_ + i, key);
                keys_to_delete[i] = key;
            }

            // delete all saved [key, value]-pairs
            for (const auto& str : keys_to_delete) {
                MPI_Info_delete(info_, str.data());
            }

            return iterator(info_, first.pos_);
        }
        /**
         * @brief Removes the [key, value]-pair (if one exists) with the key equivalent to @p key.
         * @details Returns either 1 (key found and removed) or 0 (no such key found and therefore nothing removed).
         * @param[in] key key value of the [key, value]-pair to remove
         * @return number of elements removed (either 0 or 1)
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         *       are invalidated, if an erasure took place (i.e. the returned `size_type` is 1). \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the point of erase remain valid, all
         *       other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p key exceeds its size limit. }
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);    // exactly once
         * int MPI_Info_delete(MPI_Info info, const char *key);                                    // at most once
         * }
         */
        size_type erase(const std::string_view key) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                    "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

            // check whether the key exists
            if (this->key_exists(key)) {
                // key exists -> delete the [key, value]-pair
                MPI_Info_delete(info_, key.data());
                return 1;
            }
            return 0;
        }

        /**
         * @brief Exchanges the contents of the info object with those of @p other.
         * @details Does not invoke any move, copy, or swap operations on individual [key, value]-pairs.
         * @param[inout] other info object to exchange the contents with
         *
         * @post `*this` is in a valid state iff @p other was in a valid state and vice versa.
         * @post All iterators remain valid, but now refer to the other info object.
         */
        void swap(info& other) noexcept {
            using std::swap;
            swap(info_, other.info_);
            swap(is_freeable_, other.is_freeable_);
        }

        /**
         * @brief Removes the [key, value]-pair at @p pos and returns it.
         * @param[in] pos iterator to the [key, value]-pair to remove
         * @return the extracted [key, value]-pair
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p pos **must** refer to `*this` info object.
         * @pre The position denoted by @p pos **must** be in the half-open interval [0, `this->size()`).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         *       are invalidated. \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the point of extraction remain valid,
         *       all other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p pos does not refer to `*this` info object. \n
         *                       If attempting an illegal dereferencing. }
         *
         * @calls{
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                  // exactly once
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);       // exactly once
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);    // exactly once
         * int MPI_Info_delete(MPI_Info info, const char *key);                                       // exactly once
         * }
         */
        [[nodiscard]]
        value_type extract(const_iterator pos) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_info_iterator(pos), "Attempt to use an info iterator referring to another info object!");
            MPICXX_ASSERT_PRECONDITION(this->info_iterator_valid(pos), "Attempt to dereference a {} iterator!", pos.state());

            // get [key, value]-pair pointed to by pos
            const value_type& pair = *pos;
            // remove [key, value]-pair from info object
            MPI_Info_delete(info_, pair.first.data());
            // return extracted [key, value]-pair
            return pair;
        }
        /**
         * @brief Removes the [key, value]-pair (if one exists) with the key equivalent to @p key and returns the removed [key, value]-pair.
         * @details Returns a [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional) holding the removed [key, value]-pair
         *          if the @p key exists, [`std::nullopt`](https://en.cppreference.com/w/cpp/utility/optional/nullopt) otherwise.
         * @param[in] key the @p key to extract
         * @return the extracted [key, value]-pair
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         *       are invalidated, if an extraction took place. \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the point of extraction remain valid,
         *       all other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p key exceeds its size limit. }
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);       // exactly once
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);    // at most once
         * int MPI_Info_delete(MPI_Info info, const char *key);                                       // at most once
         * }
         */
        [[nodiscard]]
        std::optional<value_type> extract(const std::string_view key) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                    "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

            // check whether the key exists
            int valuelen, flag;
            MPI_Info_get_valuelen(info_, key.data(), &valuelen, &flag);
            if (static_cast<bool>(flag)) {
                // key exists -> delete the [key, value]-pair and return an iterator
                // get the value associated with the given key
                std::string value(valuelen, ' ');
                MPI_Info_get(info_, key.data(), valuelen, value.data(), &flag);
                // delete the [key, value]-pair from the info object
                MPI_Info_delete(info_, key.data());
                // return the extracted [key, value]-pair
                return std::make_optional<value_type>(std::make_pair(std::string(key), std::move(value)));
            }
            return std::nullopt;
        }

        /**
         * @brief Attempts to extract each [key, value]-pair in @p source and insert it into `*this`.
         * @details If there is a [key, value]-pair in `*this` with key equivalent to an [key, value]-pair from @p source, than the
         *          [key, value]-pair is not extracted from @p source.
         *
         *          Directly returns if a "self-extraction" is attempted.
         *
         *          The [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) only guarantees that the number of a
         *          given key does not change **as long as** no call to
         *          [*MPI_Info_set*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) or
         *          [*MPI_Info_delete*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) is made.
         *
         *          Therefore (to be compliant with the standard) this function performs two steps:
         *          1. Save all keys of the [key, value]-pairs that will be extracted in a
         *             [`std::vector<std::string>>`](https://en.cppreference.com/w/cpp/container/vector). Add the respective
         *             [key, value]-pairs to `*this`.
         *          2. Delete all [key, value]-pairs in @p source with a key contained in the previously created vector.
         * @param[inout] source the info object to transfer the [key, value]-pairs from
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p source **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         *       and @p source are invalidated, if a transfer of [key, value]-pairs took place. \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the first point of insertion/extraction
         *       remain valid, all other iterators are invalidated.
         *
         * @assert_precondition{ If `*this` or @p source refer to
         *                       [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         * @assert_sanity{ If `*this` and @p source are the same info object. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                         // exactly once
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);       // at least 'source.size()' times, at most '2 * source.size()' times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);    // at most 'source.size()' times
         * int MPI_Info_delete(MPI_Info info, const char *key);                                       // at most 'source.size()' times
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                       // at most 'source.size()' times
         * }
         */
        void merge(info& source) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object ('*this') referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(!source.refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object ('source') referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_SANITY(!this->identical(source), "Attempt to perform a \"self merge\"!");

            // do nothing if a "self merge" is attempted
            if (this == std::addressof(source)) return;

            size_type size = source.size();
            char source_key[MPI_MAX_INFO_KEY];
            std::vector<std::string> keys_to_delete;

            // loop as long as there is at least one [key, value]-pair not visited yet
            for (size_type i = 0; i < size; ++i) {
                // get source_key
                MPI_Info_get_nthkey(source.info_, i, source_key);

                // check if source_key already exists in *this
                if (!this->key_exists(source_key)) {
                    // get the value associated with source_key
                    int valuelen, flag;
                    MPI_Info_get_valuelen(source.info_, source_key, &valuelen, &flag);
                    auto source_value = std::make_unique<char[]>(valuelen + 1);
                    MPI_Info_get(source.info_, source_key, valuelen, source_value.get(), &flag);
                    // remember the source's key
                    keys_to_delete.emplace_back(source_key);
                    // add [key, value]-pair to *this info object
                    MPI_Info_set(info_, source_key, source_value.get());
                }
            }

            // delete all [key, value]-pairs merged into *this info object from source
            for (const auto& str : keys_to_delete) {
                MPI_Info_delete(source.info_, str.data());
            }
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                   lookup                                                   //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name lookup
        ///@{
        /**
         * @brief Returns the number of [key, value]-pairs with key equivalent to @p key.
         * @details Since info objects don't allow duplicated keys the returned value is either 0 (key not found) or 1 (key found).
         *
         *    Therefore @ref contains(const std::string_view) const may be a better choice.
         * @param[in] key @p key value of the [key, value]-pairs to count
         * @return number of [key, value]-pairs with key equivalent to @p key, which is either 0 or 1
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p key exceeds its size limit. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);           // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);    // at most 'this->size()' times
         * }
         */
        [[nodiscard]]
        size_type count(const std::string_view key) const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                    "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

            return static_cast<size_type>(this->contains(key));
        }
        /**
         * @brief Finds a [key, value]-pair with key equivalent to @p key.
         * @details If the key is found, returns an iterator pointing to the corresponding [key, value]-pair,
         *          otherwise the past-the-end iterator is returned (see @ref mpicxx::info::end()).
         * @param[in] key @p key value of the [key, value]-pair to search for
         * @return iterator to a [key, value]-pair with key equivalent to @p key or the past-the-end iterator if no such key is found
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p key exceeds its size limit. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);           // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);    // at most 'this->size()' times
         * }
         */
        [[nodiscard]]
        iterator find(const std::string_view key) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                    "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

            const size_type size = this->size();
            return iterator(info_, this->find_pos(key, size));
        }
        /**
         * @brief Finds a [key, value]-pair with key equivalent to @p key.
         * @details If the key is found, returns a const_iterator pointing to the corresponding [key, value]-pair,
         *          otherwise the past-the-end const_iterator is returned (see @ref mpicxx::info::cend()).
         * @param[in] key @p key value of the [key, value]-pair to search for
         * @return const_iterator to a [key, value]-pair with key equivalent to @p key or the past-the-end iterator if no such key is found
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p key exceeds its size limit. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);           // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);    // at most 'this->size()' times
         * }
         */
        [[nodiscard]]
        const_iterator find(const std::string_view key) const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                    "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

            const size_type size = this->size();
            return const_iterator(info_, this->find_pos(key, size));
        }
        /**
         * @brief Checks if there is a [key, value]-pair with key equivalent to @p key.
         * @param[in] key @p key value of the [key, value]-pair to search for
         * @return `true` if there is such a [key, value]-pair, otherwise `false`
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p key exceeds its size limit. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);           // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);    // at most 'this->size()' times
         * }
         */
        [[nodiscard]]
        bool contains(const std::string_view key) const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                    "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

            const size_type size = this->size();
            return this->find_pos(key, size) != size;
        }
        /**
         * @brief Returns a range containing all [key, value]-pairs with key equivalent to @p key.
         * @details The range is defined by two iterators, the first pointing to the first [key, value]-pair of the wanted range and the
         *          second pointing past the last [key, value]-pair of the range.
         *
         *          Since info objects don't allow duplicated keys
         *          [`std::distance(it_range.first, it_range.second)`](https://en.cppreference.com/w/cpp/iterator/distance)
         *          is either 0 (key not found) or 1 (key found).
         *
         *          Therefore @ref mpicxx::info::find(const std::string_view) may be a better choice.
         * @param[in] key @p key value of the [key, value]-pair to search for
         * @return [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) containing a pair of iterators defining the wanted range;\n
         *         if there is no such element, past-the-end (see @ref mpicxx::info::end()) iterators are returned as both elements of the pair
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p key exceeds its size limit. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);           // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);    // at most 'this->size()' times
         * }
         */
        [[nodiscard]]
        std::pair<iterator, iterator> equal_range(const std::string_view key) {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                    "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

            const size_type size = this->size();
            const size_type pos = this->find_pos(key, size);
            if (pos != size) {
                // found key in the info object
                return std::make_pair(iterator(info_, pos), iterator(info_, pos + 1));
            } else {
                // the key is not in the info object
                return std::make_pair(iterator(info_, size), iterator(info_, size));
            }
        }
        /**
         * @brief Returns a range containing all [key, value]-pairs with key equivalent to @p key.
         * @details The range is defined by two const_iterators, the first pointing to the first [key, value]-pair of the wanted range and
         *          the second pointing past the last [key, value]-pair of the range.
         *
         *          Since info objects don't allow duplicated keys
         *          [`std::distance(const_it_range.first, const_it_range.second)`](https://en.cppreference.com/w/cpp/iterator/distance)
         *          is either 0 (key not found) or 1 (key found).
         *
         *          Therefore @ref mpicxx::info::find(const std::string_view) const may be a better choice.
         * @param[in] key @p key value of the [key, value]-pair to search for
         * @return [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) containing a pair of const_iterators defining the wanted
         *         range; \n
         *         if there is no such element, past-the-end (see @ref mpicxx::info::end()) const_iterators are returned as both elements
         *         of the pair
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than
         *      [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). \n
         *                       If @p key exceeds its size limit. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);           // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);    // at most 'this->size()' times
         * }
         */
        [[nodiscard]]
        std::pair<const_iterator, const_iterator> equal_range(const std::string_view key) const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
            MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                    "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

            const size_type size = this->size();
            const size_type pos = this->find_pos(key, size);
            if (pos != size) {
                // found key in the info object
                return std::make_pair(const_iterator(info_, pos), const_iterator(info_, pos + 1));
            } else {
                // the key is not in the info object
                return std::make_pair(const_iterator(info_, size), const_iterator(info_, size));
            }
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            non-member functions                                            //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name non-member functions
        ///@{
        /**
         * @brief Compares the contents of the two info objects for equality.
         * @details Two info objects compare equal iff they have the same size and their contents compare equal. \n
         *          Automatically generates `mpicxx::info::operator!=(const info&, const info&)`.
         * @param[in] lhs the @p lhs info object to compare
         * @param[in] rhs the @p rhs info object to compare
         * @return `true` if the contents of the info objects are equal, `false` otherwise
         * @nodiscard
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                         // at most twice
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                  // at most 'lhs.size()' times
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);       // at most '2 * lhs.size()' times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);    // at most '2 * lhs.size()' times
         * }
         */
        [[nodiscard]]
        friend bool operator==(const info& lhs, const info& rhs) {
            // if both info object refer to MPI_INFO_NULL they compare equal
            if (lhs.info_ == MPI_INFO_NULL && rhs.info_ == MPI_INFO_NULL) return true;
            // if only one info object refers to MPI_INFO_NULL they don't compare equal
            if (lhs.info_ == MPI_INFO_NULL || rhs.info_ == MPI_INFO_NULL) return false;

            // not the same number of [key, value]-pairs therefore can't compare equal
            const size_type size = lhs.size();
            if (size != rhs.size()) return false;

            // check all [key, value]-pairs for equality
            char key[MPI_MAX_INFO_KEY];
            for (size_type i = 0; i < size; ++i) {
                // retrieve key
                MPI_Info_get_nthkey(lhs.info_, i, key);

                // check if rhs contains the current key
                int valuelen, flag;
                MPI_Info_get_valuelen(rhs.info_, key, &valuelen, &flag);
                if (!static_cast<bool>(flag)) {
                    // rhs does not contain the currently inspected lhs key -> info objects can't compare equal
                    return false;
                }

                // both info objects contain the same key -> check for the respective values
                int lhs_valuelen;
                MPI_Info_get_valuelen(lhs.info_, key, &lhs_valuelen, &flag);
                if (valuelen != lhs_valuelen) {
                    // both values have different lengths -> different values -> info objects can't compare equal
                    return false;
                }

                // allocate a buffer for each value with the correct length
                auto lhs_value = std::make_unique<char[]>(valuelen + 1);
                auto rhs_value = std::make_unique<char[]>(valuelen + 1);
                // retrieve values
                MPI_Info_get(lhs.info_, key, valuelen, lhs_value.get(), &flag);
                MPI_Info_get(rhs.info_, key, valuelen, rhs_value.get(), &flag);
                // check if the values are equal
                const bool are_values_equal = std::strcmp(lhs_value.get(), rhs_value.get()) == 0;
                if (!are_values_equal) {
                    // values compare inequal -> info objects can't compare equal
                    return false;
                }
            }

            // all elements are equal
            return true;
        }
        /**
         * @brief Specializes the [`std::swap`](https://en.cppreference.com/w/cpp/algorithm/swap) algorithm for info objects.
         *        Swaps the contents of @p lhs and @p rhs.
         * @details Calls `lhs.swap(rhs)`.
         *
         *          Does not invoke any move, copy or swap operations on individual elements.
         * @param[inout] lhs the info object whose contents to swap
         * @param[inout] rhs the info object whose contents to swap
         *
         * @post All iterators remain valid, but now refer to the other info object.
         */
        friend void swap(info& lhs, info& rhs) noexcept { lhs.swap(rhs); }
        /**
         * @brief Erases all [key, value]-pairs that satisfy the predicate @p pred from the info object.
         * @details The [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) only guarantees that the number of a
         *          given key does not change **as long as** no call to
         *          [*MPI_Info_set*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) or
         *          [*MPI_Info_delete*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) is made.
         *
         *          Therefore (to be compliant with the standard) this function performs two steps:
         *          1. Save all keys, for which the corresponding [key, value]-pair satisfies the predicate @p pred, in a
         *             [`std::vector<std::string>>`](https://en.cppreference.com/w/cpp/container/vector).
         *          2. Delete all [key, value]-pairs in the info object @p c with a key contained in the previously created vector.
         * @tparam Pred a valid predicate which accepts a `value_type` and returns a `bool`
         * @param[inout] c info object from which to erase
         * @param[in] pred predicate that returns `true` if the element should be erased
         *
         * @pre @p c **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to @p c
         *       are invalidated. \n
         *       Specific MPI implementations **may** differ in this regard, i.e. iterators before the first point of erase remain valid,
         *       all other iterators are invalidated.
         *
         * @assert_precondition{ If `c` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls{
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                  // exactly 'c.size()' times
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);       // exactly 'c.size()' times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);    // exactly 'c.size()' times
         * int MPI_Info_delete(MPI_Info info, const char *key);                                       // at most 'c.size()' times
         * }
         */
        template <typename Pred>
        friend void erase_if(info& c, Pred pred) requires std::is_invocable_r_v<bool, Pred, value_type> {
            MPICXX_ASSERT_PRECONDITION(!c.refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object ('c') referring to 'MPI_INFO_NULL'!");

            size_type size = c.size();
            char key[MPI_MAX_INFO_KEY];

            std::vector<std::string> keys_to_delete;

            // loop through all [key, value]-pairs
            for (info::size_type i = 0; i < size; ++i) {
                // get key
                MPI_Info_get_nthkey(c.info_, i, key);
                // get value associated with key
                int valuelen, flag;
                MPI_Info_get_valuelen(c.info_, key, &valuelen, &flag);
                std::string value(valuelen, ' ');
                MPI_Info_get(c.info_, key, valuelen, value.data(), &flag);
                // create [key, value]-pair as a std::pair
                const value_type& pair = std::make_pair(std::string(key), std::move(value));

                // check whether the predicate holds
                if (std::invoke(pred, pair)) {
                    // the predicate evaluates to true -> remember key for erasure
                    keys_to_delete.emplace_back(std::move(pair.first));
                }
            }

            // delete all [key, value]-pairs for which the predicate returns true
            for (const auto& str : keys_to_delete) {
                MPI_Info_delete(c.info_, str.data());
            }
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            additional functions                                            //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name additional member functions
        /// (member functions which are not related to the [`std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map)
        /// or [`std::map`](https://en.cppreference.com/w/cpp/container/map) interface)
        ///@{
        /**
         * @brief Returns a [`std::vector`](https://en.cppreference.com/w/cpp/container/vector) containing all keys of the info object.
         * @return all keys of the info object
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);           // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);    // exactly 'this->size()' times
         * }
         */
        [[nodiscard]]
        std::vector<key_type> keys() const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

            // create vector which will hold all keys
            const size_type size = this->size();
            std::vector<key_type> keys(size);
            char key[MPI_MAX_INFO_KEY];

            for (size_type i = 0; i < size; ++i) {
                // get key and add it to the vector
                MPI_Info_get_nthkey(info_, i, key);
                keys[i] = key;
            }

            return keys;
        }
        /**
         * @brief Returns a [`std::vector`](https://en.cppreference.com/w/cpp/container/vector) containing all values of the info object.
         * @return all values of the info object
         * @nodiscard
         *
         * @pre `*this` **must not** refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         *
         * @assert_precondition{ If `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm). }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                         // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                  // exactly 'this->size()' times
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);       // exactly 'this->size()' times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);    // exactly 'this->size()' times
         * }
         */
        [[nodiscard]]
        std::vector<mapped_type> values() const {
            MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                    "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

            // create vector which will hold all values
            const size_type size = this->size();
            std::vector<mapped_type> values(size);
            char key[MPI_MAX_INFO_KEY];

            for (size_type i = 0; i < size; ++i) {
                // get key
                MPI_Info_get_nthkey(info_, i, key);
                // get value associated with key and add it to the vector
                int valuelen, flag;
                MPI_Info_get_valuelen(info_, key, &valuelen, &flag);
                std::string value(valuelen, ' ');
                MPI_Info_get(info_, key, valuelen, value.data(), &flag);
                values[i] = std::move(value);
            }

            return values;
        }
        /**
         * @brief Returns the maximum possible key size of any [key, value]-pair.
         * @return the maximum key size (= [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm))
         * @nodiscard
         */
        [[nodiscard]]
        static constexpr size_type max_key_size() { return static_cast<size_type>(MPI_MAX_INFO_KEY); }
        /**
         * @brief Returns the maximum possible value size of any [key, value]-pair.
         * @return the maximum value size (= [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm))
         * @nodiscard
         */
        [[nodiscard]]
        static constexpr size_type max_value_size() { return static_cast<size_type>(MPI_MAX_INFO_VAL); }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                   getter                                                   //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Get the underlying [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object.
         * @return the [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object wrapped in this
         *         @ref mpicxx::info object
         * @nodiscard
         */
        [[nodiscard]]
        const MPI_Info& get() const noexcept { return info_; }
        /**
         * @brief Get the underlying [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object.
         * @return the [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object wrapped in this
         *         @ref mpicxx::info object
         * @nodiscard
         */
        [[nodiscard]]
        MPI_Info& get() noexcept { return info_; }
        /**
         * @brief Returns whether the underlying [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object gets
         *        automatically freed upon destruction, i.e. the destructor calls
         *        [*MPI_Info_free*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @return `true` if [*MPI_Info_free*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) gets called upon
         *         destruction, `false` otherwise
         * @nodiscard
         */
        [[nodiscard]]
        bool freeable() const noexcept { return is_freeable_; }
        ///@}


    private:
        /*
         * @brief Finds the position of the given @p key in the info object.
         * @param[in] key the @p key to find
         * @param[in] size the current size of the info object
         * @return the position of the @p key or @p size if the @p key does not exist in this info object
         *
         * @attention No assertions are performed because this function is only callable from within this class and
         *            every caller already checks all preconditions.
         *
         * @calls{ int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);    // at most 'this->size()' times }
         */
        size_type find_pos(const std::string_view key, const size_type size) const {
            char info_key[MPI_MAX_INFO_KEY];
            // loop until a matching key is found
            for (size_type i = 0; i < size; ++i) {
                MPI_Info_get_nthkey(info_, i, info_key);
                // found equal key
                if (key.compare(info_key) == 0) {
                    return i;
                }
            }
            // no matching key found
            return size;
        }

        /*
         * @brief Tests whether the given @p key already exists in the info object.
         * @param[in] key the @p key to check for
         * @return `true` if the @p key already exists, `false` otherwise
         *
         * @attention No assertions are performed because this function is only callable from within this class and
         *            every caller already checks all preconditions.
         *
         * @calls{ int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);    // exactly once }
         */
        bool key_exists(const std::string_view key) const {
            int valuelen, flag;
            MPI_Info_get_valuelen(info_, key.data(), &valuelen, &flag);
            return static_cast<bool>(flag);
        }
#if MPICXX_ASSERTION_LEVEL > 0
        /*
         * @brief Check whether `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
         * @return `true` if `*this` refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm),
         *         otherwise `false`
         */
        bool refers_to_mpi_info_null() const {
            return info_ == MPI_INFO_NULL;
        }
        /*
         * @brief Check whether `*this` and @p other are identical, i.e. they refer to the **same** underlying
         *        [*MPI_INFO*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object.
         * @param[in] other the other info object
         * @return `true` if `*this` and @p other are the same object, otherwise `false`
         */
        bool identical(const info& other) const {
            return this == std::addressof(other);
        }
        /*
         * @brief Check whether @p val has a legal size.
         * @details @p val has a legal size if it is greater than zero and less then @p max_size.
         * @param[in] val the string to check
         * @param[in] max_size the maximum legal size
         * @return `true` if the size is legal, otherwise `false`
         */
        bool legal_string_size(const std::string_view val, const int max_size) const {
            return 0 < val.size() && val.size() < static_cast<std::size_t>(max_size);
        }
        /*
         * @brief Check whether @p first and @p last denote a valid range, i.e. @p first is less or equal than @p last.
         * @details Checks whether the distance bewteen @p first and @p last is not negative.
         * @param[in] first iterator to the first element
         * @param[in] last past-the-end iterator
         * @return `true` if @p first and @p last denote a valid iterator range, otherwise `false`
         */
        template <std::input_iterator InputIt>
        bool legal_iterator_range(InputIt first, InputIt last) {
            return std::distance(first, last) >= 0;
        }
        /*
         * @brief Checks whether @p it is a legal info iterator, i.e. @it is not singular and points to `*this` info object.
         * @param[in] it the iterator to check
         * @return `true` if @p it is legal, otherwise `false`
         */
        bool legal_info_iterator(const_iterator it) const {
            return !it.singular() && it.info_ == std::addressof(info_);
        }
        /*
         * @brief Checks wether @p it can be savely used as an iterator to denote an iterator range, i.e. @p it is a legal info iterator and
         *        points to a [key, value]-pair in the range `[0, this->size()]`.
         * @param[in] it the iterator to check
         * @return `true` if @p it is legal and points to a legal position, otherwise `false`
         */
        bool info_iterator_valid(const_iterator it) const {
            return this->legal_info_iterator(it) && 0 <= it.pos_ && it.pos_ <= static_cast<const_iterator::difference_type>(this->size());
        }
#endif

        MPI_Info info_;
        bool is_freeable_;
    };

    // initialize static environment object
    inline const info info::env = info(MPI_INFO_ENV, false);

    // initialize static null object
    inline const info info::null = info(MPI_INFO_NULL, false);

}

#endif // MPICXX_INFO_HPP