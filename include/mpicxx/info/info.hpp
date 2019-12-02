/**
 * @file info.hpp
 * @author Marcel Breyer
 * @date 2019-12-02
 *
 * @brief Implements a wrapper class around the MPI info object.
 *
 * The @ref mpicxx::info class interface is inspired by the `std::map` interface.
 */

#ifndef MPICXX_INFO_HPP
#define MPICXX_INFO_HPP

#include <cstring> // TODO 2019-11-30 20:22 marcel: remove later
#include <initializer_list>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <mpi.h>

#include <mpicxx/utility/assert.hpp>
#include <mpicxx/utility/string.hpp> // TODO 2019-11-30 20:23 marcel: remove later

namespace mpicxx {
    /**
     * This class is a wrapper to the *MPI_Info* object providing a interface inspired by
     * <a href="https://en.cppreference.com/w/cpp/container/map">std::map</a>.
     *
     * TODO: usage example
     */
    class info {

        // ---------------------------------------------------------------------------------------------------------- //
        //                                             string proxy class                                             //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief A proxy class for a `std::string` object to distinguish between a read or write access.
         * @details Calls @ref operator std::string() const on a write access and @ref operator=(const std::string&) on a read access.\n
         * Can be printed directly through an `operator<<` overload.
         */
        class string_proxy {
        public:
            /**
             * @brief Constructs a new proxy object.
             * @details Uses perfect forwarding to construct the internal key as `const std::string`.
             * @tparam T type of the key
             * @param ptr pointer to the parent info object
             * @param key the provided key
             */
            template <typename T>
            string_proxy(info* ptr, T&& key) : ptr_(ptr), key_(std::forward<T>(key)) { }

            /**
             * @brief On write access, add the provided @p value and saved key to the info object.
             * @details Creates a new [key, value]-pair if the key doesn't already exist, otherwise overwrites the existing @p value.
             * @param value the value associated with the key
             *
             * @pre the length of the value (including the null-terminator) may **not** be greater then *MPI_MAX_INFO_VAL*
             * @post the info::size() increases iff the requested key did not already exist
             *
             * @assert{ if the value's length (including the null-terminator) is greater then *MPI_MAX_INFO_VAL* }
             *
             * @calls{ int MPI_Info_set(MPI_Info info, const char *key, const char *value); }
             */
            void operator=(const std::string& value) {
                MPICXX_ASSERT(value.size() < MPI_MAX_INFO_VAL,
                              "Info value to long!: max size: %u, provided size (with null-terminator): %u",
                              MPI_MAX_INFO_VAL, value.size() + 1);
                MPI_Info_set(ptr_->info_, key_.data(), value.data());
            }

            /**
             * @brief On read access return the value associated with the key.
             * @details If the key doesn't exists yet, it will be inserted with an empty string as value, also returning an empty string.
             * @return the value associated with key
             *
             * @post the info::size() increases iff the requested key did not already exist
             * @attention This function returns the associated value *by-value*, i.e. changing the returned `std::string` **won't** alter
             * this object's internal value!
             *
             * @calls{
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // always
             * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                         // iff key doesn't exist yet
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // always
             * }
             */
            operator std::string() const {
                // get the length of the value
                int valuelen = 0, flag;
                MPI_Info_get_valuelen(ptr_->info_, key_.data(), &valuelen, &flag);

                if (flag == 0) {
                    // the key doesn't exist yet -> add a new [key, value]-pair and return an empty `std::string`
                    MPI_Info_set(ptr_->info_, key_.data(), "");
                    return std::string();
                }

                // key exists -> get the associated value
                std::string value(valuelen, ' ');
                MPI_Info_get(ptr_->info_, key_.data(), valuelen, value.data(), &flag);
                return value;
            }

            /**
             * @brief Convenient overload to be able to directly print a string_proxy object.
             * @param out the output-stream to write on
             * @param rhs the string_proxy object
             * @return the output-stream
             *
             * @post the info::size() increases iff the requested key did not already exist
             *
             * @calls{
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);         // always
             * int MPI_Info_set(MPI_Info info, const char *key, const char *value);                         // iff key doesn't exist yet
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);      // always
             * }
             */
            friend std::ostream& operator<<(std::ostream& out, const string_proxy& rhs) {
                out << static_cast<std::string>(rhs.operator std::string());
                return out;
            }

