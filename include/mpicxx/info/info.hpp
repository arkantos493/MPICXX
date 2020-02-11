/**
 * @file include/mpicxx/info/info.hpp
 * @author Marcel Breyer
 * @date 2020-02-11
 *
 * @brief Implements a wrapper class around the *MPI_Info* object.
 *
 * The @ref mpicxx::info class interface is inspired by the
 * [`std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map) and
 * [`std::map`](https://en.cppreference.com/w/cpp/container/map) interface.
 */

#ifndef MPICXX_INFO_HPP
#define MPICXX_INFO_HPP

#include <cstddef>
#include <cstring>
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

#include <mpi.h>

#include <mpicxx/detail/assert.hpp>
#include <mpicxx/detail/concepts.hpp>


// TODO 2020-01-22 19:26 marcel: remaining tests
// TODO 2020-01-26 19:35 marcel: usage example
namespace mpicxx {
    /**
     * This class is a wrapper to the *MPI_Info* object providing a interface inspired by
     * [`std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map).
     *
     * TODO: usage example
     */
    class info {

        // ---------------------------------------------------------------------------------------------------------- //
        //                                                 proxy class                                                //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief A proxy class for @ref info::at(detail::string auto&&), @ref info::at(const std::string_view) const and
         * @ref info::operator[](detail::string auto&&) to distinguish between read and write accesses.
         * @details Calls @ref operator std::string() const on a write access and @ref operator=(const std::string_view) on a read access.
         *
         * Can be printed directly through an @ref operator<<(std::ostream&, const proxy&) overload.
         */
        class proxy {
        public:
            /**
             * @brief Construct a new proxy object.
             * @param[in] info pointer to the parent info object
             * @param[in] key the provided @p key (must meet the requirements of the detail::string concept)
             */
            proxy(MPI_Info info, detail::string auto&& key) : info_(info), key_(std::forward<decltype(key)>(key)) { }

            /**
             * @brief On write access, add the provided @p value and saved key to the info object.
             * @details Creates a new [key, value]-pair if the key doesn't already exist, otherwise overwrites the existing @p value.
             * @param[in] value the @p value associated with key
             *
             * @pre @p value **must** include the null-terminator.
             * @pre The @p value's length **must** be greater than 0 and less than *MPI_MAX_INFO_VAL*.
             *
             * @assert{ If @p value exceeds its size limit. }
             *
             * @calls{
             * int MPI_Info_set(MPI_Info info, const char *key, const char *value);         // exactly once
             * }
             */
            void operator=(const std::string_view value) {
                MPICXX_ASSERT(0 < value.size() && value.size() < MPI_MAX_INFO_VAL,
                              "Illegal info value: 0 < %u < %i (MPI_MAX_INFO_VAL)",
                              value.size(), MPI_MAX_INFO_VAL);

                MPI_Info_set(info_, key_.data(), value.data());
            }

            /**
             * @brief On read access return the value associated with the saved key.
             * @details If the key doesn't exist yet, it will be inserted with a string consisting only of one whitespace as value,
             * also returning a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string)`(" ")`.
             * @return the value associated with key
             *
             * @attention This function returns the associated value *by-value*, i.e. changing the returned
             * [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) **won't** alter the info object's internal value!
             * @attention Because inserting an empty string `""` is not allowed, a `" "` string is inserted instead, if the key does not
             * already exist.
             *
             * @calls{
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
             * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                         // at most once
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // at most once
             * }
             */
            operator std::string() const {
                // get the length of the value
                int valuelen, flag;
                MPI_Info_get_valuelen(info_, key_.data(), &valuelen, &flag);

                if (!static_cast<bool>(flag)) {
                    // the key doesn't exist yet
                    // -> add a new [key, value]-pair and return a std::string consisting of only one whitespace
                    std::string value(" ");
                    MPI_Info_set(info_, key_.data(), value.data());
                    return value;
                }

                // key exists -> get the associated value
                std::string value(valuelen, ' ');
                MPI_Info_get(info_, key_.data(), valuelen, value.data(), &flag);
                return value;
            }

            /**
             * @brief Convenience overload to be able to directly print a proxy object.
             * @details Calls @ref operator std::string() const to get the value that should be printed, i.e. if key doesn't exist yet, a
             * new [key, value]-pair will be inserted into the info object.
             * @param[inout] out the output stream to write on
             * @param[in] rhs the proxy object
             * @return the output stream
             *
             * @calls{
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
             * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                         // at most once
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // at most once
             * }
             */
            friend std::ostream& operator<<(std::ostream& out, const proxy& rhs) {
                out << static_cast<std::string>(rhs.operator std::string());
                return out;
            }

