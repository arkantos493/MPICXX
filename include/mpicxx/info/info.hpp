/**
 * @file info.hpp
 * @author Marcel Breyer
 * @date 2020-01-22
 *
 * @brief Implements a wrapper class around the MPI info object.
 *
 * The @ref mpicxx::info class interface is inspired by the `std::map` interface.
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

#include <mpicxx/utility/assert.hpp>
#include <mpicxx/utility/concepts.hpp>


namespace mpicxx {
    /**
     * This class is a wrapper to the *MPI_Info* object providing a interface inspired by
     * [`std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map).
     *
     * TODO: usage example
     */
    class info {

        // ---------------------------------------------------------------------------------------------------------- //
        //                                             string proxy class                                             //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief A proxy class for a `std::string` object to distinguish between a read or write access.
         * @details Calls @ref operator std::string() const on a write access and @ref operator=(const std::string_view) on a read access.\n
         * Can be printed directly through an `operator<<` overload.
         */
        class string_proxy {
        public:
            /**
             * @brief Construct a new proxy object.
             * @param[in] ptr pointer to the parent info object
             * @param[in] key the provided key (must meet the requirements of the detail::string concept)
             */
            string_proxy(MPI_Info ptr, detail::string auto&& key) : ptr_(ptr), key_(std::forward<decltype(key)>(key)) { }

            /**
             * @brief On write access, add the provided @p value and saved key to the referred info object.
             * @details Creates a new [key, value]-pair if the key doesn't already exist, otherwise overwrites the existing @p value.
             * @param value the value associated with key
             *
             * @pre @p value **must** include a null-terminator.
             * @pre The @p value's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_VAL*.
             *
             * @assert{ If @p value exceeds its size limit. }
             *
             * @calls{
             * int MPI_Info_set(MPI_Info info, const char *key, const char *value);         // exactly once
             * }
             */
            void operator=(const std::string_view value) {
                MPICXX_ASSERT(value.size() < MPI_MAX_INFO_VAL,
                              "Info value too long!: max. size: %i, provided size (with the null-terminator): %u",
                              MPI_MAX_INFO_VAL, value.size() + 1);

                MPI_Info_set(ptr_, key_.data(), value.data());
            }

            /**
             * @brief On read access return the value associated with the saved key.
             * @details If the key doesn't exist yet, it will be inserted with a string consisting only of one whitespace as value,
             * also returning a `std::string(" ")`.
             * @return the value associated with key
             *
             * @attention This function returns the associated value *by-value*, i.e. changing the returned `std::string` **won't** alter
             * this object's internal value!
             * @attention Because inserting an empty string `""` is not allowed, a `" "` string is inserted instead if the key does not
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
                MPI_Info_get_valuelen(ptr_, key_.data(), &valuelen, &flag);

                if (flag == 0) {
                    // the key doesn't exist yet -> add a new [key, value]-pair and return a `std::string` consisting of only one whitespace
                    MPI_Info_set(ptr_, key_.data(), " ");
                    return std::string(" ");
                }

                // key exists -> get the associated value
                std::string value(valuelen, ' ');
                MPI_Info_get(ptr_, key_.data(), valuelen, value.data(), &flag);
                return value;
            }

            /**
             * @brief Convenient overload to be able to directly print a string_proxy object.
             * @details Calls @ref operator std::string() const to get the value that should be printed.
             * @param out the output stream to write on
             * @param rhs the string_proxy object
             * @return the output stream
             *
             * @calls{
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
             * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                         // at most once
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // at most once
             * }
             */
            friend std::ostream& operator<<(std::ostream& out, const string_proxy& rhs) {
                out << static_cast<std::string>(rhs.operator std::string());
                return out;
            }

        private:
            MPI_Info ptr_;
            const std::string key_;
        };


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  iterators                                                 //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Provides iterator and const_iterator for an info object.
         * @details The standard reverse_iterator and const_reverse_iterator are provided
         * in terms of std::reverse_iterator<iterator> and std::reverse_iterator<const_iterator> respectively.
         * @tparam is_const if `true` a const_iterator is instantiated, otherwise a non-const iterator
         */
        template <bool is_const>
        class info_iterator {
            // needed to be able to construct a const_iterator from a iterator
            template <bool>
            friend class info_iterator;
            // info class can now directly access the pos member
            friend class info;
        public:
            // ---------------------------------------------------------------------------------------------------------- //
            //                                         iterator_traits definitions                                        //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) difference type to identify the
             * distance between iterators.
             */
            using difference_type = int;
            /**
             * @brief [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) value type that can be obtained
             * by dereferencing the iterator.
             * @details In case of a non-const iterator, the value will be returned by a @ref info::string_proxy object to allow changing
             * its value. \n
             * In case of a const iterator, the value will directly be returned as `const std::string` because changing the
             * value is forbidden by definition.
             */
            using value_type = std::conditional_t<is_const,
                                                  std::pair<const std::string, const std::string>,
                                                  std::pair<const std::string, string_proxy>>;
            /**
             * @brief [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) pointer type to the elements
             * iterated over.
             * @details Because it is not possible to simply return value_type* (dangling pointer to a local object),
             * it is necessary to wrap value_type in a `std::unique_ptr`.
             */
            using pointer = std::unique_ptr<value_type>;
            /**
             * @brief [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) reference type
             * (**not** meaningful because operator*() and operator->() has to return **by-value** (using a string_proxy for write access)).
             */
            using reference = value_type;
            /// [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) iterator category.
            using iterator_category = std::random_access_iterator_tag;
            /// [`std::iterator_traits`](https://en.cppreference.com/w/cpp/iterator/iterator_traits) iterator concept (for C++20 concepts)
            using iterator_concept = std::random_access_iterator_tag;


            // ---------------------------------------------------------------------------------------------------------- //
            //                                                constructors                                                //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Construct a new iterator.
             * @param[inout] wrapped_info pointer to the iterated over *MPI_Info* object
             * @param[in] pos the iterator's start position
             */
            info_iterator(MPI_Info wrapped_info, const int pos) : info_(wrapped_info), pos_(pos) { }
            /**
             * @brief Special copy constructor: defined to be able to convert a non-const iterator to a const_iterator.
             * @tparam is_const_iterator
             * @param[in] other the copied iterator
             */
            template <bool is_const_iterator> requires is_const
            info_iterator(const info_iterator<is_const_iterator>& other) : info_(other.info_), pos_(other.pos_) { }


            // ---------------------------------------------------------------------------------------------------------- //
            //                                            assignment operator                                             //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Special copy assignment operator: defined to be able to assign a non-const iterator to a const_iterator.
             * @tparam is_const_iterator
             * @param[in] rhs the copied iterator
             */
            template <bool is_const_iterator>
            info_iterator& operator=(const info_iterator<is_const_iterator>& rhs) requires is_const {
                info_ = rhs.info_;
                pos_ = rhs.pos_;
                return *this;
            }


            // ---------------------------------------------------------------------------------------------------------- //
            //                                            relational operators                                            //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Perform the respective comparison operation on this iterator and the given @p rhs one.
             * @details This iterator and @p rhs iterator may not necessarily have the same constness.
             * @tparam is_rhs_const
             * @param[in] rhs the other iterator
             * @return the comparison result
             *
             * @pre This iterator and @p rhs iterator have to point to the same info object.
             *
             * @assert{ If the two iterators don't point to the same info object. }
             */
            template <bool is_rhs_const>
            bool operator==(const info_iterator<is_rhs_const>& rhs) const {
                MPICXX_ASSERT(info_ == rhs.info_,
                              "The two iterators have to point to the same info object in order to compare them!");

                return info_ == rhs.info_ && pos_ == rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator!=(const info_iterator<is_rhs_const>& rhs) const {
                MPICXX_ASSERT(info_ == rhs.info_,
                              "The two iterators have to point to the same info object in order to compare them!");

                return info_ != rhs.info_ || pos_ != rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator<(const info_iterator<is_rhs_const>& rhs) const {
                MPICXX_ASSERT(info_ == rhs.info_,
                              "The two iterators have to point to the same info object in order to compare them!");

                return info_ == rhs.info_ && pos_ < rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator>(const info_iterator<is_rhs_const>& rhs) const {
                MPICXX_ASSERT(info_ == rhs.info_,
                              "The two iterators have to point to the same info object in order to compare them!");

                return info_ == rhs.info_ && pos_ > rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator<=(const info_iterator<is_rhs_const>& rhs) const {
                MPICXX_ASSERT(info_ == rhs.info_,
                              "The two iterators have to point to the same info object in order to compare them!");

                return info_ == rhs.info_ && pos_ <= rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator>=(const info_iterator<is_rhs_const>& rhs) const {
                MPICXX_ASSERT(info_ == rhs.info_,
                              "The two iterators have to point to the same info object in order to compare them!");

                return info_ == rhs.info_ && pos_ >= rhs.pos_;
            }


            // ---------------------------------------------------------------------------------------------------------- //
            //                                            modifying operations                                            //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Move this iterator one position forward.
             * @return the modified iterator referring to the new position
             */
            info_iterator& operator++() {
                ++pos_;
                return *this;
            }
            /**
             * @brief Move the iterator one position forward and return the old iterator.
             * @return an iterator referring to the old position
             */
            info_iterator operator++(int) {
                info_iterator tmp{*this};
                operator++();
                return tmp;
            }
            /**
             * @brief Move this iterator @p inc steps forward.
             * @param[in] inc number of steps
             * @return the modified iterator referring to the new position
             */
            info_iterator& operator+=(const int inc) {
                pos_ += inc;
                return *this;
            }
            /**
             * @brief Move the iterator @p inc steps forward.
             * @param[in] it copy of the old iterator
             * @param[in] inc number of steps
             * @return the new iterator referring to the new position
             */
            friend info_iterator operator+(info_iterator it, const int inc) {
                it.pos_ += inc;
                return it;
            }
            /**
             * @copydoc info_iterator::operator+(info_iterator, const int)
             */
            friend info_iterator operator+(const int inc, info_iterator it) {
                return it + inc;
            }
            /**
             * @brief Move this iterator one position backward.
             * @return the modified iterator referring to the new position
             */
            info_iterator& operator--() {
                --pos_;
                return *this;
            }
            /**
             * @brief Move the iterator one position backward and return the old iterator.
             * @return an iterator referring to the old position
             */
            info_iterator operator--(int) {
                info_iterator tmp{*this};
                operator--();
                return tmp;
            }
            /**
             * @brief Move this iterator @p inc steps backward.
             * @param[in] inc number of steps
             * @return the modified iterator referring to the new position
             */
            info_iterator& operator-=(const int inc) {
                pos_ -= inc;
                return *this;
            }
            /**
             * @brief Move the iterator @p inc steps backward.
             * @param[in] it copy of the old iterator
             * @param[in] inc number of steps
             * @return the new iterator referring to the new position
             */
            friend info_iterator operator-(info_iterator it, const int inc) {
                it.pos_ -= inc;
                return it;
            }


            // ---------------------------------------------------------------------------------------------------------- //
            //                                            distance calculation                                            //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Calculate the distance between this iterator and the given @p rhs one.
             * @details This iterator and @p rhs iterator may not necessarily have the same constness.
             * @tparam is_rhs_const determines whether the @p rhs iterator is const or not
             * @param rhs the end iterator
             * @return number of elements between this iterator and the @p rhs iterator
             *
             * @pre This iterator and @p rhs iterator have to point to the same info object.
             *
             * @assert{ If the two iterators don't point to the same info object. }
             */
            template <bool is_rhs_const>
            difference_type operator-(const info_iterator<is_rhs_const>& rhs) {
                MPICXX_ASSERT(info_ == rhs.info_,
                              "The two iterators have to point to the same info object in order to calculate the distance between them!");

                return pos_ - rhs.pos_;
            }


            // ---------------------------------------------------------------------------------------------------------- //
            //                                          dereferencing operations                                          //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Get the [key, value]-pair at the current iterator position + @p n.
             * @details If the current iterator is a const_iterator, the returned type is a
             * `std::pair<const std::string, const std::string>`, i.e. everything gets returned **by-value** and can't by changed. \n
             * If the current iterator is a non-const iterator, the returned type is a `std::pair<const std::string, string_proxy>`, i.e.
             * even though the [key, value]-pair gets returned **by-value**, someone can change the value through the string_proxy class.
             * @param[in] n the requested offset of this iterator
             * @return the [key, value]-pair
             *
             * @pre The referred to info object **may not** be in the moved-from state.
             * @pre The position denoted by the current iterator position + @p n **must** be in the half-open
             * interval [0, `wrapped_info_object.size()`).
             *
             * @assert{
             * If called while referring to a moved-from object. \n
             * If trying to access an out-of-bounds position.
             * }
             *
             * @calls_ref{
             * @code
             * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                    // exactly once
             * @endcode
             * const_iterator: \n
             * @code
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // exactly once
             * @endcode
             * iterator: \n
             * for access operations see @ref string_proxy
             * }
             */
            reference operator[](const int n) const {
                // calculate size if in debug mode to assert an out-of-bounds access
#ifndef NDEBUG
                int nkeys;
                MPI_Info_get_nkeys(info_, &nkeys);
#endif
                MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Accessing an element of a \"moved-from\" object is not supported.");
                MPICXX_ASSERT((pos_ + n) >= 0 && (pos_ + n) < nkeys,
                              "Requested an illegal out-of-bounds access! Legal interval: [%i, %u), requested position: %i",
                              0, nkeys, pos_ + n);

                // get the requested key (with an offset of n)
                char key[MPI_MAX_INFO_KEY];
                MPI_Info_get_nthkey(info_, pos_ + n, key);

                if constexpr (is_const) {
                    // this is currently a const_iterator
                    // -> retrieve the value associated to the key

                    // get the length of the value associated with the current key
                    int valuelen, flag;
                    MPI_Info_get_valuelen(info_, key, &valuelen, &flag);

                    // get the value associated with the current key
                    std::string value(valuelen, ' ');
                    MPI_Info_get(info_, key, valuelen, value.data(), &flag);

                    return std::make_pair(std::string(key), std::move(value));
                } else {
                    // this is currently a non-const iterator
                    // -> create a string_proxy object and return that as value in place of a `std::string` to allow changing the value

                    return std::make_pair(std::string(key), string_proxy(info_, key));
                }
            }
            /**
             * @brief Get the [key, value]-pair at the current iterator position.
             * @details If the current iterator is a const_iterator, the returned type is a
             * `std::pair<const std::string, const std::string>`, i.e. everything gets returned **by-value** and can't by changed. \n
             * If the current iterator is a non-const iterator, the returned type is a `std::pair<const std::string, string_proxy>`, i.e.
             * even though the [key, value]-pair gets returned **by-value**, someone can change the value through the string_proxy class.
             * @return the [key, value]-pair
             *
             * @pre The referred to info object **may not** be in the moved-from state.
             * @pre The position denoted by the current iterator position + @p n **must** be in the half-open
             * interval [0, `wrapped_info_object.size()`).
             *
             * @assert{
             * If called while referring to a moved-from object. \n
             * If trying to access an out-of-bounds position.
             * }
             *
             * @calls_ref{
             * @code
             * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                    // exactly once
             * @endcode
             * const_iterator: \n
             * @code
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // exactly once
             * @endcode
             * iterator: \n
             * for access operations see @ref string_proxy
             * }
             */
            reference operator*() const {
                return this->operator[](0);
            }
            /**
             * @copydoc operator*()
             */
            pointer operator->() const {
                return std::make_unique<value_type>(this->operator[](0));
            }


        private:
            MPI_Info info_;
            int pos_;
        };


    public:
        // ---------------------------------------------------------------------------------------------------------- //
        //                                              alias definitions                                             //
        // ---------------------------------------------------------------------------------------------------------- //
        /// The type of the keys of the info object class.
        using key_type = std::string;
        /// The type of the value of the info object class.
        using mapped_type = std::string;
        /// The value type saved in the info object class.
        using value_type = std::pair<const key_type, mapped_type>;
        /// The type used for every function which returns a value associated with an info objects size.
        using size_type = std::size_t;
        /// The type used for the difference calculation of two pointers pointing to info objects.
        using difference_type = std::ptrdiff_t;
        /// The type of value_type used as a reference.
        using reference = value_type&;
        /// The type of value_type used as a const reference.
        using const_reference = const value_type&;
        /// The type of value_type used as a pointer.
        using pointer = value_type*;
        /// The type of value_type used as a const pointer.
        using const_pointer = const value_type*;
        /// Alias for an iterator using the `info_iterator` template class with `is_const` set to `false`.
        using iterator = info_iterator<false>;
        /// Alias for a const_iterator using the `info_iterator` template class with `is_const` set to `true`.
        using const_iterator = info_iterator<true>;
        /// Alias for a reverse_iterator using [`std::reverse_iterator`](https://en.cppreference.com/w/cpp/iterator/reverse_iterator).
        using reverse_iterator = std::reverse_iterator<iterator>;
        /// Alias for a const_reverse_iterator using [`std::reverse_iterator`](https://en.cppreference.com/w/cpp/iterator/reverse_iterator).
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;


        // ---------------------------------------------------------------------------------------------------------- //
        //                                             static data member                                             //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Static member that holds all environment information contained in *MPI_INFO_ENV*.
         * @details **No** *MPI_Info_free* gets called upon destruction.
         */
        static const info env;


        // ---------------------------------------------------------------------------------------------------------- //
        //                                        constructors and destructor                                         //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Constructs an empty info object.
         *
         * @post The newly constructed info object is in a valid state,
         *
         * @calls{
         * int MPI_Info_create(MPI_Info *info);     // exactly once
         * }
         */
        info() : is_freeable_(true) {
            // initialize empty info object
            MPI_Info_create(&info_);
        }
        /**
         * @brief Copy constructor. Constructs this info object with a copy of the contents of @p other.
         * @details Retains @p other's [key, value]-pair ordering.
         * @param[in] other another info object to be used as source to initialize the elements of this info object with
         *
         * @pre @p other **may not** be in the moved-from state.
         * @post The newly constructed info object is in a valid state.
         * @attention Every copied info object is marked **freeable** independent of the **freeable** state of the copied-from info object.
         *
         * @assert{ If called with a moved-from object. }
         *
         * @calls{
         * int MPI_Info_dup(MPI_info info, MPI_info *newinfo);      // exactly once
         * }
         */
        info(const info& other) : is_freeable_(true) {
            MPICXX_ASSERT(other.info_ != MPI_INFO_NULL, "Copying a \"moved-from\" object is not supported.");

            MPI_Info_dup(other.info_, &info_);
        }
        /**
         * @brief Move constructor. Constructs this info object with the contents of @p other using move semantics.
         * @details Retains @p other's [key, value]-pair ordering.
         * @param[in] other another info object to be used as source to initialize the elements of this info object with
         *
         * @post The newly constructed object is in a valid state iff @p other was in a valid state.
         * @post @p other is now in the moved-from state.
         * @post All iterators referring to @p other remain valid, but now refer to `*this`.
         */
        info(info&& other) noexcept : info_(std::move(other.info_)), is_freeable_(std::move(other.is_freeable_)) {
            // set moved from object to the moved-from state
            other.info_ = MPI_INFO_NULL;
            other.is_freeable_ = false;
        }
        /**
         * @brief Constructs this info object with the contents of the range [first, last).
         * @details If multiple elements in the range share the same key, the **last** occurrence determines the value.
         * @tparam InputIt must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator).
         * @param[in] first iterator to the first element in the range
         * @param[in] last iterator one-past the last element in the range
         *
         * Example:
         * @code
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
         * @pre The length of **any** key (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         * @pre The length of **any** value (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_VAL*.
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
            // default construct this info object via the default constructor
            // add all given pairs
            this->insert_or_assign(first, last);
        }
        /**
         * @brief Constructs this info object with the contents of the initializer list @p init.
         * @details If multiple elements in the range share the same key, the **last** occurrence determines the value.
         * @param[in] init initializer list to initialize the elements of this info object with
         *
         * Example:
         * @code
         * mpicxx::info obj = { {"key1", "value1"},
         *                      {"key2", "value2"},
         *                      {"key1", "value1_override"},
         *                      {"key3", "value3"} };
         * @endcode
         * Results in the following [key, value]-pairs stored in the info object (not necessarily in this order):\n
         * `["key1", "value1_override"]`, `["key2", "value2"]` and `["key3", "value3"]`
         *
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         * @pre The length of **any** value (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_VAL*.
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
            // default construct this info object via the default constructor
            // add all given pairs
            this->insert_or_assign(init);
        }
        /**
         * @brief Wrap a *MPI_Info* object in an info object.
         * @param other the *MPI_Info* object
         * @param is_freeable mark whether the *MPI_Info* object wrapped in this info object should be automatically
         * freed at the end of its lifetime
         *
         * @post The newly constructed object is in a valid state iff @p other was in a valid state (i.e. **not** *MPI_INFO_NULL*).
         * @attention If @p is_freeable is set to `false` the user **has** to ensure that the *MPI_Info* object @p other gets properly freed
         * via a call to *MPI_Info_free*.
         */
        constexpr info(MPI_Info other, const bool is_freeable) noexcept : info_(other), is_freeable_(is_freeable) { }
        /**
         * @brief Destructs this info object.
         * @details Only calls *MPI_Info_free* if:
         *      - The object is marked freeable. Only objects created through @ref info(MPI_Info, const bool) can be marked as non-freeable
         *        (or info objects which are moved-from such objects). \n
         *        For example info::env is **non-freeable** due to the fact that the *MPI runtime system* would crash if
         *        *MPI_Info_free* is called with *MPI_INFO_ENV*.
         *      - This object ist **not** in the moved-from state.
         *
         * If any of this conditions is **not** fulfilled, no free function will be called (because doing so is unnecessary and would lead
         * to a crash of the *MPI runtime system*).
         *
         * @post Invalidates **all** iterators referring to `*this`.
         *
         * @calls{
         * int MPI_Info_free(MPI_info *info);       // at most once
         * }
         */
        ~info() {
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
         * @pre @p rhs **may not** be in the moved-from state.
         * @post The assigned to info object is in a valid state.
         * @attention Every copied info object is marked **freeable** independent of the **freeable** state of the copied-from info object.
         *
         * @assert{ If called with a moved-from object. }
         *
         * @calls{
         * int MPI_Info_free(MPI_info *info);                       // at most once
         * int MPI_Info_dup(MPI_info info, MPI_info *newinfo);      // at most once
         * }
         */
        info& operator=(const info& rhs) {
            MPICXX_ASSERT(rhs.info_ != MPI_INFO_NULL, "Copying a \"moved-from\" object is not supported.");

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
         * @post The assigned to object is in a valid state iff @p rhs was in a valid state.
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
            // set moved from object to the moved-from state
            rhs.info_ = MPI_INFO_NULL;
            rhs.is_freeable_ = false;
            return *this;
        }
        /**
         * @brief Replaces the contents with those identified by initializer list @p ilist.
         * @details If multiple elements in the range share the same key, the **last** occurrence determines the value.
         * @param ilist initializer list to initialize the elements of this info object with
         * @return `*this`
         *
         * @pre All @p keys and @p values **must** include the null-terminator.
         * @pre The length of **any** key (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         * @pre The length of **any** value (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_VAL*.
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
            // recreate this info object
            MPI_Info_create(&info_);
            // assign all elements in ilist to this info object
            this->insert_or_assign(ilist);
            return *this;
        }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  iterators                                                 //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Returns an @ref iterator to the first element of this info object.
         * @details If the info object is empty, the returned @ref iterator will be equal to end().
         * @return iterator to the first element
         *
         * @calls_ref{ For additionally called *MPI* functions see the @ref info_iterator documentation. }
         */
        [[nodiscard]] iterator begin() { return iterator(info_, 0); }
        /**
         * @brief Returns an @ref iterator to the element following the last element of this info object.
         * @details This element acts as a placeholder; attempting to access it results in **undefined behavior**.
         * @return @ref iterator to the element following the last element
         *
         * @calls_ref{
         * @code int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys);    // exactly once @endcode
         * For additionally called *MPI* functions see the @ref info_iterator documentation.
         * }
         */
        [[nodiscard]] iterator end() { return iterator(info_, this->size()); }
        /**
         * @brief Returns a @ref const_iterator to the first element of this info object.
         * @details If the info object is empty, the returned @ref const_iterator will be equal to cend().
         * @return @ref const_iterator to the first element
         *
         * @calls_ref{ For additionally called *MPI* functions see the @ref info_iterator documentation. }
         */
        [[nodiscard]] const_iterator begin() const { return const_iterator(info_, 0); }
        /**
         * @brief Returns a @ref const_iterator to the element following the last element of this info object.
         * @details This element acts as a placeholder; attempting to access it results in **undefined behavior**.
         * @return @ref const_iterator to the element following the last element
         *
         * @calls_ref{
         * @code int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys);    // exactly once @endcode
         * For additionally called *MPI* functions see the @ref info_iterator documentation.
         * }
         */
        [[nodiscard]] const_iterator end() const { return const_iterator(info_, this->size()); }
        /**
         * @copydoc begin() const
         */
        [[nodiscard]] const_iterator cbegin() const { return const_iterator(info_, 0); }
        /**
         * @copydoc end() const
         */
        [[nodiscard]] const_iterator cend() const { return const_iterator(info_, this->size()); }

        /**
         * @brief Returns a @ref reverse_iterator to the first element of the reversed info object.
         * @details It corresponds to the last element of the non-reversed info object.
         * If the info object is empty, the returned @ref reverse_iterator will be equal to rend().
         * @return @ref reverse_iterator to the first element
         *
         * @calls_ref{
         * @code int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys);    // exactly once @endcode
         * For additionally called *MPI* functions see the @ref info_iterator documentation.
         * }
         */
        [[nodiscard]] reverse_iterator rbegin() { return std::make_reverse_iterator(this->end()); }
        /**
         * @brief Returns a @ref reverse_iterator to the element following the last element of the reversed info object.
         * @details It corresponds to the element preceding the first element of the non-reversed info object.
         * This element acts as a placeholder, attempting to access it results in **undefined behavior**.
         * @return @ref reverse_iterator to the element following the last element
         *
         * @calls_ref{ For additionally called *MPI* functions see the @ref info_iterator documentation. }
         */
        [[nodiscard]] reverse_iterator rend() { return std::make_reverse_iterator(this->begin()); }
        /**
         * @brief Returns a @ref const_reverse_iterator to the first element of the reversed info object.
         * @details It corresponds to the last element of the non-reversed info object.
         * If the info object is empty, the returned @ref const_reverse_iterator will be equal to crend().
         * @return @ref const_reverse_iterator to the element following the last element
         *
         * @calls_ref{
         * @code int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys);    // exactly once @endcode
         * For additionally called *MPI* functions see the @ref info_iterator documentation.
         * }
         */
        [[nodiscard]] const_reverse_iterator rbegin() const { return std::make_reverse_iterator(this->cend()); }
        /**
         * @brief Returns a @ref const_reverse_iterator to the element following the last element of the reversed info object.
         * @details It corresponds to the element preceding the first element of the non-reversed info object.
         * This element acts as a placeholder, attempting to access it results in **undefined behavior**.
         * @return @ref const_reverse_iterator to the element following the last element
         *
         * @calls_ref{ For additionally called *MPI* functions see the @ref info_iterator documentation. }
         */
        [[nodiscard]] const_reverse_iterator rend() const { return std::make_reverse_iterator(this->cbegin()); }
        /**
         * @copydoc rbegin() const
         */
        [[nodiscard]] const_reverse_iterator crbegin() const { return std::make_reverse_iterator(this->cend()); }
        /**
         * @copydoc rend() const
         */
        [[nodiscard]] const_reverse_iterator crend() const { return std::make_reverse_iterator(this->cbegin()); }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  capacity                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Checks if this info object has no elements, i.e. whether `begin() == end()`.
         * @return `true` if this info object is empty, `false` otherwise
         *
         * @pre `*this` **may not** be in the moved-from state.
         *
         * @assert{ If called with a moved-from object. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);       // exactly once
         * }
         */
        [[nodiscard]] bool empty() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");

            return this->size() == 0;
        }
        /**
         * @brief Returns the number of [key, value]-pairs in this info object, i.e.
         * [`std::distance`](https://en.cppreference.com/w/cpp/iterator/distance)`(begin(), end())`.
         * @return the number of [key, value]-pairs in this info object
         *
         * @pre `*this` **may not** be in the moved-from state.
         *
         * @assert{ If called with a moved-from object. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);       // exactly once
         * }
         */
        [[nodiscard]] size_type size() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");

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
        [[nodiscard]] size_type max_size() const noexcept {
            return std::numeric_limits<difference_type>::max();
        }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  modifier                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Access the value associated with the given @p key including bounds checks.
         * @details Returns a proxy class which is used to distinguish between read and write accesses.
         * @param[in] key the key of the element to find (must meet the requirements of the detail::string concept)
         * @return a proxy object
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p key **must** include a null-terminator.
         * @pre The @p key's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         * @pre The @p key **must** already exist, otherwise an exception will be thrown.
         * @post On write access: all iterators before the insertion point remain valid, all other iterators are invalidated.
         * @attention The proxy returns the associated value *by-value*, i.e. changing the returned value won't alter
         * this object's internal value!
         *
         * @assert{
         * If called with a moved-from object. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @throws std::out_of_range if the info object does not have an element with the specified key
         *
         * @calls_ref{
         * @code
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
         * @endcode
         * On write: \n
         * @code
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                         // exactly once
         * @endcode
         * On read: \n
         * @code
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // exactly once
         * @endcode
         * }
         */
        [[nodiscard]] string_proxy at(detail::string auto&& key) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");
            MPICXX_ASSERT(std::string_view(key).size() < MPI_MAX_INFO_KEY,
                          "Info key too long!: max. size: %i, provided size (with the null-terminator): %u",
                          MPI_MAX_INFO_KEY, std::string_view(key).size() + 1);

            // check whether the key really exists
            if (!this->key_exists(key)) {
                // key doesn't exist
                throw std::out_of_range("The specified key does not exist!");
            }
            // create proxy object and forward key
            return string_proxy(info_, std::forward<decltype(key)>(key));
        }
        /**
         * @brief Access the value associated with the given @p key including bounds checks.
         * @param[in] key the key of the element to find
         * @return the value associated with @p key
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p key **must** include a null-terminator.
         * @pre The @p key's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         * @pre The @p key **must** already exist, otherwise an exception will be thrown.
         * @post On write access: all iterators before the insertion point remain valid, all other iterators are invalidated.
         * @attention This const overload does **not** return a proxy object, but a real `std::string` (by-value)!
         *
         * @assert{
         * If called with a moved-from object. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @throws std::out_of_range if the info object does not have an element with the specified key
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // at most once
         * }
         */
        [[nodiscard]] std::string at(const std::string_view key) const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");
            MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                          "Info key too long!: max. size: %i, provided size (with the null-terminator): %u",
                          MPI_MAX_INFO_KEY, key.size() + 1);

            // get the length of the value associated with key
            int valuelen, flag;
            MPI_Info_get_valuelen(info_, key.data(), &valuelen, &flag);
            // check whether the key really exists
            if (flag == 0) {
                // key doesn't exist
                throw std::out_of_range("The specified key does not exist!");
            }
            // get the value associated with key
            std::string value(valuelen, ' ');
            MPI_Info_get(info_, key.data(), valuelen, value.data(), &flag);
            return value;
        }
        /**
         * @brief Access the value associated with the given @p key.
         * @details Returns a proxy class which is used to distinguish between read and write accesses.
         *
         * `operator[]` is non-const because it inserts the @p key if it doesn't exist. If this behavior is undesirable or if the container
         * is const, `at()` may be used.
         * @param[in] key the key of the element to find (must meet the requirements of the detail::string concept)
         * @return a proxy object
         *
         * Example:
         * @code
         * info_object["key"] = "value";                    // add new [key, value]-pair or override existing value
         * const std::string value = info_object["key"];    // read value associated with key
         * @endcode
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p key **must** include a null-terminator.
         * @pre The @p key's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         * @post On write access: all iterators before the insertion point remain valid, all other iterators are invalidated.
         * @attention The proxy returns the associated value *by-value*, i.e. changing the returned value won't alter
         * this object's internal value!
         *
         * @assert{
         * If called with a moved-from object. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls_ref{
         * On write: \n
         * @code
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                         // exactly once
         * @endcode
         * On read: \n
         * @code
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly once
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                         // at most once
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // at most once
         * @endcode
         * }
         */
        [[nodiscard]] string_proxy operator[](detail::string auto&& key) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");
            MPICXX_ASSERT(std::string_view(key).size() < MPI_MAX_INFO_KEY,
                          "Info key too long!: max. size: %i, provided size (with the null-terminator): %u",
                          MPI_MAX_INFO_KEY, std::string_view(key).size() + 1);

            // create proxy object and forward key
            return string_proxy(info_, std::forward<decltype(key)>(key));
        }

        /**
         * @brief Insert the given [key, value]-pair if the info object doesn't already contain an element with an equivalent key.
         * @param[in] key element @p key to insert
         * @param[in] value element @p value to insert
         * @return a pair consisting of an iterator to the inserted element (or the one element that prevented the insertion) and a `bool`
         * denoting whether the insertion took place
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre **Both** @p key **and** @p value **must** include a null-terminator.
         * @pre The @p key's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         * @pre The @p value's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_VAL*.
         * @post If an assertion took place, all iterators before the insertion point remain valid, all other iterators are invalidated.
         *
         * @assert{
         * If called with a moved-from object. \n
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
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");
            MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                          "Info key to long!: max size: %i, provided size (including the null-terminator): %u",
                          MPI_MAX_INFO_KEY, key.size() + 1);
            MPICXX_ASSERT(value.size() < MPI_MAX_INFO_VAL,
                          "Info value to long!: max size: %i, provided size (including the null-terminator): %u",
                          MPI_MAX_INFO_VAL, value.size() + 1);

            // check if key already exists
            const bool key_already_exists = this->key_exists(key);
            if (!key_already_exists) {
                // key doesn't exist -> add new [key, value]-pair
                MPI_Info_set(info_, key.data(), value.data());
            }
            // search position of the key and return iterator
            return std::make_pair(iterator(info_, this->find_pos(key, this->size())), !key_already_exists);
        }
        /**
         * @brief Inserts elements from range [first, last) if the info object does not already contain an element with an equivalent key.
         * @details If multiple elements in the range have the same key, the **first** occurrence determines the value.
         * @tparam InputIt must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator).
         * @param[in] first iterator to the first element in the range
         * @param[in] last iterator one-past the last element in the range
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid range, i.e. @p first must be less or equal than @p last.
         * @pre All @p keys and @p values **must** include a null-terminator.
         * @pre The length of **any** key (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         * @pre The length of **any** value (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_VAL*.
         * @post If an assertion took place, all iterators before **any** insertion point remain valid, all other iterators are invalidated.
         *
         * @assert{
         * If called with a moved-from object. \n
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
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");
            MPICXX_ASSERT(first <= last, "first must be less or equal than last.");

            // try to insert every element in the range [first, last)
            for (; first != last; ++first) {
                // retrieve element
                const auto& [key, value] = *first;

                MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                              "Info key to long!: max size: %i, provided size (including the null-terminator): %u",
                              MPI_MAX_INFO_KEY, key.size() + 1);
                MPICXX_ASSERT(value.size() < MPI_MAX_INFO_VAL,
                              "Info value to long!: max size: %i, provided size (including the null-terminator): %u",
                              MPI_MAX_INFO_VAL, value.size() + 1);

                // check if key already exists
                if (!this->key_exists(key)) {
                    // key doesn't exist yet -> add new [key, value]-pair
                    MPI_Info_set(info_, key.data(), value.data());
                }
            }
        }
        /**
         * @brief Inserts elements from the initializer list @p ilist if the info object does not already contain an element with
         * an equivalent key.
         * @details If multiple elements in the initializer list have the same key, the **first** occurrence determines the value.
         * @param[in] ilist initializer list to insert the [key, value]-pairs from
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre All @p keys and @p values **must** include a null-terminator.
         * @pre The length of **any** key (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         * @pre The length of **any** value (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_VAL*.
         * @post If an assertion took place, all iterators before the insertion point remain valid, all other iterators are invalidated.
         *
         * @assert{
         * If called with a moved-from object. \n
         * If any key or value exceed their size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                           // exactly 'ilist.size()' times
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);         // at most 'ilist.size()' times
         * }
         */
        void insert(std::initializer_list<value_type> ilist) { this->insert(ilist.begin(), ilist.end()); }

        /**
         * @brief Insert or assign the given [key, value]-pair to this info object.
         * @param[in] key element @p key to insert
         * @param[in] value element @p value to insert
         * @return a pair consisting of an iterator to the inserted or assigned element and a `bool`
         * denoting whether the insertion (`true`) or the assignment (`false`) took place
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre **Both** @p key **and** @p value **must** include a null-terminator.
         * @pre The @p key's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         * @pre The @p value's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_VAL*.
         * @post If an assertion took place, all iterators before the insertion point remain valid, all other iterators are invalidated.
         *
         * @assert{
         * If called with a moved-from object. \n
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
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");
            MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                          "Info key to long!: max size: %u, provided size (including null-terminator): %u",
                          MPI_MAX_INFO_KEY, key.size() + 1);
            MPICXX_ASSERT(value.size() < MPI_MAX_INFO_VAL,
                          "Info value to long!: max size: %u, provided size (including null-terminator): %u",
                          MPI_MAX_INFO_VAL, value.size() + 1);

            // check whether an insertion or assignment will take place
            const bool key_already_exists = this->key_exists(key);
            // updated (i.e. insert or assign) the [key, value]-pair
            MPI_Info_set(info_, key.data(), value.data());
            // search position of the key and return iterator
            return std::make_pair(iterator(info_, this->find_pos(key, this->size())), !key_already_exists);
        }
        /**
         * @brief Inserts or assigns elements from range [first, last) to this info object.
         * @details If multiple elements in the range have the same key, the **last** occurrence determines the value.
         * @tparam InputIt must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator).
         * @param[in] first iterator to the first element in the range
         * @param[in] last iterator one-past the last element in the range
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid range, i.e. @p first must be less or equal than @p last.
         * @pre All @p keys and @p values **must** include a null-terminator.
         * @pre The length of **any** key (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         * @pre The length of **any** value (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_VAL*.
         * @post If an assertion took place, all iterators before **any** insertion point remain valid, all other iterators are invalidated.
         *
         * @assert{
         * If called with a moved-from object. \n
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
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");
            MPICXX_ASSERT(first <= last, "first must be less or equal than last.");

            // insert or assign every element in the range [first, last)
            for (; first != last; ++first) {
                // retrieve element
                const auto [key, value] = *first;

                MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                              "Info key to long!: max size: %u, provided size (including null-terminator): %u",
                              MPI_MAX_INFO_KEY, key.size() + 1);
                MPICXX_ASSERT(value.size() < MPI_MAX_INFO_VAL,
                              "Info value to long!: max size: %u, provided size (including null-terminator): %u",
                              MPI_MAX_INFO_VAL, value.size() + 1);

                // insert or assign [key, value]-pair
                MPI_Info_set(info_, key.data(), value.data());
            }
        }
        /**
         * @brief Inserts or assigns elements from the initializer list @p ilist to this info object.
         * @details If multiple elements in the initializer list have the same key, the **last** occurrence determines the value.
         * @param[in] ilist initializer list to insert or assign the [key, value]-pairs from
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre All @p keys and @p values **must** include a null-terminator.
         * @pre The length of **any** key (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         * @pre The length of **any** value (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_VAL*.
         * @post If an assertion took place, all iterators before the insertion point remain valid, all other iterators are invalidated.
         *
         * @assert{
         * If called with a moved-from object. \n
         * If any key or value exceed their size limit.
         * }
         *
         * @calls{
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);         // exactly 'ilist.size()' times
         * }
         */
        void insert_or_assign(std::initializer_list<value_type> ilist) { this->insert_or_assign(ilist.begin(), ilist.end()); }

        /**
         * @brief Erase all elements from the info object.
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @post The info object is empty, i.e. `this->size() == 0` respectively `this->empty() == true`.
         * @post Invalidates **all** iterators referring to `*this`.
         *
         * @assert{ If called with a moved-from object. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);    // exactly 'this->size()' times
         * int MPI_Info_delete(MPI_Info info, const char *key);         // exactly 'this->size()' times
         * }
         */
        void clear() {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");

            const size_type size = this->size();
            char key[MPI_MAX_INFO_KEY];
            // repeat nkeys times and always remove the first element
            for (size_type i = 0; i < size; ++i) {
                MPI_Info_get_nthkey(info_, 0, key);
                MPI_Info_delete(info_, key);
            }
        }

        /**
         * @brief Removes the element at @p pos.
         * @param[in] pos iterator to the element to remove
         * @return iterator following the last removed element
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p pos **must** refer to `*this` info object.
         * @pre The position denoted by @p pos **must** be in the half-open interval [0, `this->size()`).
         * @post All iterators before the erasure point remain valid, all other iterators are invalidated.
         *
         * @assert{
         * If called with a moved-from object. \n
         * If @p pos does not refer to `*this` info object. \n
         * If trying to dereference an out-of-bounds iterator.
         * }
         *
         * @calls{
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // exactly once
         * int MPI_Info_delete(MPI_Info info, const char *key);             // exactly once
         * }
         */
        iterator erase(const_iterator pos) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");
            MPICXX_ASSERT(pos.info_ == info_, "The given iterator must refer to the same info object as *this.");
            MPICXX_ASSERT(pos.pos_ >= 0 && pos.pos_ < static_cast<int>(this->size()),
                          "Requested an illegal out-of-bounds access! Legal interval: [%i, %u), requested position: %i",
                          0, this->size(), pos.pos_);

            char key[MPI_MAX_INFO_KEY];
            MPI_Info_get_nthkey(info_, pos.pos_, key);
            MPI_Info_delete(info_, key);
            return iterator(info_, pos.pos_);
        }
        /**
         * @brief Removes the elements in the range [first, last).
         * @details [first, last) must be a valid range in `*this`.
         *
         * The [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) only guarantees that the number of a
         * given key does not change **as long as** no call to *MPI_Info_set* or *MPI_Info_delete* is made. Therefore (to be compliant with
         * the standard) at first all keys of the [key, value]-pairs, which should be deleted, are saved in a `std::vector<std::string>>`.
         * In a second step all [key, value]-pairs with a key contained in the vector are deleted.
         * @param[in] first iterator to the first element in the range
         * @param[in] last iterator one-past the last element in the range
         * @return iterator following the last removed element
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p first and @p last **must** refer to `*this` info object
         * @pre The position denoted by @p first **must** be in the half-open interval [0, `this->size()`).
         * @pre The position denoted by @p last **must** be in the half-open interval [0, `this->size()`).
         * @pre @p first **must** be less or equal than @p last.
         *
         * @assert{
         * If called with a moved-from object. \n
         * If @p first or @p last does not refer to `*this` info object. \n
         * If trying to dereference an out-of-bounds iterator.
         * If @p first is greater than @p last.
         * }
         *
         * @calls{
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // exactly 'last - first' times
         * int MPI_Info_delete(MPI_Info info, const char *key);             // exactly 'last - first' times
         * }
         */
        iterator erase(const_iterator first, const_iterator last) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");
            MPICXX_ASSERT(first.info_ == info_, "The given iterator (first) must refer to the same info object as this.");
            MPICXX_ASSERT(last.info_ == info_, "The given iterator (last) must refer to the same info object as this.");
            MPICXX_ASSERT(first.pos_ >= 0 && first.pos_ < static_cast<int>(this->size()),
                          "Requested an illegal out-of-bounds access (first)! Legal interval: [%i, %u), requested position: %i",
                          0, this->size(), first.pos_);
            MPICXX_ASSERT(last.pos_ >= 0 && last.pos_ <= static_cast<int>(this->size()),
                          "Requested an illegal out-of-bounds access (last)! Legal interval: [%i, %u], requested position: %i",
                          0, this->size(), last.pos_);
            MPICXX_ASSERT(first <= last, "first must be less or equal than last.");

            const int count = last - first;
            char key[MPI_MAX_INFO_KEY];
            std::vector<std::string> keys_to_delete(count);

            // delete all [key, value]-pairs in the range [first, last)
            for (int i = 0; i < count; ++i) {
                MPI_Info_get_nthkey(info_, first.pos_ + i, key);
                keys_to_delete[i] = key;
            }

            // delete all requested [key, value]-pairs
            for (const auto& str : keys_to_delete) {
                MPI_Info_delete(info_, str.data());
            }

            return iterator(info_, first.pos_);
        }
        /**
         * @brief Removes the element (if one exists) with the [**key**, value]-pair equivalent to @p key.
         * @details Returns either 1 (key found and removed) or 0 (no such key found and therefore nothing removed).
         * @param[in] key the key to delete
         * @return number of elements removed
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p key **must** include a null-terminator.
         * @pre The @p key's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         * @post If an erasure took place, all iterators before the erasure point remain valid, all other iterators are invalidated.
         *
         * @assert{
         * If called with a moved-from object. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);      // exactly once
         * int MPI_Info_delete(MPI_Info info, const char *key);                                      // at most once
         * }
         */
        size_type erase(const std::string_view key) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");
            MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                          "To be deleted info key to long!: max size: %i, provided size (with null-terminator): %u",
                          MPI_MAX_INFO_KEY, key.size() + 1);

            // check if key really exists
            if (this->key_exists(key)) {
                // key exists -> delete the [key, value]-pair
                MPI_Info_delete(info_, key.data());
                return 1;
            }
            return 0;
        }

        /**
         * @brief Exchanges the contents of this info object with those of @p other.
         * @details Does not invoke any move, copy, or swap operations on individual elements.
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
         * @brief Removes the element at @p pos and returns the removed [key, value]-pair.
         * @param[in] pos iterator to the element to remove
         * @return the extracted [key, value]-pair
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p pos **must** refer to `*this` info object.
         * @pre The position denoted by @p pos **must** be in the half-open interval [0, `this->size()`).
         * @post All iterators before the extraction point remain valid, all other iterators are invalidated.
         *
         * @assert{
         * If called with a moved-from object. \n
         * If @p pos does not refer to `*this` info object. \n
         * If trying to dereference an out-of-bounds iterator.
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
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");
            MPICXX_ASSERT(pos.info_ == info_, "The given iterator must refer to the same info object as this.");
            MPICXX_ASSERT(pos.pos_ >= 0 && pos.pos_ < static_cast<int>(this->size()),
                          "Requested an illegal out-of-bounds access! Legal interval: [%i, %u), requested position: %i",
                          0, this->size(), pos.pos_);

            // get [key, value]-pair pointed to by pos
            value_type key_value_pair = *pos;
            // remove [key, value]-pair from info object
            MPI_Info_delete(info_, key_value_pair.first.data());
            // return extracted [key, value]-pair
            return key_value_pair;
        }
        /**
         * @brief Removes the element (if one exists) with the [**key**, value]-pair equivalent to @p key
         * and returns the removed [key, value]-pair.
         * @details Returns a `std::optional` holding the removed [key, value]-pair if the @p key exists, `std::nullopt` otherwise
         * @param[in] key the key to extract
         * @return the extracted [key, value]-pair
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p key **must** include a null-terminator.
         * @pre The @p key's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         * @post If an extraction took place, all iterators before the extraction point remain valid, all other iterators are invalidated.
         *
         * @assert{
         * If called with a moved-from object. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // at least once, at most twice
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // at most once
         * int MPI_Info_delete(MPI_Info info, const char *key);                                         // at most once
         * }
         */
        [[nodiscard]] std::optional<value_type> extract(const std::string_view key) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");
            MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                          "To be deleted info key to long!: max size: %u, provided size (with null-terminator): %u",
                          MPI_MAX_INFO_KEY, key.size() + 1);

            // check if key really exists
            if (this->key_exists(key)) {
                // key exists -> delete the [key, value]-pair and return it
                // get the value associated with the given key
                int valuelen, flag;
                MPI_Info_get_valuelen(info_, key.data(), &valuelen, &flag);
                std::string value(valuelen, ' ');
                MPI_Info_get(info_, key.data(), valuelen, value.data(), &flag);
                // delete the [key, value]-pair from this info object
                MPI_Info_delete(info_, key.data());
                // return the extracted [key, value]-pair
                return std::make_optional<value_type>(std::make_pair(std::string(key), std::move(value)));
            }
            return std::nullopt;
        }

        /**
         * @brief Attempts to extract each element in @p source and insert it into `*this`.
         * @details If there is an element in `*this` with key equivalent of an element from @p source, than the element is not extracted
         * from @p source. \n
         * Directly returns if a "self-extraction" is attempted.
         *
         * The [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) only guarantees that the number of a
         * given key does not change **as long as** no call to *MPI_Info_set* or *MPI_Info_delete* is made. Therefore (to be compliant with
         * the standard) at first all keys of the [key, value]-pairs, which should be deleted, are saved in a `std::vector<std::string>>`.
         * In a second step all [key, value]-pairs with a key contained in the vector are deleted.
         * @param[inout] source the info object to transfer the [key, value]-pairs from
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p source **may not** be in the moved-from state.
         * @post All iterators before **any** insertion point in `*this` remain valid, all other iterators in `*this` are invalidated.
         * @post All iterators before **any** extraction point in @p other remain valid, all other iterators in @p other are invalidated.
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
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported (this).");
            MPICXX_ASSERT(source.info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported (source).");

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

            // delete all [key, value]-pairs merged into *this info object
            for (const auto& str : keys_to_delete) {
                MPI_Info_delete(source.info_, str.data());
            }
        }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                   lookup                                                   //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Returns the number of elements with [**key**, value]-pair that compares equivalent to the specified @p key.
         * @details Since info objects don't allow duplicated keys the returned value is either 0 (key not found) or 1 (key found).\n
         * Therefore @ref contains(const std::string_view) const may be a better choice.
         * @param[in] key @p key value of the elements to count
         * @return number of elements with [**key**, value]-pair that compares equivalent to @p key, which is either 0 or 1
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         *
         * @assert{
         * If called with a moved-from object. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most 'this->size()' times
         * }
         */
        [[nodiscard]] size_type count(const std::string_view key) const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "*this is in the \"moved-from\" state");
            MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                          "Searched info key too long!: max size: %i, provided size (including the null-terminator): %u",
                          MPI_MAX_INFO_KEY, key.size() + 1);

            return static_cast<size_type>(this->contains(key));
        }
        /**
         * @brief Finds an element with [**key**, value]-pair equivalent to @p key.
         * @details If the key is found, returns an iterator pointing to the corresponding element,
         * otherwise the past-the-end iterator is returned (see @ref end()).
         * @param[in] key @p key value of the element to search for
         * @return iterator to an element with [**key**, value]-pair equivalent to @p key
         * or the past-the-end iterator if no such key is found
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         *
         * @assert{
         * If called with a moved-from object. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most 'this->size()' times
         * }
         */
        [[nodiscard]] iterator find(const std::string_view key) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "*this is in the \"moved-from\" state");
            MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                          "Searched info key too long!: max size: %i, provided size (including the null-terminator): %u",
                          MPI_MAX_INFO_KEY, key.size() + 1);

            const size_type size = this->size();
            return iterator(info_, this->find_pos(key, size));
        }
        /**
         * @brief Finds an element with [**key**, value]-pair equivalent to @p key.
         * @details If the key is found, returns a const_iterator pointing to the corresponding element,
         * otherwise the past-the-end const_iterator is returned (see @ref cend()).
         * @param[in] key @p key value of the element to search for
         * @return const_iterator to an element with [**key**, value]-pair equivalent to @p key
         * or the past-the-end iterator if no such key is found
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         *
         * @assert{
         * If called with a moved-from object. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most 'this->size()' times
         * }
         */
        [[nodiscard]] const_iterator find(const std::string_view key) const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "*this is in the \"moved-from\" state");
            MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                          "Searched info key too long!: max size: %i, provided size (including the null-terminator): %u",
                          MPI_MAX_INFO_KEY, key.size() + 1);

            const size_type size = this->size();
            return const_iterator(info_, this->find_pos(key, size));
        }
        /**
         * @brief Checks if there is an element with [**key**, value]-pair equivalent to @p key in this info object.
         * @param[in] key @p key value of the element to search for
         * @return `true` if there is such an element, otherwise `false`
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         *
         * @assert{
         * If called with a moved-from object. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most 'this->size()' times
         * }
         */
        [[nodiscard]] bool contains(const std::string_view key) const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "*this is in the \"moved-from\" state");
            MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                          "Searched info key too long!: max size: %i, provided size (including the null-terminator): %u",
                          MPI_MAX_INFO_KEY, key.size() + 1);

            const size_type size = this->size();
            return this->find_pos(key, size) != size;
        }
        /**
         * @brief Returns a range containing all elements with [**key**, value]-pair that compare equal to @p key in this info object.
         * The range is defined by two iterators, the first pointing to the first element of the wanted range and the second pointing past
         * the last element of the range. \n
         * Since info objects don't allow duplicated keys
         * [`std::distance`](https://en.cppreference.com/w/cpp/iterator/distance)`(it_range.first, it_range.second)`
         * is either 0 (key not found) or 1 (key found). \n
         * Therefore @ref find(const std::string_view) may be a better choice.
         * @param[in] key @p key value to compare the elements to
         * @return [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) containing a pair of iterators defining the wanted range;
         * if there is no such element, past-the-end (see @ref end()) iterators are returned as both elements of the pair
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         *
         * @assert{
         * If called with a moved-from object. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most 'this->size()' times
         * }
         */
        [[nodiscard]] std::pair<iterator, iterator> equal_range(const std::string_view key) {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "*this is in the \"moved-from\" state");
            MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                          "Searched info key too long!: max size: %i, provided size (including the null-terminator): %u",
                          MPI_MAX_INFO_KEY, key.size() + 1);

            const size_type size = this->size();
            const size_type pos = this->find_pos(key, size);
            if (pos != size) {
                // found key in this info object
                return std::make_pair(iterator(info_, pos), iterator(info_, pos + 1));
            } else {
                // the key is not in this info object
                return std::make_pair(iterator(info_, size), iterator(info_, size));
            }
        }
        /**
         * @brief Returns a range containing all elements with [**key**, value]-pair that compare equal to @p key in this info object.
         * The range is defined by two const_iterators, the first pointing to the first element of the wanted range and the second pointing
         * past the last element of the range. \n
         * Since info objects don't allow duplicated keys
         * [`std::distance`](https://en.cppreference.com/w/cpp/iterator/distance)`(const_it_range.first, const_it_range.second)`
         * is either 0 (key not found) or 1 (key found). \n
         * Therefore @ref find(const std::string_view) const may be a better choice.
         * @param[in] key @p key value to compare the elements to
         * @return [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) containing a pair of const_iterators defining the wanted
         * range; if there is no such element, past-the-end (see @ref end()) const_iterators are returned as both elements of the pair
         *
         * @pre `*this` **may not** be in the moved-from state.
         * @pre @p key **must** include the null-terminator.
         * @pre The @p key's length (including the null-terminator) **may not** be greater than *MPI_MAX_INFO_KEY*.
         *
         * @assert{
         * If called with a moved-from object. \n
         * If @p key exceeds its size limit.
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most 'this->size()' times
         * }
         */
        [[nodiscard]] std::pair<const_iterator, const_iterator> equal_range(const std::string_view key) const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "*this is in the \"moved-from\" state");
            MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                          "Searched info key too long!: max size: %i, provided size (including the null-terminator): %u",
                          MPI_MAX_INFO_KEY, key.size() + 1);

            const size_type size = this->size();
            const size_type pos = this->find_pos(key, size);
            if (pos != size) {
                // found key in this info object
                return std::make_pair(const_iterator(info_, pos), const_iterator(info_, pos + 1));
            } else {
                // the key is not in this info object
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
         * @pre @p lhs and @p rhs **may not** be in the moved-from state.
         *
         * @assert{ If called with a moved-from object. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                       // exactly twice
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                // at most 'lhs.size()' times
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);     // at most '2 * lhs.size()' times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);  // at most '2 * lhs.size()' times
         * }
         */
        [[nodiscard]] friend bool operator==(const info& lhs, const info& rhs) {
            MPICXX_ASSERT(lhs.info_ != MPI_INFO_NULL, "lhs is in the \"moved-from\" state");
            MPICXX_ASSERT(rhs.info_ != MPI_INFO_NULL, "rhs is in the \"moved-from\" state");

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
                    // values compare inequal -> info objects can't be equal
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
         * @pre @p lhs and @p rhs **may not** be in the moved-from state.
         *
         * @assert{ If called with a moved-from object. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                       // exactly twice
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                // at most 'lhs.size()' times
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);     // at most '2 * lhs.size()' times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);  // at most '2 * lhs.size()' times
         * }
         */
        [[nodiscard]] friend bool operator!=(const info& lhs, const info& rhs) {
            MPICXX_ASSERT(lhs.info_ != MPI_INFO_NULL, "lhs is in the \"moved-from\" state");
            MPICXX_ASSERT(rhs.info_ != MPI_INFO_NULL, "rhs is in the \"moved-from\" state");

            return !(lhs == rhs);
        }
        /**
         * @brief Specializes the `std::swap` algorithm for info objects. Swaps the contents of @p lhs and @p rhs.
         * @details Does not invoke any move, copy or swap operations on individual elements.
         * @param[inout] lhs the info object whose contents to swap with @p rhs
         * @param[inout] rhs the info object whose contents to swap with @p lhs
         *
         * @post @p lhs is in a valid state iff @p rhs was in a valid state (and vice versa).
         * @post All iterators remain valid, but now refer to the other info object.
         */
        friend void swap(info& lhs, info& rhs) noexcept { lhs.swap(rhs); }
        /**
         * @brief Erases all elements that satisfy the predicate @p pred from the info object.
         * @details The [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) only guarantees that the number of a
         * given key does not change **as long as** no call to *MPI_Info_set* or *MPI_Info_delete* is made. Therefore (to be compliant with
         * the standard) at first all keys of the [key, value]-pairs, which should be deleted, are saved in a `std::vector<std::string>>`.
         * In a second step all [key, value]-pairs with a key contained in the vector are deleted.
         * @tparam Pred a valid predicate which accepts a `value_type` and returns a `bool`
         * @param[inout] c info object from which to erase
         * @param[in] pred predicate that returns `true` if the element should be erased
         *
         * @pre The info object @p c **may not** be in the moved-from state.
         *
         * @assert{ If called with a moved-from object. }
         *
         * @calls{
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                    // exactly `c.size()` times
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly `c.size()` times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // exactly `c.size()` times
         * int MPI_Info_delete(MPI_Info info, const char *key);                                         // at most `c.size()` times
         * }
         */
        template <typename Pred>
        friend void erase_if(info& c, Pred pred) requires std::is_invocable_r_v<bool, Pred, value_type> {
            MPICXX_ASSERT(c.info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");

            info::size_type size = c.size();
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
                // create [key, value]-pair as std::pair
                info::value_type key_value_pair = std::make_pair(std::string(key), std::move(value));

                // check whether the predicate holds
                if (pred(key_value_pair)) {
                    // the predicate evaluates to true -> remember key for deletion
                    keys_to_delete.emplace_back(std::move(key_value_pair.first));
                }
            }

            // delete all [key, value]-pairs for which pred returns true
            for (const auto& str : keys_to_delete) {
                MPI_Info_delete(c.info_, str.data());
            }
        }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            additional functions                                            //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Returns a `std::vector` containing all keys of this info object.
         * @return all keys of this info object
         *
         * @pre `*this` **may not** be in the moved-from state.
         *
         * @assert{ If called with a moved-from object. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // exactly 'this->size()' times
         * }
         */
        [[nodiscard]] std::vector<std::string> keys() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");

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
         * @brief Returns a `std::vector` containing all values of this info object.
         * @return all values of this info object
         *
         * @pre `*this` **may not** be in the moved-from state.
         *
         * @assert{ If called with a moved-from object. }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                           // exactly once
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                    // exactly 'this->size()' times
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // exactly 'this->size()' times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // exactly 'this->size()' times
         * }
         */
        [[nodiscard]] std::vector<std::string> values() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");

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


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                   getter                                                   //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Get the underlying *MPI_Info* object.
         * @return the *MPI_Info* object wrapped in this info object
         */
        [[nodiscard]] MPI_Info get() const noexcept { return info_; }
        /**
         * @brief Returns whether the underlying *MPI_Info* object gets automatically freed upon construction.
         * @return destructor calls *MPI_Info_free* only if @ref freeable() const noexcept returns `true`
         */
        [[nodiscard]] bool freeable() const noexcept { return is_freeable_; }


    private:
        /*
         * @brief Finds the position of the given @p key in this info object.
         * @param key the @p key to find
         * @param size the current size of this info object
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
         * @brief Tests whether the given @p key already exists in this info object.
         * @param key the @p key to check for
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