        private:
            info* ptr_;
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
         *
         * @attention Any modifying operation on the info object may invalidate the iterators.
         */
        template <bool is_const>
        class info_iterator {
            // needed to be able to construct a const_iterator from a iterator
            template <bool>
            friend class info_iterator;
            // use pointer to const if const_iterator has been requested
            using info_pointer = std::conditional_t<is_const, const info*, info*>;
        public:
            // ---------------------------------------------------------------------------------------------------------- //
            //                                         iterator_traits definitions                                        //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief <a href="https://en.cppreference.com/w/cpp/iterator/iterator_traits"><i>std::iterator_traits</i></a>
             * difference type to identify the distance between iterators
             */
            using difference_type = int;
            /**
             * @brief <a href="https://en.cppreference.com/w/cpp/iterator/iterator_traits"><i>std::iterator_traits</i></a>
             * value type that can be obtained by dereferencing the iterator
             * @details In case of a non-const iterator, the value will be returned by a @ref info::string_proxy object to allow changing
             * its value. \n In case of a const iterator, the value will directly be returned as `const std::string` because changing the
             * value is forbidden by definition.
             */
            using value_type = std::conditional_t<is_const,
                                                  std::pair<const std::string, const std::string>,
                                                  std::pair<const std::string, string_proxy>>;
            /**
             * @brief <a href="https://en.cppreference.com/w/cpp/iterator/iterator_traits"><i>std::iterator_traits</i></a>
             * pointer type to the elements iterated over
             * @details Because it is not possible to simply return value_type* (dangling pointer to a local object),
             * it is necessary to wrap value_type in a `std::unique_ptr`.
             */
            using pointer = std::unique_ptr<value_type>;
            /**
             * @brief <a href="https://en.cppreference.com/w/cpp/iterator/iterator_traits"><i>std::iterator_traits</i></a>
             * reference type (**not** meaningful because operator*() has to return **by-value** (using a string_proxy for write access))
             */
            using reference = value_type;
            /**
             * @brief <a href="https://en.cppreference.com/w/cpp/iterator/iterator_traits"><i>std::iterator_traits</i></a>
             * iterator category
             */
            using iterator_category = std::random_access_iterator_tag;
            /**
             * @brief <a href="https://en.cppreference.com/w/cpp/iterator/iterator_traits"><i>std::iterator_traits</i></a>
             * iterator concept (for C++20 concepts)
             */
            using iterator_concept = std::random_access_iterator_tag;


            // ---------------------------------------------------------------------------------------------------------- //
            //                                                constructors                                                //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Construct a new iterator.
             * @param ptr pointer to the iterated over info object
             * @param pos the iterator's start position
             */
            info_iterator(info_pointer ptr, const int pos) : ptr_(ptr), pos_(pos) { }
            /**
             * @brief Copy constructor: defined to be able to convert a non-const iterator to a const_iterator.
             * @details For const_iterator the default generated copy constructor is used.
             * @tparam is_const_iterator
             * @param other the copied iterator
             */
            template <bool is_const_iterator>
            requires is_const
            info_iterator(const info_iterator<is_const_iterator>& other) : ptr_(other.ptr_), pos_(other.pos_) { }