        private:
            MPI_Info info_;
            const std::string key_;
        };


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  iterators                                                 //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Provides iterator and const_iterator for an info object.
         * @details The standard reverse_iterator and const_reverse_iterator are provided
         * in terms of [`std::reverse_iterator<iterator>`](https://en.cppreference.com/w/cpp/iterator/reverse_iterator) and
         * [`std::reverse_iterator<const_iterator>`](https://en.cppreference.com/w/cpp/iterator/reverse_iterator) respectively.
         * @tparam is_const if `true` a const_iterator is instantiated, otherwise a non-const iterator
         */
        template <bool is_const>
        class iterator_impl {
            // needed to be able to construct a const_iterator from an iterator
            template <bool>
            friend class iterator_impl;
            // info class can now directly access the pos member
            friend class info;
        public:
            // ---------------------------------------------------------------------------------------------------------- //
            //                                         iterator_traits definitions                                        //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) difference type to identify the
             * distance between two iterators.
             */
            using difference_type = std::ptrdiff_t;
            /**
             * @brief [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) value type that can be obtained
             * by dereferencing the iterator.
             * @details In case of a non-const iterator, the value will be returned by a @ref info::proxy object to allow changing
             * its value.
             *
             * In case of a const_iterator, the value will directly be returned as `const std::string` because changing the
             * value is impossible by definition.
             */
            using value_type = std::conditional_t<is_const,
                                                  std::pair<const std::string, const std::string>,
                                                  std::pair<const std::string, proxy>>;
            /**
             * @brief [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) pointer type to the
             * [key, value]-pair iterated over.
             * @details Because it is not possible to simply return value_type* (dangling pointer to a local object),
             * it is necessary to wrap value_type in a `std::unique_ptr`.
             */
            using pointer = std::unique_ptr<value_type>;
            /**
             * @brief [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) reference type
             * (**not** meaningful because operator*() and operator->() has to return **by-value** (using a proxy for write access)).
             */
            using reference = value_type;
            /// [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) iterator category.
            using iterator_category = std::random_access_iterator_tag;
            /// [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) iterator concept (for C++20 concepts)
            using iterator_concept = std::random_access_iterator_tag;

            /// pointer type to the referred to info object (pointer to const if `is_const` is `true`)
            using MPI_Info_ptr = std::conditional_t<is_const, const MPI_Info*, MPI_Info*>;
            /// reference type to the referred to info object (reference to const if `is_const` is `true`)
            using MPI_Info_ref = std::conditional_t<is_const, const MPI_Info&, MPI_Info&>;
            // ---------------------------------------------------------------------------------------------------------- //
            //                                                constructors                                                //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Default construct a new iterator.
             *
             * @post The iterator is **singular**.
             */
            iterator_impl() : info_(nullptr), pos_(0) { }
            /**
             * @brief Construct a new iterator.
             * @param[inout] info pointer to the iterated over *MPI_Info* object
             * @param[in] pos the iterator's start position
             *
             * @post The iterator is **not singular**.
             */
            iterator_impl(MPI_Info_ref info, const difference_type pos) : info_(&info), pos_(pos) { }
            /**
             * @brief Special copy constructor: defined to be able to convert a non-const iterator to a const_iterator.
             * @tparam is_const_iterator
             * @param[in] other the copied iterator
             *
             * @post The iterator is not singular if and only if @p other is not singular.
             */
            template <bool is_const_iterator> requires is_const
            iterator_impl(const iterator_impl<is_const_iterator>& other) : info_(other.info_), pos_(other.pos_) { }


            // ---------------------------------------------------------------------------------------------------------- //
            //                                            assignment operator                                             //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Special copy assignment operator: defined to be able to assign a non-const iterator to a const_iterator.
             * @tparam is_const_iterator
             * @param[in] rhs the copied iterator
             *
             * @post The iterator is not singular if and only if @p rhs is not singular.
             */
            template <bool is_const_iterator>
            iterator_impl& operator=(const iterator_impl<is_const_iterator>& rhs) requires is_const {
                info_ = rhs.info_;
                pos_ = rhs.pos_;
                return *this;
            }

            // TODO 2020-02-09 20:54 marcel: TESTS and DOCUMENTATION !!!!
            // ---------------------------------------------------------------------------------------------------------- //
            //                                            relational operators                                            //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Perform the respective comparison operation on the iterator and the given @p rhs one.
             * @details The iterators `*this` and @p rhs **may not** necessarily have the same constness.
             * @tparam rhs_const
             * @param[in] rhs the other iterator
             * @return boolean comparison result
             *
             * @assert_sanity{
             * If `*this` or @p rhs is a singular iterator. \n
             * If `*this` and @p rhs don't refer to the same info object.
             * }
             */
            template <bool rhs_const>
            bool operator==(const iterator_impl<rhs_const>& rhs) const {
                MPICXX_ASSERT_SANITY(!this->singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
                        this->state(), rhs.state());
                MPICXX_ASSERT_SANITY(this->comparable(rhs), "Attempt to compare iterators from different sequences!");

                return info_ == rhs.info_ && pos_ == rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator!=(const iterator_impl<is_rhs_const>& rhs) const {
                MPICXX_ASSERT_SANITY(!this->singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
                        this->state(), rhs.state());
                MPICXX_ASSERT_SANITY(this->comparable(rhs), "Attempt to compare iterators from different sequences!");

                return info_ != rhs.info_ || pos_ != rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator<(const iterator_impl<is_rhs_const>& rhs) const {
                MPICXX_ASSERT_SANITY(!this->singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
                        this->state(), rhs.state());
                MPICXX_ASSERT_SANITY(this->comparable(rhs), "Attempt to compare iterators from different sequences!");

                return info_ == rhs.info_ && pos_ < rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator>(const iterator_impl<is_rhs_const>& rhs) const {
                MPICXX_ASSERT_SANITY(!this->singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
                        this->state(), rhs.state());
                MPICXX_ASSERT_SANITY(this->comparable(rhs), "Attempt to compare iterators from different sequences!");

                return info_ == rhs.info_ && pos_ > rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator<=(const iterator_impl<is_rhs_const>& rhs) const {
                MPICXX_ASSERT_SANITY(!this->singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
                        this->state(), rhs.state());
                MPICXX_ASSERT_SANITY(this->comparable(rhs), "Attempt to compare iterators from different sequences!");

                return info_ == rhs.info_ && pos_ <= rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator>=(const iterator_impl<is_rhs_const>& rhs) const {
                MPICXX_ASSERT_SANITY(!this->singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
                        this->state(), rhs.state());
                MPICXX_ASSERT_SANITY(this->comparable(rhs), "Attempt to compare iterators from different sequences!");

                return info_ == rhs.info_ && pos_ >= rhs.pos_;
            }


            // ---------------------------------------------------------------------------------------------------------- //
            //                                            modifying operations                                            //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Move the iterator one position forward.
             * @return modified iterator pointing to the new position
             */
            iterator_impl& operator++() {
                MPICXX_ASSERT_SANITY(this->incrementable(), "Attempt to increment a {} iterator!", this->state());

                ++pos_;
                return *this;
            }
            /**
             * @brief Move the iterator one position forward and return the old iterator.
             * @return iterator pointing to the old position
             */
            iterator_impl operator++(int) {
                MPICXX_ASSERT_SANITY(this->incrementable(), "Attempt to increment a {} iterator!", this->state());

                iterator_impl tmp{*this};
                operator++();
                return tmp;
            }
            /**
             * @brief Move this iterator @p inc steps forward.
             * @param[in] inc number of steps
             * @return modified iterator pointing to the new position
             */
            iterator_impl& operator+=(const difference_type inc) {
                MPICXX_ASSERT_SANITY(this->advanceable(inc),
                        "Attempt to advance a {} iterator {} steps, which falls outside its valid range!", this->state(), inc);

                pos_ += inc;
                return *this;
            }
            /**
             * @brief Move the iterator @p inc steps forward.
             * @param[in] it the iterator to increment
             * @param[in] inc number of steps
             * @return new iterator pointing to the new position
             */
            friend iterator_impl operator+(iterator_impl it, const difference_type inc) {
                MPICXX_ASSERT_SANITY(it.advanceable(inc),
                        "Attempt to advance a {} iterator {} steps, which falls outside its valid range!", it.state(), inc);

                it.pos_ += inc;
                return it;
            }
            /**
             * @copydoc operator+(iterator_impl, const difference_type)
             */
            friend iterator_impl operator+(const difference_type inc, iterator_impl it) {
                MPICXX_ASSERT_SANITY(it.advanceable(inc),
                        "Attempt to advance a {} iterator {} steps, which falls outside its valid range!", it.state(), inc);

                return it + inc;
            }
            /**
             * @brief Move the iterator one position backward.
             * @return modified iterator pointing to the new position
             */
            iterator_impl& operator--() {
                MPICXX_ASSERT_SANITY(this->decrementable(), "Attempt to decrement a {} iterator!", this->state());

                --pos_;
                return *this;
            }
            /**
             * @brief Move the iterator one position backward and return the old iterator.
             * @return iterator pointing to the old position
             */
            iterator_impl operator--(int) {
                MPICXX_ASSERT_SANITY(this->decrementable(), "Attempt to decrement a {} iterator!", this->state());

                iterator_impl tmp{*this};
                operator--();
                return tmp;
            }
            /**
             * @brief Move the iterator @p inc steps backward.
             * @param[in] inc number of steps
             * @return modified iterator pointing to the new position
             */
            iterator_impl& operator-=(const difference_type inc) {
                MPICXX_ASSERT_SANITY(this->advanceable(-inc),
                        "Attempt to retreat a {} iterator {} steps, which falls outside its valid range!", this->state(), inc);

                pos_ -= inc;
                return *this;
            }
            /**
             * @brief Move the iterator @p inc steps backward.
             * @param[in] it the iterator to decrement
             * @param[in] inc number of steps
             * @return new iterator pointing to the new position
             */
            friend iterator_impl operator-(iterator_impl it, const difference_type inc) {
                MPICXX_ASSERT_SANITY(it.advanceable(-inc),
                        "Attempt to retreat a {} iterator {} steps, which falls outside its valid range!", it.state(), inc);

                it.pos_ -= inc;
                return it;
            }


            // ---------------------------------------------------------------------------------------------------------- //
            //                                            distance calculation                                            //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Calculate the distance between the iterator and the given @p rhs one.
             * @details The iterators `*this` and @p rhs **may not** necessarily have the same constness.
             *
             * It holds: `it2 - it1 ==` [`std::distance`](https://en.cppreference.com/w/cpp/iterator/distance)`(it1, it2)`
             * @tparam is_rhs_const
             * @param[in] rhs the end iterator
             * @return number of [key, value]-pairs between the iterators `*this` and  @p rhs
             *
             * @pre The iterators `*this` and @p rhs iterator **must** point to the same info object.
             *
             * @assert{ If the iterators `*this` and `rhs` don't point to the same info object. }
             */
            template <bool is_rhs_const>
            difference_type operator-(const iterator_impl<is_rhs_const>& rhs) {
                MPICXX_ASSERT_SANITY(!this->singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
                        this->state(), rhs.state());
                MPICXX_ASSERT_SANITY(this->comparable(rhs),
                        "Attempt to compute the different between two iterators from different sequences!");

                return pos_ - rhs.pos_;
            }


            // ---------------------------------------------------------------------------------------------------------- //
            //                                          dereferencing operations                                          //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Get the [key, value]-pair at the current iterator position + @p n.
             * @details If the current iterator is a const_iterator, the returned type is a
             * [`std::pair<const std::string, const std::string>`](https://en.cppreference.com/w/cpp/utility/pair).
             *
             * If the current iterator is a non-const iterator, the returned type is a
             * [`std::pair<const std::string, proxy>`](https://en.cppreference.com/w/cpp/utility/pair), i.e. the [key, value]-pair's value
             * can be changed through the proxy object.
             * @param[in] n the requested offset of the iterator
             * @return the [key, value]-pair
             *
             * @pre The referred to info object **must not** be in the moved-from state.
             * @pre The position denoted by the current iterator position + @p n **must** be in the half-open
             * interval [0, nkeys), where `nkeys` ist the size of the referred to info object.
             *
             * @assert{
             * If the referred to info object is in the moved-from state. \n
             * If attempting an illegal dereferencing.
             * }
             *
             * @calls_ref{
             * @code{.cpp}
             * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                    // exactly once
             * @endcode
             * `const_iterator` (alias for an `iterator_impl<true>`): \n
             * @code{.cpp}
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // exactly once
             * @endcode
             * `iterator` (alias for an `iterator_impl<false>`): \n
             * For *MPI* functions called while using a proxy see the @ref proxy documentation.
             * }
             */
            reference operator[](const difference_type n) const {
                MPICXX_ASSERT_PRECONDITION(!this->info_moved_from(),
                        "Attempt to access a [key, value]-pair of an info object in the moved-from state!");
                MPICXX_ASSERT_PRECONDITION(this->advanceable(n) && this->advanceable(n + 1),
                        "Attempt to subscript a {} iterator {} step from its current position, which falls outside its dereferenceable range.",
                        this->state());

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
             * [`std::pair<const std::string, const std::string>`](https://en.cppreference.com/w/cpp/utility/pair).
             *
             * If the current iterator is a non-const iterator, the returned type is a
             * [`std::pair<const std::string, proxy>`](https://en.cppreference.com/w/cpp/utility/pair), i.e. the [key, value]-pair's value
             * can be changed through the proxy object.
             * @return the [key, value]-pair
             *
             * @pre The referred to info object **must not** be in the moved-from state.
             * @pre The position denoted by the current iterator position **must** be in the half-open
             * interval [0, nkeys), where `nkeys` ist the size of the referred to info object.
             *
             * @assert{
             * If the referred to info object is in the moved-from state. \n
             * If attempting an illegal dereferencing.
             * }
             *
             * @calls_ref{
             * @code{.cpp}
             * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                    // exactly once
             * @endcode
             * `const_iterator` (alias for an `iterator_impl<true>`): \n
             * @code{.cpp}
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // exactly once
             * @endcode
             * `iterator` (alias for an `iterator_impl<false>`): \n
             * For *MPI* functions called while using a proxy see the @ref proxy documentation.
             * }
             */
            reference operator*() const {
                MPICXX_ASSERT_PRECONDITION(!this->info_moved_from(),
                        "Attempt to access a [key, value]-pair of an info object in the moved-from state!");
                MPICXX_ASSERT_PRECONDITION(this->dereferenceable(), "Attempt to dereference a {} iterator!", this->state());

                return this->operator[](0);
            }
            /**
             * @copydoc operator*()
             */
            pointer operator->() const {
                MPICXX_ASSERT_PRECONDITION(!this->info_moved_from(),
                                           "Attempt to access a [key, value]-pair of an info object in the moved-from state!");
                MPICXX_ASSERT_PRECONDITION(this->dereferenceable(), "Attempt to dereference a {} iterator!", this->state());

                return std::make_unique<value_type>(this->operator[](0));
            }


        private:
            difference_type info_size() const {
                int nkeys = 0;
                MPI_Info_get_nkeys(*info_, &nkeys);
                return static_cast<difference_type>(nkeys);
            }

            bool singular() const {
                return info_ == nullptr;
            }
            template <bool rhs_const>
            bool comparable(const iterator_impl<rhs_const>& rhs) const {
                return !this->singular() && !rhs.singular() && info_ == rhs.info_;
            }
            bool past_the_end() const {
                return pos_ >= this->info_size();
            }
            bool start_of_sequence() const {
                return pos_ == 0;
            }
            bool incrementable() const {
                return !this->singular() && !this->past_the_end();
            }
            bool decrementable() const {
                return !this->singular() && !this->start_of_sequence();
            }
            bool advanceable(const difference_type n) const {
                if (this->singular()) {
                    return false;
                }
                if (n > 0) {
                    return pos_ + n <= this->info_size();
                } else {
                    return pos_ + n >= 0;
                }
            }
            bool dereferenceable() const {
                return !this->singular() && !this->past_the_end() && pos_ >= 0;
            }
            bool info_moved_from() const {
                return info_ == MPI_INFO_NULL;
            }

            std::string state() const {
                if (this->singular()) {
                    return std::string("singular");
                } else if (this->past_the_end()) {
                    return std::string("past-the-end");
                } else if (this->start_of_sequence()) {
                    return std::string("dereferenceable (start-of-sequence)");
                } else {
                    return std::string("dereferenceable");
                }
            }

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
         * @brief Static member that holds all execution environment information defined in *MPI_INFO_ENV*.
         * @details As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) predefined keys of *MPI_INFO_ENV*
         * are:
         *
         *  key         | description
         * :----------- | :-------------------------------------------------------------------------------------------------------------- |
         * command      | name of program executed                                                                                        |
         * argv         | space separated arguments to command                                                                            |
         * maxprocs     | maximum number of MPI processes to start (e.g. "1024")                                                          |
         * soft         | allowed values for number of processors                                                                         |
         * host         | hostname                                                                                                        |
         * arch         | architecture name                                                                                               |
         * wdir         | working directory of the MPI process                                                                            |
         * file         | value is the name of a file in which additional information is specified                                        |
         * thread_level | requested level of thread support, if requested before the program started execution (e.g. "MPI_THREAD_SINGLE”) |
         *
         * Note: The contents of *MPI_INFO_ENV* are implementation defined, i.e. not all of the predefined keys have to be defined.
         *
         * @attention **No** *MPI_Info_free* gets called upon destruction (doing so would result in a MPI runtime failure).
         */
        static const info env;


        // ---------------------------------------------------------------------------------------------------------- //
        //                                        constructors and destructor                                         //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Constructs an empty info object.
         *
         * @post The newly constructed info object is in a valid state.
         *
         * @calls{
         * int MPI_Info_create(MPI_Info *info);     // exactly once
         * }
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
         * @pre @p other **must not** be in the moved-from state.
         * @post The newly constructed info object is in a valid state.
         * @attention Every copied info object is marked **freeable** independent of the **freeable** state of the copied-from info object.
         *
         * @assert{ If @p other is in the moved-from state. }
         *
         * @calls{
         * int MPI_Info_dup(MPI_info info, MPI_info *newinfo);      // exactly once
         * }
         */
        info(const info& other) : is_freeable_(true) {
            MPICXX_ASSERT(other.info_ != MPI_INFO_NULL, "'other' is in the moved-from state!");

            MPI_Info_dup(other.info_, &info_);
        }
        /**
         * @brief Move constructor. Constructs the info object with the contents of @p other using move semantics.
         * @details Retains @p other's [key, value]-pair ordering.
         * @param[in] other another info object to be used as source to initialize the [key, value]-pairs of this info object with
         *
         * @post The newly constructed info object is in a valid state iff @p other was in a valid state.
         * @post @p other is now in the moved-from state.
         * @post All iterators referring to @p other remain valid, but now refer to `*this`.
         */
        info(info&& other) noexcept : info_(std::move(other.info_)), is_freeable_(std::move(other.is_freeable_)) {
            // set other to the moved-from state
            other.info_ = MPI_INFO_NULL;
            other.is_freeable_ = false;
        }
        /**
         * @brief Constructs the info object with the contents of the range [@p first, @p last).
         * @details If multiple [key, value]-pairs in the range share the same key, the **last** occurrence determines the final value.
         * @tparam InputIt must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator).
         * @param[in] first iterator to the first [key, value]-pair in the range
         * @param[in] last iterator one-past the last [key, value]-pair in the range
         *
         * Example:
         * @code{.cpp}
         * std::vector<std::pair<const std::string, std::string>> key_value_pairs;
         * key_value_pairs.emplace_back("key1", "value1");
         * key_value_pairs.emplace_back("key2", "value2");
         * key_value_pairs.emplace_back("key1", "value1_override");
         * key_value_pairs.emplace_back("key3", "value3");
         *
         * mpicxx::info obj(key_value_pairs.begin(), key_value_pairs.end());
         * @endcode
         * Results in the following [key, value]-pairs stored in the info object (not necessarily in this order): \n
         * `["key1", "value1_override"]`, `["key2", "value2"]` and `["key3", "value3"]`
         *
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid range, i.e. @p first must be less or equal than @p last.
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         * @pre The length of **any** value **must** be greater than 0 and less than *MPI_MAX_INFO_VAL*.
         * @post The newly constructed info object is in a valid state.
         *
         * @assert{
         * If @p first and @p last don't denote a valid range. \n
         * If any key or value exceed their size limit.
         * }
         *
         * @calls{
         * int MPI_Info_create(MPI_Info *info);                                         // exactly once
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);         // exactly 'last - first' times
         * }
         */
        template <std::input_iterator InputIter>
        info(InputIter first, InputIter last) : info() {
            // default construct the info object via the default constructor
            // add all [key, value]-pairs
            this->insert_or_assign(first, last);
        }
        /**
         * @brief Constructs the info object with the contents of the initializer list @p init.
         * @details If multiple [key, value]-pairs in the range share the same key, the **last** occurrence determines the final value.
         * @param[in] init initializer list to initialize the [key, value]-pairs of the info object with
         *
         * Example:
         * @code{.cpp}
         * mpicxx::info obj = { {"key1", "value1"},
         *                      {"key2", "value2"},
         *                      {"key1", "value1_override"},
         *                      {"key3", "value3"} };
         * @endcode
         * Results in the following [key, value]-pairs stored in the info object (not necessarily in this order):\n
         * `["key1", "value1_override"]`, `["key2", "value2"]` and `["key3", "value3"]`
         *
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         * @pre The length of **any** value **must** be greater than 0 and less than *MPI_MAX_INFO_VAL*.
         * @post The newly constructed info object is in a valid state.
         *
         * @assert{ If any key or value exceed their size limit. }
         *
         * @calls{
         * int MPI_Info_create(MPI_Info *info);                                         // exactly once
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);         // exactly 'init.size()' times
         * }
         */
        info(std::initializer_list<value_type> init) : info() {
            // default construct the info object via the default constructor
            // add all [key, value]-pairs
            this->insert_or_assign(init);
        }
        /**
         * @brief Wrap a *MPI_Info* object in an mpicxx::info object.
         * @param[in] other the raw *MPI_Info* object
         * @param[in] is_freeable mark whether the *MPI_Info* object wrapped in this info object should be automatically
         * freed at the end of its lifetime
         *
         * @post The newly constructed info object is in a valid state iff @p other was in a valid state (i.e. was **not** *MPI_INFO_NULL*).
         * @attention If @p is_freeable is set to `false`, **the user** has to ensure that the *MPI_Info* object @p other gets properly
         * freed (via a call to *MPI_Info_free*) at the end of its lifetime.
         *
         * @assert{ If @p other equals to *MPI_INFO_NULL* or *MPI_INFO_ENV* **and** @p is_freeable is set to `true`. }
         */
        constexpr info(MPI_Info other, const bool is_freeable) noexcept : info_(other), is_freeable_(is_freeable) {
            MPICXX_ASSERT(!(other == MPI_INFO_NULL && is_freeable == true), "'MPI_INFO_NULL' can't be marked freeable!");
            MPICXX_ASSERT(!(other == MPI_INFO_ENV && is_freeable == true), "'MPI_INFO_ENV' can't be marked freeable!");
        }
        /**
         * @brief Destructs the info object.
         * @details Only calls *MPI_Info_free* if:
         *      - The info object is marked freeable. Only objects created through @ref info(MPI_Info, const bool) can be marked as
         *        non-freeable (or info objects which are moved-from such objects). \n
         *        For example info::env is **non-freeable** due to the fact that the MPI runtime system would crash if
         *        *MPI_Info_free* is called with *MPI_INFO_ENV*.
         *      - The info object ist **not** in the moved-from state.
         *
         * If any of these conditions are **not** fulfilled, no free function will be called (because doing so is unnecessary and would lead
         * to a crash of the MPI runtime system).
         *
         * @post Invalidates **all** iterators referring to `*this`.
         *
         * @calls{
         * int MPI_Info_free(MPI_info *info);       // at most once
         * }
         */
        ~info() {
            // destroy info object if necessary
            if (is_freeable_ && info_ != MPI_INFO_NULL) {
                MPI_Info_free(&info_);
            }
        }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            assignment operators                                            //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Copy assignment operator. Replaces the contents with a copy of the contents of @p other.
         * @details Retains @p rhs's [key, value]-pair ordering. Gracefully handles self-assignment.
         * @param[in] rhs another info object to use as data source
         * @return `*this`
         *
         * @pre @p rhs **must not** be in the moved-from state.
         * @post The assigned to info object is in a valid state.
         * @attention Every copied info object is marked **freeable** independent of the **freeable** state of the copied-from info object.
         *
         * @assert{ If @p rhs is in the moved-from state. }
         *
         * @calls{
         * int MPI_Info_free(MPI_info *info);                       // at most once
         * int MPI_Info_dup(MPI_info info, MPI_info *newinfo);      // at most once
         * }
         */
        info& operator=(const info& rhs) {
            MPICXX_ASSERT(rhs.info_ != MPI_INFO_NULL, "'rhs' is in the moved-from state!");

            // check against self-assignment
            if (this != std::addressof(rhs)) {
                // delete current MPI_Info object iff it is marked as freeable and in a valid state
                if (is_freeable_ && info_ != MPI_INFO_NULL) {
                    MPI_Info_free(&info_);
                }
                // copy rhs info object
                MPI_Info_dup(rhs.info_, &info_);
                is_freeable_ = true;
            }
            return *this;
        }
        /**
         * @brief Move assignment operator. Replaces the contents with contents of @p other using move semantics.
         * @details Retains @p rhs's [key, value]-pair ordering. Does **not** handle self-assignment
         * (as of https://isocpp.org/wiki/faq/assignment-operators).
         * @param[in] rhs another info object to use as data source
         * @return `*this`
         *
         * @post The assigned to info object is in a valid state iff @p rhs was in a valid state.
         * @post @p rhs is now in the moved-from state.
         * @post All iterators referring to @p rhs remain valid, but now refer to `*this`.
         *
         * @calls{
         * int MPI_Info_free(MPI_info *info);       // at most once
         * }
         */
        info& operator=(info&& rhs) {
            // delete current MPI_Info object iff it is marked as freeable and in a valid state
            if (is_freeable_ && info_ != MPI_INFO_NULL) {
                MPI_Info_free(&info_);
            }
            // transfer ownership
            info_ = std::move(rhs.info_);
            is_freeable_ = std::move(rhs.is_freeable_);
            // set rhs to the moved-from state
            rhs.info_ = MPI_INFO_NULL;
            rhs.is_freeable_ = false;
            return *this;
        }
        /**
         * @brief Replaces the contents with those identified by the
         * [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) @p ilist.
         * @details If multiple [key, value]-pairs in the range share the same key, the **last** occurrence determines the final value.
         * @param[in] ilist initializer list to initialize the [key, value]-pairs of the info object with
         * @return `*this`
         *
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         * @pre The length of **any** value **must** be greater than 0 and less than *MPI_MAX_INFO_VAL*.
         * @post The assigned to info object is in a valid state.
         *
         * @assert{ If any key or value exceed their size limit. }
         *
         * @calls{
         * int MPI_Info_free(MPI_info *info);                                           // at most once
         * int MPI_Info_create(MPI_Info *info);                                         // exactly once
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);         // exactly 'init.size()' times
         * }
         */
        info& operator=(std::initializer_list<value_type> ilist) {
            // delete current MPI_Info object iff it is marked as freeable and in a valid state
            if (is_freeable_ && info_ != MPI_INFO_NULL) {
                MPI_Info_free(&info_);
            }
            // recreate the info object
            MPI_Info_create(&info_);
            is_freeable_ = true;
            // add all [key, value]-pairs
            this->insert_or_assign(ilist);
            return *this;
        }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  iterators                                                 //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Returns an @ref iterator to the first [key, value]-pair of the info object.
         * @details If the info object is empty, the returned @ref iterator will be equal to @ref end().
         * @return iterator to the first [key, value]-pair
         *
         * @pre `*this` **must not** be in the moved-from state.
         *
         * @assert{ If `*this` is in the moved-from state. }
         *
         * @calls_ref{ For *MPI* functions called while using an iterator see the @ref iterator_impl documentation. }
         */
        [[nodiscard]] iterator begin() {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            return iterator(info_, 0);
        }
        /**
         * @brief Returns an @ref iterator to the element following the last [key, value]-pair of the info object.
         * @details This element acts as a placeholder; attempting to access it results in **undefined behavior**.
         * @return @ref iterator to the element following the last [key, value]-pair
         *
         * @pre `*this` **must not** be in the moved-from state.
         *
         * @assert{ If `*this` is in the moved-from state. }
         *
         * @calls_ref{
         * @code{.cpp} int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys);    // exactly once @endcode
         * For *MPI* functions called while using an iterator see the @ref iterator_impl documentation.
         * }
         */
        [[nodiscard]] iterator end() {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            return iterator(info_, this->size());
        }
        /**
         * @brief Returns a @ref const_iterator to the first [key, value]-pair of the info object.
         * @details If the info object is empty, the returned @ref const_iterator will be equal to @ref cend().
         * @return @ref const_iterator to the first [key, value]-pair
         *
         * @pre `*this` **must not** be in the moved-from state.
         *
         * @assert{ If `*this` is in the moved-from state. }
         *
         * @calls_ref{ For *MPI* functions called while using an iterator see the @ref iterator_impl documentation. }
         */
        [[nodiscard]] const_iterator begin() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            return const_iterator(info_, 0);
        }
        /**
         * @brief Returns a @ref const_iterator to the element following the last [key, value]-pair of the info object.
         * @details This element acts as a placeholder; attempting to access it results in **undefined behavior**.
         * @return @ref const_iterator to the element following the last [key, value]-pair
         *
         * @pre `*this` **must not** be in the moved-from state.
         *
         * @assert{ If `*this` is in the moved-from state. }
         *
         * @calls_ref{
         * @code{.cpp} int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys);    // exactly once @endcode
         * For *MPI* functions called while using an iterator see the @ref iterator_impl documentation.
         * }
         */
        [[nodiscard]] const_iterator end() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            return const_iterator(info_, this->size());
        }
        /**
         * @copydoc begin() const
         */
        [[nodiscard]] const_iterator cbegin() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            return const_iterator(info_, 0);
        }
        /**
         * @copydoc end() const
         */
        [[nodiscard]] const_iterator cend() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            return const_iterator(info_, this->size());
        }

        /**
         * @brief Returns a @ref reverse_iterator to the first [key, value]-pair of the reversed info object.
         * @details It corresponds to the last [key, value]-pair of the non-reversed info object.
         * If the info object is empty, the returned @ref reverse_iterator will be equal to @ref rend().
         * @return @ref reverse_iterator to the first [key, value]-pair
         *
         * @pre `*this` **must not** be in the moved-from state.
         *
         * @assert{ If `*this` is in the moved-from state. }
         *
         * @calls_ref{
         * @code{.cpp} int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys);    // exactly once @endcode
         * For *MPI* functions called while using an iterator see the @ref iterator_impl documentation.
         * }
         */
        [[nodiscard]] reverse_iterator rbegin() {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            return std::make_reverse_iterator(this->end());
        }
        /**
         * @brief Returns a @ref reverse_iterator to the element following the last [key, value]-pair of the reversed info object.
         * @details It corresponds to the element preceding the first [key, value]-pair of the non-reversed info object.
         * This element acts as a placeholder, attempting to access it results in **undefined behavior**.
         * @return @ref reverse_iterator to the element following the last [key, value]-pair
         *
         * @pre `*this` **must not** be in the moved-from state.
         *
         * @assert{ If `*this` is in the moved-from state. }
         *
         * @calls_ref{ For *MPI* functions called while using an iterator see the @ref iterator_impl documentation. }
         */
        [[nodiscard]] reverse_iterator rend() {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            return std::make_reverse_iterator(this->begin());
        }
        /**
         * @brief Returns a @ref const_reverse_iterator to the first [key, value]-pair of the reversed info object.
         * @details It corresponds to the last [key, value]-pair of the non-reversed info object.
         * If the info object is empty, the returned @ref const_reverse_iterator will be equal to @ref crend().
         * @return @ref const_reverse_iterator to the first [key, value]-pair
         *
         * @pre `*this` **must not** be in the moved-from state.
         *
         * @assert{ If `*this` is in the moved-from state. }
         *
         * @calls_ref{
         * @code{.cpp} int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys);    // exactly once @endcode
         * For *MPI* functions called while using an iterator see the @ref iterator_impl documentation.
         * }
         */
        [[nodiscard]] const_reverse_iterator rbegin() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            return std::make_reverse_iterator(this->cend());
        }
        /**
         * @brief Returns a @ref const_reverse_iterator to the element following the last [key, value]-pair of the reversed info object.
         * @details It corresponds to the element preceding the first [key, value]-pair of the non-reversed info object.
         * This element acts as a placeholder, attempting to access it results in **undefined behavior**.
         * @return @ref const_reverse_iterator to the element following the last [key, value]-pair
         *
         * @pre `*this` **must not** be in the moved-from state.
         *
         * @assert{ If `*this` is in the moved-from state. }
         *
         * @calls_ref{ For *MPI* functions called while using an iterator see the @ref iterator_impl documentation. }
         */
        [[nodiscard]] const_reverse_iterator rend() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            return std::make_reverse_iterator(this->cbegin());
        }
        /**
         * @copydoc rbegin() const
         */
        [[nodiscard]] const_reverse_iterator crbegin() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            return std::make_reverse_iterator(this->cend());
        }
        /**
         * @copydoc rend() const
         */
        [[nodiscard]] const_reverse_iterator crend() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            return std::make_reverse_iterator(this->cbegin());
        }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  capacity                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Checks if the info object has no [key, value]-pairs, i.e. whether `begin() == end()`.
         * @return `true` if the info object is empty, `false` otherwise
         *
         * @pre `*this` **must not** be in the moved-from state.
         *
         * @assert{ If `*this` is in the moved-from state. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);       // exactly once
         * }
         */
        [[nodiscard]] bool empty() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            return this->size() == 0;
        }
        /**
         * @brief Returns the number of [key, value]-pairs in the info object, i.e.
         * [`std::distance`](https://en.cppreference.com/w/cpp/iterator/distance)`(begin(), end())`.
         * @return the number of [key, value]-pairs in the info object
         *
         * @pre `*this` **must not** be in the moved-from state.
         *
         * @assert{ If `*this` is in the moved-from state. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);       // exactly once
         * }
         */
        [[nodiscard]] size_type size() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            int nkeys;
            MPI_Info_get_nkeys(info_, &nkeys);
            return static_cast<size_type>(nkeys);
        }
        /**
         * @brief Returns the maximum number of [key, value]-pairs an info object is able to hold due to system or library implementation
         * limitations, i.e. [`std::distance`](https://en.cppreference.com/w/cpp/iterator/distance)`(begin(), end())` for the largest
         * info object.
         * @return maximum number of [key, value]-pairs
         *
         * @attention This value typically reflects the theoretical limit on the size of the info object, at most
         * [`std::numeric_limits<`](https://en.cppreference.com/w/cpp/types/numeric_limits)@ref difference_type[`>::%max()`]
         * (https://en.cppreference.com/w/cpp/types/numeric_limits).
         * At runtime, the size of the info object may be limited to a value smaller than max_size() by the amount of RAM available.
         */
        [[nodiscard]] static constexpr size_type max_size() {
            return std::numeric_limits<difference_type>::max();
        }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  modifier                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Access the value associated with the given @p key including bounds checks.
         * @details Returns a proxy class, which is used to distinguish between read and write accesses.
         * @param[in] key the @p key of the [key, value]-pair to find (must meet the requirements of the @p detail::string concept)
         * @return a proxy object
         *
         * Example:
         * @code{.cpp}
         * mpicxx::info obj = { {"key", "foo"} };
         * try {
         *     obj.at("key") = "bar";                   // write access
         *     std::string str_val = obj.at("key");     // read access: returns a proxy object, which is immediately casted to a std::string
         *     str_val = "baz";                         // changing str_val will (obviously) not change the value of obj.at("key")
         *
         *     // same as: obj.at("key") = "baz";
         *     auto val = obj.at("key");                // read access: returns a proxy object
         *     val = "baz";                             // write access: now obj.at("key") will return "baz"
         *
         *     obj.at("key_2") = "baz";                 // will throw
         * } catch (const std::out_of_range& e) {
         *     std::cerr << e.what() << std::endl;      // prints: "key_2 doesn't exist!"
         * }
         * @endcode
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         * @pre The @p key **must** already exist, otherwise a
         * [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) exception will be thrown.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @throws std::out_of_range if the info object does not have a [key, value]-pair with the specified @p key
         *
         * @calls_ref{
         * @code{.cpp}
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
         * @endcode
         * For *MPI* functions called while using a proxy see the @ref proxy documentation.
         * }
         */
        proxy at(detail::string auto&& key) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "*this is in the \"moved-from\" state.");
            MPICXX_ASSERT(0 < std::string_view(key).size() && std::string_view(key).size() < MPI_MAX_INFO_KEY,
                          "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                          std::string_view(key).size(), MPI_MAX_INFO_KEY);

            // check whether the key exists
            if (!this->key_exists(key)) {
                // key doesn't exist
                throw std::out_of_range(std::string(std::forward<decltype(key)>(key)) + std::string(" doesn't exist!"));
            }
            // create proxy object and forward key
            return proxy(info_, std::forward<decltype(key)>(key));
        }
        /**
         * @brief Access the value associated with the given @p key including bounds checks.
         * @param[in] key the @p key of the [key, value]-pair to find
         * @return the value associated with @p key
         *
         * Example:
         * @code{.cpp}
         * const mpicxx::info obj = { {"key", "foo"} };
         * try {
         *     obj.at("key") = "bar";                      // write access: modifying a temporary is nonsensical
         *     std::string str_val = obj.at("key");        // read access: directly returns a std::string
         *     str_val = "baz";                            // changing str_val will (obviously) not change the value of obj.at("key")
         *
         *     auto val = obj.at("key");                   // read access: directly returns a std::string
         *     val = "baz";                                // typeof val is std::string -> changing val will not change the value of obj.at("key)"
         *
         *     std::string throw_val = obj.at("key_2");    // will throw
         * } catch (const std::out_of_range& e) {
         *     std::cerr << e.what() << std::endl;         // prints: "key_2 doesn't exist!"
         * }
         * @endcode
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         * @pre The @p key **must** already exist, otherwise a
         * [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) exception will be thrown.
         * @attention This const overload does **not** return a proxy object but a
         * [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string)!
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @throws std::out_of_range if the info object does not have a [key, value]-pair with the specified @p key
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // at most once
         * }
         */
        std::string at(const std::string_view key) const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(0 < key.size() && key.size() < MPI_MAX_INFO_KEY,
                          "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                          key.size(), MPI_MAX_INFO_KEY);

            // get the length of the value associated with key
            int valuelen, flag;
            MPI_Info_get_valuelen(info_, key.data(), &valuelen, &flag);
            // check whether the key exists
            if (!static_cast<bool>(flag)) {
                // key doesn't exist
                throw std::out_of_range(std::string(std::forward<decltype(key)>(key)) + std::string(" doesn't exist!"));
            }
            // get the value associated with key
            std::string value(valuelen, ' ');
            MPI_Info_get(info_, key.data(), valuelen, value.data(), &flag);
            return value;
        }
        /**
         * @brief Access the value associated with the given @p key.
         * @details Returns a proxy class which is used to distinguish between read and write access.
         *
         * `operator[]` is non-const because it inserts the @p key if it doesn't exist. If this behavior is undesirable or if the container
         * is const, `at()` may be used.
         * @param[in] key the @p key of the [key, value]-pair to find (must meet the requirements of the detail::string concept)
         * @return a proxy object
         *
         * Example:
         * @code{.cpp}
         * mpicxx::info obj = { {"key", "foo"} };
         *
         * obj["key"] = "bar";                 // write access
         * std::string str_val = obj["key"];   // read access: returns a proxy object, which is immediately casted to a std::string
         * str_val = "baz";                    // changing val won't alter obj["key"] !!!
         *
         * // same as: obj["key"] = "baz";
         * auto val = obj["key"];              // read access: returns a proxy object
         * val = "baz";                        // write access: now obj["key"] will return "baz"
         *
         * obj["key_2"] = "baz";               // inserts a new [key, value]-pair in obj
         * @endcode
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this` are
         * invalidated, if an insertion took place. \n
         * invalidated, if an insertion took place. \n
         * Specific MPI implementations **may** differ in this regard, i.e. iterators before the insertion point remain valid, all other
         * iterators are invalidated.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls_ref{
         * For *MPI* functions called while using a proxy see the @ref proxy documentation.
         * }
         */
        proxy operator[](detail::string auto&& key) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(0 < std::string_view(key).size() && std::string_view(key).size() < MPI_MAX_INFO_KEY,
                          "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                          std::string_view(key).size(), MPI_MAX_INFO_KEY);

            // create proxy object and forward key
            return proxy(info_, std::forward<decltype(key)>(key));
        }

        /**
         * @brief Insert the given [key, value]-pair if the info object doesn't already contain a [key, value]-pair with an equivalent key.
         * @param[in] key @p key of the [**key**, value]-pair to insert
         * @param[in] value @p value of the [key, **value**]-pair to insert
         * @return a pair consisting of an iterator to the inserted [key, value]-pair (or the one that prevented the insertion) and a `bool`
         * denoting whether the insertion took place
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre **Both** @p key **and** @p value **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         * @pre The @p value's length **must** be greater than 0 and less than *MPI_MAX_INFO_VAL*.
         * @post As of the [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         * are invalidated, if an insertion took place (i.e. the returned `bool` is `true`). \n
         * Specific MPI implementations **may** differ in this regard, i.e. iterators before the insertion point remain valid, all other
         * iterators are invalidated.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p key or @p value exceed their size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);      // exactly once
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                      // at most once
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                        // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                 // at most 'this->size()' times
         * }
         */
        std::pair<iterator, bool> insert(const std::string_view key, const std::string_view value) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(0 < key.size() && key.size() < MPI_MAX_INFO_KEY,
                          "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                          key.size(), MPI_MAX_INFO_KEY);
            MPICXX_ASSERT(0 < value.size() && value.size() < MPI_MAX_INFO_VAL,
                          "Illegal info value: 0 < %u < %i (MPI_MAX_INFO_VAL)",
                          value.size(), MPI_MAX_INFO_VAL);

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
         * [key, value]-pair with an equivalent key.
         * @details If multiple [key, value]-aprse in the range have the same key, the **first** occurrence determines the final value.
         * @tparam InputIt must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator).
         * @param[in] first iterator to the first [key, value]-pair in the range
         * @param[in] last iterator one-past the last [key, value]-pair in the range
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid range, i.e. @p first must be less or equal than @p last.
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         * @pre The length of **any** value **must** be greater than 0 and less than *MPI_MAX_INFO_VAL*.
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         * are invalidated, if an insertion took place. \n
         * Specific MPI implementations **may** differ in this regard, i.e. iterators before the first insertion point remain valid, all
         * other iterators are invalidated.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p first and @p last don't denote a valid range. \n
         * If any key or value exceed their size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly 'last - first' times
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                         // at most 'last - first' times
         * }
         */
        template <std::input_iterator InputIt>
        void insert(InputIt first, InputIt last) requires (!std::is_constructible_v<std::string, InputIt>) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(first <= last, "Iterator 'first' must be less or equal than iterator 'last'!");

            // try to insert every element in the range [first, last)
            for (; first != last; ++first) {
                // retrieve element
                const auto& [key, value] = *first;

                MPICXX_ASSERT(0 < key.size() && key.size() < MPI_MAX_INFO_KEY,
                              "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                              key.size(), MPI_MAX_INFO_KEY);
                MPICXX_ASSERT(0 < value.size() && value.size() < MPI_MAX_INFO_VAL,
                              "Illegal info value: 0 < %u < %i (MPI_MAX_INFO_VAL)",
                              value.size(), MPI_MAX_INFO_VAL);

                // check whether the key exists
                if (!this->key_exists(key)) {
                    // key doesn't exist -> add new [key, value]-pair
                    MPI_Info_set(info_, key.data(), value.data());
                }
            }
        }
        /**
         * @brief Inserts all [key, value]-pairs from the initializer list @p ilist if the info object does not already contain a
         * [key, value]-pair with an equivalent key.
         * @details If multiple [key, value]-pairs in the initializer list have the same key, the **first** occurrence determines the final
         * value.
         * @param[in] ilist initializer list to insert the [key, value]-pairs from
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         * @pre The length of **any** value **must** be greater than 0 and less than *MPI_MAX_INFO_VAL*.
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         * are invalidated, if an insertion took place. \n
         * Specific MPI implementations **may** differ in this regard, i.e. iterators before the first insertion point remain valid, all
         * other iterators are invalidated.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If any key or value exceed their size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                           // exactly 'ilist.size()' times
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);         // at most 'ilist.size()' times
         * }
         */
        void insert(std::initializer_list<value_type> ilist) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            this->insert(ilist.begin(), ilist.end());
        }

        /**
         * @brief Insert or assign the given [key, value]-pair to the info object.
         * @param[in] key @p key of the [**key**, value]-pair to insert
         * @param[in] value @p value of the [key, **value**]-pair to insert
         * @return a pair consisting of an iterator to the inserted or assigned [key, value]-pair and a `bool`
         * denoting whether the insertion (`true`) or the assignment (`false`) took place
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre **Both** @p key **and** @p value **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         * @pre The @p value's length **must** be greater than 0 and less than *MPI_MAX_INFO_VAL*.
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         * are invalidated, if an insertion took place (i.e. the returned `bool` is `true`). \n
         * Specific MPI implementations **may** differ in this regard, i.e. iterators before the insertion point remain valid, all other
         * iterators are invalidated.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p key or @p value exceed their size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                         // exactly once
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                           // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                    // at most 'this->size()' times
         * }
         */
        std::pair<iterator, bool> insert_or_assign(const std::string_view key, const std::string_view value) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(0 < key.size() && key.size() < MPI_MAX_INFO_KEY,
                          "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                          key.size(), MPI_MAX_INFO_KEY);
            MPICXX_ASSERT(0 < value.size() && value.size() < MPI_MAX_INFO_VAL,
                          "Illegal info value: 0 < %u < %i (MPI_MAX_INFO_VAL)",
                          value.size(), MPI_MAX_INFO_VAL);

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
         * @tparam InputIt must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator).
         * @param[in] first iterator to the first [key, value]-pair in the range
         * @param[in] last iterator one-past the last [key, value]-pair in the range
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid range, i.e. @p first must be less or equal than @p last.
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         * @pre The length of **any** value **must** be greater than 0 and less than *MPI_MAX_INFO_VAL*.
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         * are invalidated, if an insertion took place. \n
         * Specific MPI implementations **may** differ in this regard, i.e. iterators before the first insertion point remain valid, all
         * other iterators are invalidated.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p first and @p last don't denote a valid range. \n
         * If any key or value exceed their size limit.
         * }
         *
         * @calls{
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);         // exactly 'last - first' times
         * }
         */
        template <std::input_iterator InputIt>
        void insert_or_assign(InputIt first, InputIt last) requires (!std::is_constructible_v<std::string, InputIt>) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(first <= last, "Iterator 'first' must be less or equal than iterator 'last'!");

            // insert or assign every element in the range [first, last)
            for (; first != last; ++first) {
                // retrieve element
                const auto [key, value] = *first;
                MPICXX_ASSERT(0 < key.size() && key.size() < MPI_MAX_INFO_KEY,
                              "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                              key.size(), MPI_MAX_INFO_KEY);
                MPICXX_ASSERT(0 < value.size() && value.size() < MPI_MAX_INFO_VAL,
                              "Illegal info value: 0 < %u < %i (MPI_MAX_INFO_VAL)",
                              value.size(), MPI_MAX_INFO_VAL);

                // insert or assign [key, value]-pair
                MPI_Info_set(info_, key.data(), value.data());
            }
        }
        /**
         * @brief Inserts or assigns [key, value]-pairs from the initializer list @p ilist to the info object.
         * @details If multiple [key, value]-pairs in the initializer list have the same key, the **last** occurrence determines the final
         * value.
         * @param[in] ilist initializer list to insert or assign the [key, value]-pairs from
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         * @pre The length of **any** value **must** be greater than 0 and less than *MPI_MAX_INFO_VAL*.
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         * are invalidated, if an insertion took place. \n
         * Specific MPI implementations **may** differ in this regard, i.e. iterators before the first insertion point remain valid, all
         * other iterators are invalidated.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If any key or value exceed their size limit.
         * }
         *
         * @calls{
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);         // exactly 'ilist.size()' times
         * }
         */
        void insert_or_assign(std::initializer_list<value_type> ilist) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            this->insert_or_assign(ilist.begin(), ilist.end());
        }

        /**
         * @brief Erase all [key, value]-pairs from the info object.
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @post The info object is empty, i.e. `this->size() == 0` respectively `this->empty() == true`.
         * @post Invalidates **all** iterators referring to `*this`.
         *
         * @assert{ If `*this` is in the moved-from state. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);    // exactly 'this->size()' times
         * int MPI_Info_delete(MPI_Info info, const char *key);         // exactly 'this->size()' times
         * }
         */
        void clear() {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

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
         * @details The iterator @p pos must be valid and **dereferenceable**. Thus the @ref end() iterator (which is valid, but is
         * not dereferencable) cannot be used as a value for @p pos.
         * @param[in] pos iterator to the [key, value]-pair to remove
         * @return iterator following the removed [key, value]-pair (= position of @p pos prior to removal); \n
         * if @p pos refers to the last [key, value]-pair, then the @ref end() iterator is returned
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p pos **must** refer to `*this` info object.
         * @pre The position denoted by @p pos **must** be in the half-open interval [0, `this->size()`).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         * are invalidated. \n
         * Specific MPI implementations **may** differ in this regard, i.e. iterators before the point of erase remain valid, all other
         * iterators are invalidated.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p pos does not refer to `*this` info object. \n
         * If attempting an illegal dereferencing.
         * }
         *
         * @calls{
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // exactly once
         * int MPI_Info_delete(MPI_Info info, const char *key);             // exactly once
         * }
         */
        iterator erase(const_iterator pos) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(*pos.info_ == info_, "Iterator 'pos' must refer to the same info object as '*this'!");
            MPICXX_ASSERT(pos.pos_ >= 0 && pos.pos_ < static_cast<int>(this->size()),
                          "Iterator 'pos' not dereferenceable (legal range: [0, %u), requested position: %i)!",
                          this->size(), pos.pos_);

            char key[MPI_MAX_INFO_KEY];
            MPI_Info_get_nthkey(info_, pos.pos_, key);
            MPI_Info_delete(info_, key);
            return iterator(info_, pos.pos_);
        }
        /**
         * @brief Removes all [key, value]-pairs in the range [@p first, @p last).
         * @details [@p first, @p last) must be a valid range in `*this`. The iterator @p first does not need to be dereferenceable if
         * `first == last`: erasing an empty range is a no-op.
         *
         * The [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) only guarantees that the number of a
         * given key does not change **as long as** no call to *MPI_Info_set* or *MPI_Info_delete* is made.
         *
         * Therefore (to be compliant with the standard) this function performs two steps:
         * 1. Save all keys of the [key, value]-pairs contained in [@p first, @p last) in a
         * [`std::vector<std::string>>`](https://en.cppreference.com/w/cpp/container/vector).
         * 2. Delete all [key, value]-pairs in the info object @p c with a key contained in the previously created vector.
         * @param[in] first iterator to the first [key, value]-pair in the range
         * @param[in] last iterator one-past the last [key, value]-pair in the range
         * @return iterator following the last removed [key, value]-pair (= position of @p first prior to any removal); \n
         * if `last == end()` prior to removal, then the updated @ref end() iterator is returned; \n
         * if [@p first, @p last) is an empty range, then @p last is returned
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p first and @p last **must** refer to `*this` info object
         * @pre The position denoted by @p first **must** be in the interval [0, `this->size()`].
         * @pre The position denoted by @p last **must** be in the interval [0, `this->size()`].
         * @pre @p first **must** be less or equal than @p last.
         * @post As of the [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         * are invalidated. \n
         * Specific MPI implementations **may** differ in this regard, i.e. iterators before the first point of erase remain valid, all
         * other iterators are invalidated.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p first or @p last does not refer to `*this` info object. \n
         * If attempting an illegal dereferencing. \n
         * If @p first is greater than @p last.
         * }
         *
         * @calls{
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // exactly 'last - first' times
         * int MPI_Info_delete(MPI_Info info, const char *key);             // exactly 'last - first' times
         * }
         */
        iterator erase(const_iterator first, const_iterator last) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(*first.info_ == info_, "Iterator 'first' must refer to the same info object as '*this'!");
            MPICXX_ASSERT(*last.info_ == info_, "Iterator 'last' must refer to the same info object as '*this'!");
            MPICXX_ASSERT(first.pos_ >= 0 && first.pos_ <= static_cast<int>(this->size()),
                          "Iterator 'first' not dereferenceable (legal interval: [0, %u], requested position: %i)!",
                          this->size(), first.pos_);
            MPICXX_ASSERT(last.pos_ >= 0 && last.pos_ <= static_cast<int>(this->size()),
                          "Iterator 'last' not dereferenceable (legal interval: [0, %u], requested position: %i)!",
                          this->size(), last.pos_);
            MPICXX_ASSERT(first <= last, "Iterator 'first' must be less or equal than iterator 'last'!");

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
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         * are invalidated, if an erasure took place (i.e. the returned `size_type` is `1`). \n
         * Specific MPI implementations **may** differ in this regard, i.e. iterators before the point of erase remain valid, all other
         * iterators are invalidated.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);      // exactly once
         * int MPI_Info_delete(MPI_Info info, const char *key);                                      // at most once
         * }
         */
        size_type erase(const std::string_view key) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(0 < key.size() && key.size() < MPI_MAX_INFO_KEY,
                          "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                          key.size(), MPI_MAX_INFO_KEY);

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
         * @post `*this` is in a valid state iff @p other was in a valid state (and vice versa).
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
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p pos **must** refer to `*this` info object.
         * @pre The position denoted by @p pos **must** be in the half-open interval [0, `this->size()`).
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         * are invalidated. \n
         * Specific MPI implementations **may** differ in this regard, i.e. iterators before the point of extraction remain valid, all other
         * iterators are invalidated.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p pos does not refer to `*this` info object. \n
         * If attempting an illegal dereferencing.
         * }
         *
         * @calls{
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                    // exactly once
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // exactly once
         * int MPI_Info_delete(MPI_Info info, const char *key);                                         // exactly once
         * }
         */
        [[nodiscard]] value_type extract(const_iterator pos) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(*pos.info_ == info_, "Iterator 'pos' must refer to the same info object as '*this'!");
            MPICXX_ASSERT(pos.pos_ >= 0 && pos.pos_ < static_cast<int>(this->size()),
                          "Iterator 'pos' not dereferenceable (legal interval: [0, %u), requested position: %i)!",
                          this->size(), pos.pos_);

            // get [key, value]-pair pointed to by pos
            value_type key_value_pair = *pos;
            // remove [key, value]-pair from info object
            MPI_Info_delete(info_, key_value_pair.first.data());
            // return extracted [key, value]-pair
            return key_value_pair;
        }
        /**
         * @brief Removes the [key, value]-pair (if one exists) with the key equivalent to @p key and returns the removed [key, value]-pair.
         * @details Returns a [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional) holding the removed [key, value]-pair
         * if the @p key exists, [`std::nullopt`](https://en.cppreference.com/w/cpp/utility/optional/nullopt) otherwise.
         * @param[in] key the @p key to extract
         * @return the extracted [key, value]-pair
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         * are invalidated, if an extraction took place. \n
         * Specific MPI implementations **may** differ in this regard, i.e. iterators before the point of extraction remain valid, all other
         * iterators are invalidated.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // at most once
         * int MPI_Info_delete(MPI_Info info, const char *key);                                         // at most once
         * }
         */
        [[nodiscard]] std::optional<value_type> extract(const std::string_view key) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(0 < key.size() && key.size() < MPI_MAX_INFO_KEY,
                          "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                          key.size(), MPI_MAX_INFO_KEY);

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
         * [key, value]-pair is not extracted from @p source.
         *
         * Directly returns if a "self-extraction" is attempted.
         *
         * The [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) only guarantees that the number of a
         * given key does not change **as long as** no call to *MPI_Info_set* or *MPI_Info_delete* is made.
         *
         * * Therefore (to be compliant with the standard) this function performs two steps:
         * 1. Save all keys of the [key, value]-pairs that will be extracted in a
         * [`std::vector<std::string>>`](https://en.cppreference.com/w/cpp/container/vector). Add the respective [key, value]-pairs to
         * `*this`.
         * 2. Delete all [key, value]-pairs in @p source with a key contained in the previously created vector.
         * @param[inout] source the info object to transfer the [key, value]-pairs from
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p source **must not** be in the moved-from state.
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to `*this`
         * and @p source are invalidated, if a transfer of [key, value]-pairs took place. \n
         * Specific MPI implementations **may** differ in this regard, i.e. iterators before the first point of insertion/extraction remain
         * valid, all other iterators are invalidated.
         *
         * @assert{ If `*this` or @p source are in the moved-from state. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                           // exactly once
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // at least 'source.size()' times, at most '2 * source.size()' times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // at most 'source.size()' times
         * int MPI_Info_delete(MPI_Info info, const char *key);                                         // at most 'source.size()' times
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                         // at most 'source.size()' times
         * }
         */
        void merge(info& source) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(source.info_ != MPI_INFO_NULL, "'source' is in the moved-from state!");

            // do nothing if a "self-merge" is attempted
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
                    char* source_value = new char[valuelen + 1];
                    MPI_Info_get(source.info_, source_key, valuelen, source_value, &flag);
                    // remember the source's key
                    keys_to_delete.emplace_back(source_key);
                    // add [key, value]-pair to *this info object
                    MPI_Info_set(info_, source_key, source_value);
                    delete[] source_value;
                }
            }

            // delete all [key, value]-pairs merged into *this info object from source
            for (const auto& str : keys_to_delete) {
                MPI_Info_delete(source.info_, str.data());
            }
        }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                   lookup                                                   //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Returns the number of [key, value]-pairs with key equivalent to @p key.
         * @details Since info objects don't allow duplicated keys the returned value is either 0 (key not found) or 1 (key found).
         *
         * Therefore @ref contains(const std::string_view) const may be a better choice.
         * @param[in] key @p key value of the [key, value]-pairs to count
         * @return number of [key, value]-pairs with key equivalent to @p key, which is either 0 or 1
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most 'this->size()' times
         * }
         */
        [[nodiscard]] size_type count(const std::string_view key) const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(0 < key.size() && key.size() < MPI_MAX_INFO_KEY,
                          "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                          key.size(), MPI_MAX_INFO_KEY);

            return static_cast<size_type>(this->contains(key));
        }
        /**
         * @brief Finds a [key, value]-pair with key equivalent to @p key.
         * @details If the key is found, returns an iterator pointing to the corresponding [key, value]-pair,
         * otherwise the past-the-end iterator is returned (see @ref end()).
         * @param[in] key @p key value of the [key, value]-pair to search for
         * @return iterator to a [key, value]-pair with key equivalent to @p key or the past-the-end iterator if no such key is found
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most 'this->size()' times
         * }
         */
        [[nodiscard]] iterator find(const std::string_view key) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(0 < key.size() && key.size() < MPI_MAX_INFO_KEY,
                          "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                          key.size(), MPI_MAX_INFO_KEY);

            const size_type size = this->size();
            return iterator(info_, this->find_pos(key, size));
        }
        /**
         * @brief Finds a [key, value]-pair with key equivalent to @p key.
         * @details If the key is found, returns a const_iterator pointing to the corresponding [key, value]-pair,
         * otherwise the past-the-end const_iterator is returned (see @ref cend()).
         * @param[in] key @p key value of the [key, value]-pair to search for
         * @return const_iterator to a [key, value]-pair with key equivalent to @p key or the past-the-end iterator if no such key is found
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most 'this->size()' times
         * }
         */
        [[nodiscard]] const_iterator find(const std::string_view key) const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(0 < key.size() && key.size() < MPI_MAX_INFO_KEY,
                          "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                          key.size(), MPI_MAX_INFO_KEY);

            const size_type size = this->size();
            return const_iterator(info_, this->find_pos(key, size));
        }
        /**
         * @brief Checks if there is a [key, value]-pair with key equivalent to @p key.
         * @param[in] key @p key value of the [key, value]-pair to search for
         * @return `true` if there is such a [key, value]-pair, otherwise `false`
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most 'this->size()' times
         * }
         */
        [[nodiscard]] bool contains(const std::string_view key) const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(0 < key.size() && key.size() < MPI_MAX_INFO_KEY,
                          "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                          key.size(), MPI_MAX_INFO_KEY);

            const size_type size = this->size();
            return this->find_pos(key, size) != size;
        }
        /**
         * @brief Returns a range containing all [key, value]-pairs with key equivalent to @p key.
         * The range is defined by two iterators, the first pointing to the first [key, value]-pair of the wanted range and the second
         * pointing past the last [key, value]-pair of the range.
         *
         * Since info objects don't allow duplicated keys
         * [`std::distance`](https://en.cppreference.com/w/cpp/iterator/distance)`(it_range.first, it_range.second)`
         * is either 0 (key not found) or 1 (key found).
         *
         * Therefore @ref find(const std::string_view) may be a better choice.
         * @param[in] key @p key value of the [key, value]-pair to search for
         * @return [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) containing a pair of iterators defining the wanted range;\n
         * if there is no such element, past-the-end (see @ref end()) iterators are returned as both elements of the pair
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most 'this->size()' times
         * }
         */
        [[nodiscard]] std::pair<iterator, iterator> equal_range(const std::string_view key) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(0 < key.size() && key.size() < MPI_MAX_INFO_KEY,
                          "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                          key.size(), MPI_MAX_INFO_KEY);

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
         * The range is defined by two const_iterators, the first pointing to the first [key, value]-pair of the wanted range and the
         * second pointing past the last [key, value]-pair of the range.
         *
         * Since info objects don't allow duplicated keys
         * [`std::distance`](https://en.cppreference.com/w/cpp/iterator/distance)`(const_it_range.first, const_it_range.second)`
         * is either 0 (key not found) or 1 (key found).
         *
         * Therefore @ref find(const std::string_view) const may be a better choice.
         * @param[in] key @p key value of the [key, value]-pair to search for
         * @return [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) containing a pair of const_iterators defining the wanted
         * range; \n
         * if there is no such element, past-the-end (see @ref end()) const_iterators are returned as both elements of the pair
         *
         * @pre `*this` **must not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length **must** be greater than 0 and less than *MPI_MAX_INFO_KEY*.
         *
         * @assert{
         * If `*this` is in the moved-from state. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most 'this->size()' times
         * }
         */
        [[nodiscard]] std::pair<const_iterator, const_iterator> equal_range(const std::string_view key) const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");
            MPICXX_ASSERT(0 < key.size() && key.size() < MPI_MAX_INFO_KEY,
                          "Illegal info key: 0 < %u < %i (MPI_MAX_INFO_KEY)",
                          key.size(), MPI_MAX_INFO_KEY);

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


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            non-member functions                                            //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Compares the contents of the two info objects for equality.
         * @details Two info objects compare equal iff they have the same size and their contents compare equal.
         * @param[in] lhs the @p lhs info object to compare
         * @param[in] rhs the @p rhs info object to compare
         * @return `true` if the contents of the info objects are equal, `false` otherwise
         *
         * @pre @p lhs and @p rhs **must not** be in the moved-from state.
         *
         * @assert{ If `lhs` or `rhs` are in the moved-from state. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                       // exactly twice
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                // at most 'lhs.size()' times
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);     // at most '2 * lhs.size()' times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);  // at most '2 * lhs.size()' times
         * }
         */
        [[nodiscard]] friend bool operator==(const info& lhs, const info& rhs) {
            MPICXX_ASSERT(lhs.info_ != MPI_INFO_NULL, "'lhs' is in the moved-from state!");
            MPICXX_ASSERT(rhs.info_ != MPI_INFO_NULL, "'rhs' is in the moved-from state!");

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
                char* lhs_value = new char[valuelen + 1];
                char* rhs_value = new char[valuelen + 1];
                // retrieve values
                MPI_Info_get(lhs.info_, key, valuelen, lhs_value, &flag);
                MPI_Info_get(rhs.info_, key, valuelen, rhs_value, &flag);
                // check if the values are equal
                const bool are_values_equal = std::strcmp(lhs_value, rhs_value) == 0;
                // release buffer
                delete[] lhs_value;
                delete[] rhs_value;
                if (!are_values_equal) {
                    // values compare inequal -> info objects can't cmopare equal
                    return false;
                }
            }

            // all elements are equal
            return true;
        }
        /**
         * @brief Compares the contents of the two info objects for inequality.
         * @details Two info objects compare inequal iff they differ in size or at least one element compares inequal.
         * @param[in] lhs the @p lhs info object to compare
         * @param[in] rhs the @p rhs info object to compare
         * @return `true` if the contents of the info objects are inequal, `false` otherwise
         *
         * @pre @p lhs and @p rhs **must not** be in the moved-from state.
         *
         * @assert{ If `lhs` or `rhs` are in the moved-from state. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                       // exactly twice
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                // at most 'lhs.size()' times
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);     // at most '2 * lhs.size()' times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);  // at most '2 * lhs.size()' times
         * }
         */
        [[nodiscard]] friend bool operator!=(const info& lhs, const info& rhs) {
            MPICXX_ASSERT(lhs.info_ != MPI_INFO_NULL, "'lhs' is in the moved-from state!");
            MPICXX_ASSERT(rhs.info_ != MPI_INFO_NULL, "'rhs' is in the moved-from state!");

            return !(lhs == rhs);
        }
        /**
         * @brief Specializes the [`std::swap`](https://en.cppreference.com/w/cpp/algorithm/swap) algorithm for info objects.
         * Swaps the contents of @p lhs and @p rhs.
         * @details Calls `lhs.swap(rhs)`.
         *
         * Does not invoke any move, copy or swap operations on individual elements.
         * @param[inout] lhs the info object whose contents to swap
         * @param[inout] rhs the info object whose contents to swap
         *
         * @post @p lhs is in a valid state iff @p rhs was in a valid state (and vice versa).
         * @post All iterators remain valid, but now refer to the other info object.
         */
        friend void swap(info& lhs, info& rhs) noexcept { lhs.swap(rhs); }
        /**
         * @brief Erases all [key, value]-pairs that satisfy the predicate @p pred from the info object.
         * @details The [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) only guarantees that the number of a
         * given key does not change **as long as** no call to *MPI_Info_set* or *MPI_Info_delete* is made.
         *
         * Therefore (to be compliant with the standard) this function performs two steps:
         * 1. Save all keys, for which the corresponding [key, value]-pair satisfies the predicate @p pred, in a
         * [`std::vector<std::string>>`](https://en.cppreference.com/w/cpp/container/vector).
         * 2. Delete all [key, value]-pairs in the info object @p c with a key contained in the previously created vector.
         * @tparam Pred a valid predicate which accepts a `value_type` and returns a `bool`
         * @param[inout] c info object from which to erase
         * @param[in] pred predicate that returns `true` if the element should be erased
         *
         * @pre @p c **must not** be in the moved-from state.
         * @post As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) all iterators referring to @p c
         * are invalidated. \n
         * Specific MPI implementations **may** differ in this regard, i.e. iterators before the first point of erase remain valid, all
         * other iterators are invalidated.
         *
         * @assert{ If `c` is in the moved-from state. }
         *
         * @calls{
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                    // exactly 'c.size()' times
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly 'c.size()' times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // exactly 'c.size()' times
         * int MPI_Info_delete(MPI_Info info, const char *key);                                         // at most 'c.size()' times
         * }
         */
        template <typename Pred>
        friend void erase_if(info& c, Pred pred) requires std::is_invocable_r_v<bool, Pred, value_type> {
            MPICXX_ASSERT(c.info_ != MPI_INFO_NULL, "'c' is in the moved-from state!");

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
                value_type key_value_pair = std::make_pair(std::string(key), std::move(value));

                // check whether the predicate holds
                if (pred(key_value_pair)) {
                    // the predicate evaluates to true -> remember key for erasure
                    keys_to_delete.emplace_back(std::move(key_value_pair.first));
                }
            }

            // delete all [key, value]-pairs for which the predicate returns true
            for (const auto& str : keys_to_delete) {
                MPI_Info_delete(c.info_, str.data());
            }
        }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            additional functions                                            //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Returns a [`std::vector`](https://en.cppreference.com/w/cpp/container/vector) containing all keys of the info object.
         * @return all keys of the info object
         *
         * @pre `*this` **must not** be in the moved-from state.
         *
         * @assert{ If `*this` is in the moved-from state. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // exactly 'this->size()' times
         * }
         */
        [[nodiscard]] std::vector<std::string> keys() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            // create vector which will hold all keys
            const size_type size = this->size();
            std::vector<std::string> keys(size);
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
         *
         * @pre `*this` **must not** be in the moved-from state.
         *
         * @assert{ If `*this` is in the moved-from state. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                           // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                    // exactly 'this->size()' times
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly 'this->size()' times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // exactly 'this->size()' times
         * }
         */
        [[nodiscard]] std::vector<std::string> values() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "'*this' is in the moved-from state!");

            // create vector which will hold all values
            const size_type size = this->size();
            std::vector<std::string> values(size);
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
         * @return the maximum key size (= *MPI_MAX_INFO_KEY*)
         */
        [[nodiscard]] static constexpr size_type max_key_size() { return static_cast<size_type>(MPI_MAX_INFO_KEY); }
        /**
         * @brief Returns the maximum possible value size of any [key, value]-pair.
         * @return the maximum value size (= *MPI_MAX_INFO_VAL*)
         */
        [[nodiscard]] static constexpr size_type max_value_size() { return static_cast<size_type>(MPI_MAX_INFO_VAL); }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                   getter                                                   //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Get the underlying *MPI_Info* object.
         * @return the *MPI_Info* object wrapped in this mpicxx::info object
         */
        [[nodiscard]] MPI_Info get() const noexcept { return info_; }
        /**
         * @brief Returns whether the underlying *MPI_Info* object gets automatically freed upon destruction, i.e. the destructor
         * calls *MPI_Info_free*.
         * @return `true` if *MPI_Info_free* gets called upon destruction, `false` otherwise
         */
        [[nodiscard]] bool freeable() const noexcept { return is_freeable_; }


    private:
        /*
         * @brief Finds the position of the given @p key in the info object.
         * @param[in] key the @p key to find
         * @param[in] size the current size of the info object
         * @return the position of the @p key or @p size if the @p key does not exist in this info object
         *
         * @attention No assertions are performed because this function is only callable from within this class and
         * every caller already checks all preconditions.
         *
         * @calls{
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most 'this->size()' times
         * }
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
         * every caller already checks all preconditions.
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);     // exactly once
         * }
         */
        bool key_exists(const std::string_view key) const {
            int valuelen, flag;
            MPI_Info_get_valuelen(info_, key.data(), &valuelen, &flag);
            return static_cast<bool>(flag);
        }

        MPI_Info info_;
        bool is_freeable_;
    };

    // initialize static environment object
    inline const info info::env = info(MPI_INFO_ENV, false);

}

#endif // MPICXX_INFO_HPP