            // ---------------------------------------------------------------------------------------------------------- //
            //                                            relational operators                                            //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Perform the respective comparison operation on this iterator and the given @p rhs one.
             * @details this iterator and @p rhs iterator may not necessarily have the same constness.
             * @tparam is_rhs_const
             * @param rhs the other iterator
             * @return the comparison result
             *
             * @pre this iterator and @p rhs iterator have to point to the same info object
             *
             * @assert{ if the two iterators don't point to the same info object }
             */
            template <bool is_rhs_const>
            bool operator==(const info_iterator<is_rhs_const>& rhs) const {
                MPICXX_ASSERT(ptr_ == rhs.ptr_, "The two iterators have to point to the same info object in order to compare them!");
                return ptr_ == rhs.ptr_ && pos_ == rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator!=(const info_iterator<is_rhs_const>& rhs) const {
                MPICXX_ASSERT(ptr_ == rhs.ptr_, "The two iterators have to point to the same info object in order to compare them!");
                return ptr_ != rhs.ptr_ || pos_ != rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator<(const info_iterator<is_rhs_const>& rhs) const {
                MPICXX_ASSERT(ptr_ == rhs.ptr_, "The two iterators have to point to the same info object in order to compare them!");
                return ptr_ == rhs.ptr_ && pos_ < rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator>(const info_iterator<is_rhs_const>& rhs) const {
                MPICXX_ASSERT(ptr_ == rhs.ptr_, "The two iterators have to point to the same info object in order to compare them!");
                return ptr_ == rhs.ptr_ && pos_ > rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator<=(const info_iterator<is_rhs_const>& rhs) const {
                MPICXX_ASSERT(ptr_ == rhs.ptr_, "The two iterators have to point to the same info object in order to compare them!");
                return ptr_ == rhs.ptr_ && pos_ <= rhs.pos_;
            }
            /**
             * @copydoc operator==()
             */
            template <bool is_rhs_const>
            bool operator>=(const info_iterator<is_rhs_const>& rhs) const {
                MPICXX_ASSERT(ptr_ == rhs.ptr_, "The two iterators have to point to the same info object in order to compare them!");
                return ptr_ == rhs.ptr_ && pos_ >= rhs.pos_;
            }


            // ---------------------------------------------------------------------------------------------------------- //
            //                                            modifying operations                                            //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Move this iterator one position forward.
             * @return the modified iterator
             */
            info_iterator& operator++() {
                ++pos_;
                return *this;
            }
            /**
             * @brief Move the iterator one position forward (postfix).
             * @return a new iterator referring to the new position
             */
            info_iterator operator++(int) {
                info_iterator tmp{*this};
                operator++();
                return tmp;
            }
            /**
             *@brief Move this iterator @p inc steps forward.
             * @param[in] inc number of steps
             * @return the modified iterator
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
             * @return the modified iterator
             */
            info_iterator& operator--() {
                --pos_;
                return *this;
            }
            /**
             * @brief Move the iterator one position backward (postfix).
             * @return a new iterator referring to the new position
             */
            info_iterator operator--(int) {
                info_iterator tmp{*this};
                operator--();
                return tmp;
            }
            /**
             * @brief Move this iterator @p inc steps backward.
             * @param[in] inc number of steps
             * @return the modified iterator
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
             * @details this iterator and @p rhs iterator may not necessarily have the same constness.
             * @tparam is_rhs_const determines whether the @p rhs iterator is const or not
             * @param rhs the end iterator
             * @return number of elements between this iterator and the @p rhs iterator
             *
             * @pre this iterator and @p rhs iterator have to point to the same info object
             *
             * @assert{ if the two iterators don't point to the same info object }
             */
            template <bool is_rhs_const>
            difference_type operator-(const info_iterator<is_rhs_const>& rhs) {
                MPICXX_ASSERT(ptr_ == rhs.ptr_,
                              "The two iterators have to point to the same info object in order to calculate the distance between them!");
                return pos_ - rhs.pos_;
            }


            // ---------------------------------------------------------------------------------------------------------- //
            //                                          dereferencing operations                                          //
            // ---------------------------------------------------------------------------------------------------------- //
            /**
             * @brief Get the [key, value]-pair at the current iterator position + @p n.
             * @details If the current iterator is a const_iterator, the returned type is a
             * `std::pair<const std::string, const std::string>`, i.e. everything gets returned **by-value** and can't by changed.\n
             * If the current iterator is a non-const iterator, the returned type is a `std::pair<const std::string, string_proxy>`, i.e.
             * even though the [key, value]-pair gets returned **by-value**, someone can change the value through the string_proxy class.
             * @param[in] n the requested offset of this iterator
             * @return the [key, value]-pair
             *
             * @pre the current position + @p n may **not** be less than 0
             * @pre the current position + @p n may **not** be greater or equal than `info::size()`
             * @pre the pointed to info object may **not** be in the moved-from state
             *
             * @assert{
             * if dereferencing an out-of-bounds iterator\n
             * if the pointed to info object is in the moved-from state
             * }
             *
             * @calls{
             * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                // always directly
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);     // const: directly, non-const: on read access
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);  // const: directly, non-const: on read access
             * }
             */
            value_type operator[](const int n) const {
                MPICXX_ASSERT(ptr_->info_ != MPI_INFO_NULL, "Accessing an element of a \"moved-from\" object is not supported.");
                MPICXX_ASSERT((pos_ + n) >= 0 && (pos_ + n) < static_cast<int>(ptr_->size()),
                              "Requested an illegal out-of-bounds access! Legal interval: [%i, %u), requested position: %i",
                              0, ptr_->size(), pos_ + n);

                // get the requested key (with an offset of n)
                char key_arr[MPI_MAX_INFO_KEY];
                MPI_Info_get_nthkey(ptr_->info_, pos_ + n, key_arr);
                const std::string key(key_arr);

                if constexpr (is_const) {
                    // this is currently a const_iterator
                    // -> retrieve the value associated to the key

                    // get the length of the value associated with the current key
                    int valuelen, flag;
                    MPI_Info_get_valuelen(ptr_->info_, key.data(), &valuelen, &flag);

                    // get the value associated with the current key
                    std::string value(valuelen, ' ');
                    MPI_Info_get(ptr_->info_, key.data(), valuelen, value.data(), &flag);

                    return std::make_pair(std::move(key), std::move(value));
                } else {
                    // this is currently a non-const iterator
                    // -> create a string_proxy object and return that as value in place of a `std::string` to allow changing the value

                    string_proxy proxy(ptr_, key);
                    return std::make_pair(std::move(key), std::move(proxy));
                }
            }
            /**
             * @brief Get the [key, value]-pair at the current iterator position.
             * @details If the current iterator is a const_iterator, the returned type is a
             * `std::pair<const std::string, const std::string>`, i.e. everything gets returned **by-value** and can't by changed.\n
             * If the current iterator is a non-const iterator, the returned type is a `std::pair<const std::string, string_proxy>`, i.e.
             * even though the [key, value]-pair gets returned **by-value**, someone can change the value through the string_proxy class.
             * @return the [key, value]-pair
             *
             * @pre the current position + @p n may **not** be less than 0
             * @pre the current position may **not** be greater or equal than `info::size()`
             * @pre the pointed to info object may **not** be in the moved-from state
             *
             * @assert{
             * if dereferencing an out-of-bounds iterator\n
             * if the pointed to info object is in the moved-from state
             * }
             *
             * @calls{
             * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                // always directly
             * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);     // const: directly, non-const: on read access
             * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);  // const: directly, non-const: on read access
             * }
             */
            value_type operator*() const {
                return this->operator[](0);
            }
            /**
             * @copydoc operator*()
             */
            pointer operator->() const {
                return std::make_unique<value_type>(this->operator[](0));
            }

        private:
            info_pointer ptr_;
            int pos_;
        };


    public:
        // ---------------------------------------------------------------------------------------------------------- //
        //                                              alias definitions                                             //
        // ---------------------------------------------------------------------------------------------------------- //
        using size_type = std::size_t;
        using value_type = std::pair<const std::string, std::string>;
        /**
         * @brief Alias for an iterator using the `info_iterator` template class with `is_const` set to `false`.
         */
        using iterator = info_iterator<false>;
        /**
         * @brief Alias for a const_iterator using the `info_iterator` template class with `is_const` set to `true`.
         */
        using const_iterator = info_iterator<true>;
        /**
         * @brief Alias for the reverse_iterator using `std::reverse_iterator`.
         */
        using reverse_iterator = std::reverse_iterator<iterator>;
        /**
         * @brief Alias for the const_reverse_iterator using `std::reverse_iterator`.
         */
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        /**
         * @brief Static member that holds all environment information contained in *MPI_INFO_ENV*.
         * @details **No** *MPI_Info_free* gets called upon destruction.
         */
        static const info env;

        // ---------------------------------------------------------------------------------------------------------- //
        //                                        constructors and destructor                                         //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Default constructor: create a new empty info object.
         *
         * @post the newly constructed object is in a valid state
         *
         * @calls{ int MPI_Info_create(MPI_Info *info); }
         */
        info() : is_freeable_(true) {
            // create empty info object
            MPI_Info_create(&info_);
        }
        /**
         * @brief Copy constructor: construct this info object with a copy of the given info object.
         * @details Retains @p other's [key, value]-pair ordering.
         * @param[in] other the copied info object
         *
         * @pre @p other may **not** be in the moved-from state
         * @post the newly constructed object is in a valid state
         * @attention Every copied info object is automatically marked freeable even if the copied-from is not.
         *
         * @assert{ if called with a moved-from object }
         *
         * @calls{ int MPI_Info_dup(MPI_info info, MPI_info *newinfo); }
         */
        info(const info& other) : is_freeable_(true) {
            MPICXX_ASSERT(other.info_ != MPI_INFO_NULL, "Copying a \"moved-from\" object is not supported.");
            MPI_Info_dup(other.info_, &info_);
        }
        /**
         * @brief Move constructor: transfer the resources from the given info object to this object.
         * @details Retains @p other's [key, value]-pair ordering.
         * @param[in] other the moved-from info object
         *
         * @post the newly constructed object is in a valid state iff @p other was in a valid state\n
         * @post @p other is now in the moved-from state
         * @post all iterators pointing to @p other are invalidated
         */
        constexpr info(info&& other) noexcept : info_(std::move(other.info_)), is_freeable_(std::move(other.is_freeable_)) {
            // other should stay in a operable state
            other.info_ = MPI_INFO_NULL;
            other.is_freeable_ = false;
        }
        /**
         * @brief Iterator-Range constructor: construct a new info object by adding all [key, value]-pairs denoted by
         * `[first, last)`.
         * @details If the same key is added multiple times, its last occurrence determines the final associated value.
         * @tparam InputIter any iterator fulfilling the
         * <a href="https://en.cppreference.com/w/cpp/iterator"><i>InputIterator</i></a> requirements
         * @param[in] first iterator to the first element of the range
         * @param[in] last one of the end iterator of the range
         *
         * Example:
         * @code
         * std::vector<std::pair<std::string, std::string>> key_value_pairs;
         * key_value_pairs.emplace_back("key1", "value1");
         * key_value_pairs.emplace_back("key2", "value2");
         * key_value_pairs.emplace_back("key1", "value1_override");
         * key_value_pairs.emplace_back("key3", "value3");
         *
         * mpicxx::info obj(key_value_pairs.begin(), key_value_pairs.end());
         * @endcode
         * Results in the following [key, value]-pairs stored in the info object (not necessarily in this order):\n
         * `["key1", "value1_override"]`, `["key2", "value2"]` and `["key3", "value3"]`
         *
         * @pre the length of **any** key (including the null-terminator) may **not** be greater then *MPI_MAX_INFO_KEY*
         * @pre the length of **any** value (including the null-terminator) may **not** be greater then *MPI_MAX_INFO_VAL*
         * @post the newly constructed object is in a valid state
         *
         * @assert{
         *  if **any** key's length (including the null-terminator) is greater then *MPI_MAX_INFO_KEY*\n
         *  if **any** value's length (including the null-terminator) is greater then *MPI_MAX_INFO_VAL*
         * }
         *
         * @calls{
         * int MPI_Info_create(MPI_Info *info);
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);         // 'last - first' times
         * }
         */
        template <std::input_iterator InputIter>
        info(InputIter first, InputIter last) : info() {
            // default construct this info object
            // add all given pairs
            this->insert_or_assign(first, last);
        }
        /**
         * @brief Provides a constructor to initialize the info object directly with multiple [key, value]-pairs.
         * @details If the same key is added multiple times, its last occurrence determines the final associated value.
         * @param[in] ilist initializer list used to initialize the info object directly with multiple [key, value]-pairs
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
         * @pre the length of **any** key (including the null-terminator) may **not** be greater then *MPI_MAX_INFO_KEY*
         * @pre the length of **any** value (including the null-terminator) may **not** be greater then *MPI_MAX_INFO_VAL*
         * @post the newly constructed object is in a valid state
         *
         * @assert{
         *  if **any** key's length (including the null-terminator) is greater then *MPI_MAX_INFO_KEY*\n
         *  if **any** value's length (including the null-terminator) is greater then *MPI_MAX_INFO_VAL*
         * }
         *
         * @calls{
         * int MPI_Info_create(MPI_Info *info);
         * int MPI_Info_set(MPI_Info info, const char *key, const char *value);         // ilist.size() times
         * }
         */
        info(std::initializer_list<value_type> ilist) : info() {
            // default construct this info object
            // add all given pairs
            this->insert_or_assign(std::move(ilist));
        }
        /**
         * @brief Wrap a *MPI_Info* object in a info object.
         * @param other the *MPI_Info* object
         * @param is_freeable mark whether the *MPI_Info* object wrapped in this info object should be freed at the end of its lifetime
         *
         * @post the newly constructed object is in a valid state iff @p other was in a valid state (i.e. **not** *MPI_INFO_NULL*)
         * @attention If @p is_freeable is set to `false` the user **has** to ensure that the *MPI_Info* object @p other gets properly freed
         */
        constexpr info(MPI_Info other, const bool is_freeable) noexcept : info_(other), is_freeable_(is_freeable) { }
        /**
         * @brief Destruct this info object.
         * @details Only calls *MPI_Info_free* if:
         *      - The object is marked freeable. Only objects created through @ref info(MPI_Info, const bool) can be marked as non-freeable
         *        (or info objects which are copies-of/moved-from such objects).\n
         *        For example info::env is **non-freeable** due to the fact that the MPI runtime system would crash if
         *        *MPI_Info_free* is called with *MPI_INFO_ENV*.
         *      - This object ist **not** in the moved-from state.
         *
         * If any of this conditions is **not** fulfilled, no free function will be called (because doing so is unnecessary and would lead
         * to a crash of the MPI runtime system).
         *
         * @calls{ int MPI_Info_free(MPI_info *info);       // iff both stated conditions are satisfied }
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
         * @brief Copy assignment operator: assign the copy of the given info object to this info object.
         * @details Retains @p rhs's [key, value]-pair ordering. Gracefully handles self-assignment.
         * @param[in] rhs the copied info object
         * @return the lhs object (being the copy of @p rhs)
         *
         * @pre @p rhs may **not** be in the moved-from state
         * @post the assigned to object is in a valid state
         * @attention Every copied info object is automatically marked freeable even if the copied-from is not.
         *
         * @assert{ if called with a moved-from object }
         *
         * @calls{
         * int MPI_Info_free(MPI_info *info);                       // iff no self-assignment and normal destruction would occur
         * int MPI_Info_dup(MPI_info info, MPI_info *newinfo);      // iff no self-assignment
         * }
         */
        info& operator=(const info& rhs) {
            MPICXX_ASSERT(rhs.info_ != MPI_INFO_NULL, "Copying a \"moved-from\" object is not supported.");
            // check against self-assignment
            if (this != std::addressof(rhs)) {
                // delete current MPI_Info object if it is in a valid state
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
         * @brief Move assignment operator: transfer the resources from the given info object to this object.
         * @details Retains @p rhs's [key, value]-pair ordering. Does **not** handle self-assignment
         * (as of https://isocpp.org/wiki/faq/assignment-operators).
         * @param[in] rhs the moved-from info object
         *
         * @post the assigned to object is in a valid state iff @p rhs was in a valid state\n
         * @post @p rhs is now in the moved-from state
         * @post all iterators pointing to @p rhs are invalidated
         *
         * @calls{ int MPI_Info_free(MPI_info *info);       // iff normal destruction would occur }
         */
        info& operator=(info&& rhs) {
            // delete the current MPI_Info object if it is freeable and in a valid state
            if (is_freeable_ && info_ != MPI_INFO_NULL) {
                MPI_Info_free(&info_);
            }
            // transfer ownership
            info_ = std::move(rhs.info_);
            is_freeable_ = std::move(rhs.is_freeable_);
            // set moved from object to the moved-from state
            rhs.info_ = MPI_INFO_NULL;
            is_freeable_ = false;
            return *this;
        }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                               element access                                               //
        // ---------------------------------------------------------------------------------------------------------- //
        template <typename T>
        string_proxy at(T&& key);
        template <typename T>
        string_proxy operator[](T&& key);
        // TODO 2019-12-02 14:30 marcel: update


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  iterators                                                 //
        // ---------------------------------------------------------------------------------------------------------- //
        // TODO 2019-12-02 19:39 marcel: pass this or info_ ??? -> iterator invalidation...
        /**
         * @brief Returns an iterator to the first element of this info object.
         * @details If the info object is empty this iterator equals info::end().
         * @return the iterator
         *
         * @calls_ref{ for dereferencing operations see @ref info_iterator }
         */
        iterator begin() { return iterator(this, 0); }
        /**
         * @brief Returns an iterator to the element one after the last element of this info object.
         * @details Attempts to access this element, i.e. dereferencing the returned iterator, results in undefined behaviour.
         * @return the iterator
         *
         * @calls_ref{
         * @code int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys); @endcode
         * for dereferencing operations see @ref info_iterator
         * }
         */
        iterator end() { return iterator(this, this->size()); }
        /**
         * @brief Returns a const_iterator to the first element of this info object.
         * @details If the info object is empty this const_iterator equals info::end().
         * @return the const_iterator
         *
         * @calls_ref{ for dereferencing operations see @ref info_iterator }
         */
        const_iterator begin() const { return const_iterator(this, 0); }
        /**
         * @brief Returns a const_iterator to the element one after the last element of this info object.
         * @details Attempts to access this element, i.e. dereferencing the returned const_iterator, results in undefined behaviour.
         * @return the const_iterator
         *
         * @calls_ref{
         * @code int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys); @endcode
         * for dereferencing operations see @ref info_iterator
         * }
         */
        const_iterator end() const { return const_iterator(this, this->size()); }
        /**
         * @copydoc begin() const
         */
        const_iterator cbegin() const { return const_iterator(this, 0); }
        /**
         * @copydoc end() const
         */
        const_iterator cend() const { return const_iterator(this, this->size()); }
        /**
         * @brief Returns a reverse_iterator to the last element of this info object.
         * @details If the info object is empty this reverse_iterator equals info::rend().
         * @return the reverse_iterator
         *
         * @calls_ref{
         * @code int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys); @endcode
         * for dereferencing operations see @ref info_iterator
         * }
         */
        reverse_iterator rbegin() { return std::make_reverse_iterator(this->end()); }
        /**
         * @brief Returns a reverse_iterator to the element one before the first element of this info object.
         * @details Attempts to access this element, i.e. dereferencing the returned reverse_iterator, results in undefined behaviour.
         * @return the reverse_iterator
         *
         * @calls_ref{ for dereferencing operations see @ref info_iterator }
         */
        reverse_iterator rend() { return std::make_reverse_iterator(this->begin()); }
        /**
         * @brief Returns a const_reverse_iterator to the last element of this info object.
         * @details If the info object is empty this const_reverse_iterator equals info::rend().
         * @return the const_reverse_iterator
         *
         * @calls_ref{
         * @code int MPI_Info_get_nkeys(MPI_Info *info, int *nkeys); @endcode
         * for dereferencing operations see @ref info_iterator
         * }
         */
        const_reverse_iterator rbegin() const { return std::make_reverse_iterator(this->cend()); }
        /**
         * @brief Returns a const_reverse_iterator to the element one before the first element of this info object.
         * @details Attempts to access this element, i.e. dereferencing the returned const_reverse_iterator, results in undefined behaviour.
         * @return the const_reverse_iterator
         *
         * @calls_ref{ for dereferencing operations see @ref info_iterator }
         */
        const_reverse_iterator rend() const { return std::make_reverse_iterator(this->cbegin()); }
        /**
         * @copydoc rbegin() const
         */
        const_reverse_iterator crbegin() const { return std::make_reverse_iterator(this->cend()); }
        /**
         * @copydoc rend() const
         */
        const_reverse_iterator crend() const { return std::make_reverse_iterator(this->cbegin()); }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  capacity                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Returns whether this info object is empty or not.
         * @details An info object is empty iff it has no [key, value]-pairs.
         * @return `true` iff `this->size() == 0`
         *
         * @pre `this` may **not** be in the moved-from state
         *
         * @assert{ if called with a moved-from object }
         *
         * @calls{ int MPI_Info_get_nkeys(MPI_Info info, int *nkeys); }
         */
        bool empty() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");
            return this->size() == 0;
        }
        /**
         * @brief Returns the number of [key, value]-pairs contained in this info object.
         * @return the number of [key, value]-pairs
         *
         * @pre `this` may **not** be in the moved-from state
         *
         * @assert{ if called with a moved-from object }
         *
         * @calls{ int MPI_Info_get_nkeys(MPI_Info info, int *nkeys); }
         */
        size_type size() const {
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");
            int nkeys;
            MPI_Info_get_nkeys(info_, &nkeys);
            return static_cast<size_type>(nkeys);
        }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  modifier                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        void insert_or_assign(const std::string& key, const std::string& value);
        template <std::input_iterator It>
        void insert_or_assign(It first, It last);
        void insert_or_assign(std::initializer_list<value_type> ilist);
        /**
         * @brief Swaps the contents of this info object with @p other.
         * @details Does not invoke any move, copy or swap operations on individual elements.\n
         * Invalidates all iterators.
         * @param other the @p other info object
         *
         * @post `this` is in a valid state iff @p other was in a valid state (and vice versa)
         */
        void swap(info& other) noexcept {
            using std::swap;
            swap(info_, other.info_);
            swap(is_freeable_, other.is_freeable_);
        }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                   lookup                                                   //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Search in this info object for number of occurrences of the given @p key.
         * @details Because an info object doesn't support duplicated keys the returned value is either 0 (key not found) or 1 (key found).
         * @param key the searched key
         * @return the number of found keys equal to @p key (0 or 1)
         *
         * @pre the length of the searched key (including the null-terminator) may **not** be greater then *MPI_MAX_INFO_KEY*
         * @pre `this` may **not** be in the moved-from state
         *
         * @assert{
         * if the key's length (including the null-terminator) is greater then *MPI_MAX_INFO_KEY*\n
         * if called with a moved-from object
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // 2 times
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most `this->size()` times
         * }
         */
        size_type count(const std::string& key) const {
            return static_cast<size_type>(this->contains(key));
        }
        /**
         * @brief Search in this info object for the given @p key.
         * @details If the key is found, returns an iterator pointing to the corresponding element,
         * otherwise the past-the-end iterator is returned (see end()).
         * @param key the searched key
         * @return the iterator
         *
         * @pre the length of the searched key (including the null-terminator) may **not** be greater then *MPI_MAX_INFO_KEY*
         * @pre `this` may **not** be in the moved-from state
         *
         * @assert{
         * if the key's length (including the null-terminator) is greater then *MPI_MAX_INFO_KEY*\n
         * if called with a moved-from object
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most `this->size()` times
         * }
         */
        iterator find(const std::string& key) {
            return iterator(this, this->find_pos(key));
        }
        /**
         * @brief Search in this info object for the given @p key.
         * @details If the key is found, returns a const_iterator pointing to the corresponding element,
         * otherwise the past-the-end const_iterator is returned (see cend()).
         * @param key the searched key
         * @return the const_iterator
         *
         * @pre the length of the searched key (including the null-terminator) may **not** be greater then *MPI_MAX_INFO_KEY*
         * @pre `this` may **not** be in the moved-from state
         *
         * @assert{
         * if the key's length (including the null-terminator) is greater then *MPI_MAX_INFO_KEY*\n
         * if called with a moved-from object
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most `this->size()` times
         * }
         */
        const_iterator find(const std::string& key) const {
            return const_iterator(this, this->find_pos(key));
        }
        /**
         * @brief Search in this info object whether the given @p key exists.
         * @param key the searched key
         * @return `true` iff the searched key exists, otherwise `false`
         *
         * @pre the length of the searched key (including the null-terminator) may **not** be greater then *MPI_MAX_INFO_KEY*
         * @pre `this` may **not** be in the moved-from state
         *
         * @assert{
         * if the key's length (including the null-terminator) is greater then *MPI_MAX_INFO_KEY*\n
         * if called with a moved-from object
         * }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);               // 2 times
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);        // at most `this->size()` times
         * }
         */
        bool contains(const std::string& key) const {
            return this->find_pos(key) != this->size();
        }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            non-member functions                                            //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Compares two info objects for equality.
         * @details Two info objects are equal iff their contents are equal.
         * @param lhs the @p lhs info object
         * @param rhs the @p rhs info object
         * @return `true` if the two info objects are equal, `false` otherwise
         *
         * @pre @p lhs and @p rhs may **not** be in the moved-from state
         *
         * @assert{ if called with a moved-from object }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                       // 2-3 times
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                // at most 2 * `lhs.size()` times
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);     // at most 2 * `lhs.size()` times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);  // at most 2 * `lhs.size()` times
         * }
         */
        friend bool operator==(const info& lhs, const info& rhs) {
            MPICXX_ASSERT(lhs.info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported (lhs).");
            MPICXX_ASSERT(rhs.info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported (rhs).");

            // not the same number of [key, value]-pairs therefore can't compare equal
            if (lhs.size() != rhs.size()) { return false; }

            // check all [key, value]-pairs for equality
            const_iterator lhs_it = lhs.cbegin();
            const_iterator rhs_it = rhs.cbegin();
            for (const_iterator end = lhs.cend(); lhs_it != end; ++lhs_it, ++rhs_it) {
                // both info elements don't compare equal
                if (*lhs_it != *rhs_it) {
                    return false;
                }
            }
            // all elements are equal
            return true;
        }
        /**
         * @brief Compares two info objects for inequality.
         * @details Two info objects are inequal if they have different number of elements or at least one element compares inequal.
         * @param lhs the @p lhs info object
         * @param rhs the @p rhs info object
         * @return `true` if the two info objects are inequal, `false` otherwise
         *
         * @pre @p lhs and @p rhs may **not** be in the moved-from state
         *
         * @assert{ if called with a moved-from object }
         *
         * @calls{
         * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);                                       // 2-3 times
         * int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);                                // at most 2 * `lhs.size()` times
         * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);     // at most 2 * `lhs.size()` times
         * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);  // at most 2 * `lhs.size()` times
         * }
         */
        friend bool operator!=(const info& lhs, const info& rhs) { return !(lhs == rhs); }
        /**
         * @brief Swaps the contents of both info objects.
         * @details Does not invoke any move, copy or swap operations on individual elements.\n
         * Invalidates all iterators.
         * @param lhs the @p lhs info object
         * @param rhs the @p rhs info object
         *
         * @post @p lhs is in a valid state iff @p rhs was in a valid state (and vice versa)
         */
        friend void swap(info& lhs, info& rhs) noexcept { lhs.swap(rhs); }


        // getter
        MPI_Info get() noexcept;
        MPI_Info get() const noexcept;

    private:

        size_type find_pos(const std::string& key) const {
            MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                          "Searched info key to long!: max size: %u, provided size (with null-terminator): %u",
                          MPI_MAX_INFO_KEY, key.size() + 1);
            MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling with a \"moved-from\" object is not supported.");

            char info_key[MPI_MAX_INFO_KEY];
            const size_type nkeys = this->size();
            // loop until a matching key is found
            for (size_type i = 0; i < nkeys; ++i) {
                MPI_Info_get_nthkey(info_, i, info_key);
                // found equal key -> return position
                if (key.compare(info_key) == 0) {
                    return i;
                }
            }
            // no matching key found
            return nkeys;
        }


        bool key_exists(const std::string& key) {
            int valuelen, flag;
            MPI_Info_get_valuelen(info_, key.data(), &valuelen, &flag);
            return static_cast<bool>(flag);
        }

        MPI_Info info_;
        bool is_freeable_;
    };

    // initialize static environment object
    inline const info info::env = info(MPI_INFO_ENV, false);



    // ---------------------------------------------------------------------------------------------------------- //
    //                                                   access                                                   //
    // ---------------------------------------------------------------------------------------------------------- //
    /**
     * @brief Access the value associated with the given @p key including bounds checks.
     * @details Returns a proxy class which is used to distinguish between read and write accesses.\n
     * Queries the @p key's value length to determine if the key exists.
     * @tparam T forwarding reference (in essence: ``std::string``, ``const char*`` or ``const char[]``)
     * @param[in] key the accessed key
     * @return a proxy object

     * @pre `this` may **not** be in the moved-from state
     * @pre if not called with a `std::string` @p key **must** contain a null terminator
     * @pre the length of the key (including a null terminator) may **not** be greater then *MPI_MAX_INFO_KEY*
     * @pre the @p key **must** already exist
     * @attention The proxy returns the associated value *by-value*, i.e. changing the returned value won't alter
     * this object's internal value!
     *
     * @assert{
     * if called through a moved-from object\n
     * if the key's length (including a null terminator) is greater then *MPI_MAX_INFO_KEY*
     * }
     *
     * @throws std::out_of_range if @p key doesn't already exist
     *
     * @calls{
     * MPI_Info_get_valuelen(info_, key.c_str(), &valuelen, &flag);                            // for bounds checking
     * int MPI_Info_set(MPI_Info info, const char *key, const char *value)                     // on write
     * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag)     // on read
     * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag)  // on read
     * }
     */
    template <typename T>
    inline info::string_proxy info::at(T&& key) {
        MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling through a \"moved-from\" object is not supported.");
        MPICXX_ASSERT(utility::string_size(key) < MPI_MAX_INFO_KEY,
                      "Info key to long!: max size: %u, provided size (with null terminator): %u",
                      MPI_MAX_INFO_KEY, utility::string_size(key) + 1);

        // query the value length associated to key to determine if the key exists
        int valuelen, flag;
        MPI_Info_get_valuelen(info_, key.c_str(), &valuelen, &flag);
        if (flag == 0) {
            // key doesn't exist
            throw std::out_of_range("The specified key doesn't exist!");
        }
        // create proxy object and forward key
        return string_proxy(this, std::forward<T>(key));
    }
    /**
     * @brief Access the value associated with the given @p key.
     * @details Returns a proxy class which is used to distinguish between read and write accesses.
     * @tparam T forwarding reference (in essence: ``std::string``, ``const char*`` or ``const char[]``)
     * @param[in] key the accessed key
     * @return a proxy object
     *
     * Example:
     * @code
     * info_object["key"] = "value";                    // write value
     * const std::string value = info_object["key"];    // read value
     * @endcode
     *
     * @pre `this` may **not** be in the moved-from state
     * @pre if not called with a `std::string` @p key **must** contain a null terminator
     * @pre the length of the key (including a null terminator) may **not** be greater then *MPI_MAX_INFO_KEY*
     * @attention The proxy returns the associated value *by-value*, i.e. changing the returned value won't alter
     * this object's internal value!
     *
     * @assert{
     * if called through a moved-from object\n
     * if the key's length (including a null terminator) is greater then *MPI_MAX_INFO_KEY*
     * }
     *
     * @calls{
     * int MPI_Info_set(MPI_Info info, const char *key, const char *value)                     // on write
     * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag)     // on read
     * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag)  // on read
     * }
     */
    template <typename T>
    inline info::string_proxy info::operator[](T&& key) {
        MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling through a \"moved-from\" object is not supported.");
        MPICXX_ASSERT(utility::string_size(key) < MPI_MAX_INFO_KEY,
                      "Info key to long!: max size: %u, provided size (with null terminator): %u",
                      MPI_MAX_INFO_KEY, utility::string_size(key) + 1);

        // create proxy object and forward key
        return string_proxy(this, std::forward<T>(key));
    }


    // ---------------------------------------------------------------------------------------------------------- //
    //                                                  modifiers                                                 //
    // ---------------------------------------------------------------------------------------------------------- //
    // TODO 2019-11-23 21:18 marcel: add methods
    inline void info::insert_or_assign(const std::string& key, const std::string& value) {
        MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                      "Info key to long!: max size: %u, provided size (including null-terminator): %u",
                      MPI_MAX_INFO_KEY, key.size() + 1);
        MPICXX_ASSERT(value.size() < MPI_MAX_INFO_VAL,
                      "Info value to long!: max size: %u, provided size (including null-terminator): %u",
                      MPI_MAX_INFO_VAL, value.size() + 1);

        MPI_Info_set(info_, key.data(), value.data());
    }
    template <std::input_iterator It>
    inline void info::insert_or_assign(It first, It last) {
        for (; first != last; ++first) {
            this->insert_or_assign(first->first, first->second);
        }
    }
    inline void info::insert_or_assign(std::initializer_list<value_type> ilist) {
        this->insert_or_assign(ilist.begin(), ilist.end());
    }


    // ---------------------------------------------------------------------------------------------------------- //
    //                                                   lookup                                                   //
    // ---------------------------------------------------------------------------------------------------------- //
    // TODO 2019-11-23 21:18 marcel: add methods


    // ---------------------------------------------------------------------------------------------------------- //
    //                                                   getter                                                   //
    // ---------------------------------------------------------------------------------------------------------- //
    /**
     * @brief Get the underlying *MPI_Info*.
     * @return the *MPI_Info* wrapped in this info object
     */
    inline MPI_Info info::get() noexcept { return info_; }
    /**
     * @brief Get the underlying *MPI_Info* (as const).
     * @return the *MPI_Info* wrapped in this info object
     */
    inline MPI_Info info::get() const noexcept { return info_; }




    // ---------------------------------------------------------------------------------------------------------- //
    //                                               iterator class                                               //
    // ---------------------------------------------------------------------------------------------------------- //

}

#endif // MPICXX_INFO_HPP
