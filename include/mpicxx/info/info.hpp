/**
 * @file info.hpp
 * @author Marcel Breyer
 * @date 2019-11-24
 *
 * @brief Implements a wrapper class around the MPI info object.
 *
 * The @ref mpicxx::info class interface is inspired by the `std::map` interface.
 */

#ifndef MPICXX_INFO_HPP
#define MPICXX_INFO_HPP

#include <cstring>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <mpi.h>

#include <mpicxx/utility/assert.hpp>
#include <mpicxx/utility/string.hpp>

namespace mpicxx {
    /**
     * This class is a wrapper to the *MPI_Info* object providing a interface inspired by
     * <a href="https://en.cppreference.com/w/cpp/container/map">std::map</a>.
     *
     * TODO: usage example
     */
    class info {
        using size_type = std::size_t;
        using value_type = std::pair<const std::string, const std::string>; // TODO 2019-11-24 20:15 marcel: more efficient?
        using iterator = void; // TODO 2019-11-23 22:06 marcel: implement iterator
        using const_iterator = void; // TODO 2019-11-23 22:06 marcel: implement const iterator

        /**
         * @brief This proxy class is used to distinguish between read and write accesses in @ref mpicxx::info::operator[].
         */
        class proxy {
        public:
            // constructors
            template <typename T>
            proxy(info* ptr, T&& key);

            // write access
            void operator=(const std::string& value);
            void operator=(const char* value);

            // read access
            operator std::string() const;

        private:
            info* ptr;
            const std::string key;
        };

    public:
        // constructors and destructor
        info();
        info(std::initializer_list<value_type> ilist); // TODO 2019-11-23 22:19 marcel: implement
        info(const info& other);
        info(info&& other);
        ~info();

        // assignment operators
        info& operator=(const info& rhs);
        info& operator=(info&& rhs);

        // access
        template <typename T>
        proxy at(T&& key);
        template <typename T>
        proxy operator[](T&& key);

        // iterators


        // capacity
        bool empty() const;
        size_type size() const;

        // modifier
//        void clear();
//        std::pair<iterator, bool> insert(const key_type& key);
//        template <typename InputIterator>
//        void insert(InputIterator first, InputIterator last);
//        void insert(std::initializer_list<value_type> ilist);
//        template <typename M>
//        std::pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj); // TODO 2019-11-23 22:17 marcel: alle anschauen

        // lookup
//        template <typename T>
//        size_type count(const T& key) const;
//        template <typename T>
//        iterator find(const T& key);
//        template <typename T>
//        const_iterator find(const T& key) const;
//        template <typename T>
//        bool contains(const T& key) const;

        // getter
        MPI_Info get() noexcept;
        MPI_Info get() const noexcept;

    private:
        MPI_Info info_;
    };


    // ---------------------------------------------------------------------------------------------------------- //
    //                                         constructors and destructor                                        //
    // ---------------------------------------------------------------------------------------------------------- //
    /**
     * @brief Default constructor: create a new empty info object.
     *
     * @post the newly constructed object is in a valid state
     *
     * @calls{ int MPI_Info_create(MPI_Info *info); }
     */
    inline info::info() {
        MPI_Info_create(&info_);
    }
    /**
     * @brief Provides a constructor to initialize the info object directly with multiple [key, value]-pairs.
     * @details If the same key is provided several times, the last usage will be the final value.
     * @param ilist initializer list used to initialize the info object directly with multiple [key, value]-paris
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
     * @pre the length of **any** key (including a null terminator) may **not** be greater then *MPI_MAX_INFO_KEY*
     * @pre the length of **any** value (including a null terminator) may **not** be greater then *MPI_MAX_INFO_VAL*
     * @post the newly constructed object is in a valid state
     *
     * @assert{
     *  if **any** key's length (including a null terminator) is greater then *MPI_MAX_INFO_KEY*\n
     *  if **any** value's length (including a null terminator) is greater then *MPI_MAX_INFO_VAL*
     * }
     *
     * @calls{
     * int MPI_Info_create(MPI_Info *info);
     * int MPI_Info_set(MPI_Info info, const char *key, const char *value);
     * }
     */
    inline info::info(std::initializer_list<value_type> ilist) {
        // create empty info object
        MPI_Info_create(&info_);
        // add all given pairs
        for (const auto& [key, value] : ilist) {
            MPICXX_ASSERT(utility::string_size(key) < MPI_MAX_INFO_KEY,
                          "Info key to long!: max size: %u, provided size (with null terminator): %u",
                          MPI_MAX_INFO_KEY, utility::string_size(key) + 1);
            MPICXX_ASSERT(utility::string_size(value) < MPI_MAX_INFO_VAL,
                          "Info value to long!: max size: %u, provided size (with null terminator): %u",
                          MPI_MAX_INFO_VAL, utility::string_size(value) + 1);
            // add [key, value]-pair
            MPI_Info_set(info_, key.c_str(), value.c_str());
        }
    }
    /**
     * @brief Copy constructor: construct this info object with a copy of the given info object.
     * @details Retains the [key, value]-pair ordering.
     * @param[in] other the copied info object
     *
     * @pre @p other may **not** be in the moved-from state
     * @post the newly constructed object is in a valid state
     *
     * @assert{ if called with a moved-from object }
     *
     * @calls{ int MPI_Info_dup(MPI_info info, MPI_info *newinfo); }
     */
    inline info::info(const info& other) {
        MPICXX_ASSERT(other.info_ != MPI_INFO_NULL, "Copying a \"moved-from\" object is not supported.");
        MPI_Info_dup(other.info_, &info_);
    }
    /**
     * @brief Move constructor: transfer the resources from the given info object to this object.
     * @details Retains the [key, value]-pair ordering.
     * @param[in] other the moved-from info object
     *
     * @post the newly constructed object is in a valid state iff @p other was **not** in the moved-from state\n
     * @post @p other is now in the moved-from state (i.e. `this->get() == MPI_INFO_NULL`)
     */
    inline info::info(info&& other) : info_(std::move(other.info_)) {
        other.info_ = MPI_INFO_NULL;
    }
    /**
     * @brief Destruct this info object.
     * @details If this object is in the moved-from state, no free function is called.
     *
     * @calls{ int MPI_Info_free(MPI_info *info); }
     */
    inline info::~info() {
        if (info_ != MPI_INFO_NULL) {
            MPI_Info_free(&info_);
        }
    }


    // ---------------------------------------------------------------------------------------------------------- //
    //                                            assignment operators                                            //
    // ---------------------------------------------------------------------------------------------------------- //
    /**
     * @brief Copy assignment operator: assign the copy of the given info object to this info object.
     * @details Retains the [key, value]-pair ordering. Gracefully handles self-assignment.
     * @param[in] rhs the copied info object
     * @return the lhs object (being the copy of @p rhs)
     *
     * @pre @p rhs may **not** be in the moved-from state
     * @post the assigned to object is in a valid state
     *
     * @assert{ if called with a moved-from object }
     *
     * @calls{ int MPI_Info_dup(MPI_info info, MPI_info *newinfo); }
     */
    inline info& info::operator=(const info& rhs) {
        MPICXX_ASSERT(rhs.info_ != MPI_INFO_NULL, "Copying a \"moved-from\" object is not supported.");
        // check against self-assignment
        if (this != std::addressof(rhs)) {
            // delete current MPI_Info object if it is in a valid state
            if (info_ != MPI_INFO_NULL) {
                MPI_Info_free(&info_);
            }
            // copy rhs info object
            MPI_Info_dup(rhs.info_, &info_);
        }
        return *this;
    }
    /**
     *
     * @brief Move assignment operator: transfer the resources from the given info object to this object.
     * @details Retains the [key, value]-pair ordering. Does **not** handle self-assignment
     * (as of https://isocpp.org/wiki/faq/assignment-operators).
     * @param[in] rhs the moved-from info object
     *
     * @post the assigned to object is in a valid state iff @p rhs was **not** in the moved-from state\n
     * @post @p rhs is now in the moved-from state (i.e. `this->get() == MPI_INFO_NULL`)
     *
     * @calls{ int MPI_Info_free(MPI_info *info); }
     */
    inline info& info::operator=(info&& rhs) {
        // delete the current MPI_Info object if it is in a valid state
        if (info_ != MPI_INFO_NULL) {
            MPI_Info_free(&info_);
        }
        // transfer ownership
        info_ = std::move(rhs.info_);
        // set moved from object to the moved-from state
        rhs.info_ = MPI_INFO_NULL;
        return *this;
    }


    // ---------------------------------------------------------------------------------------------------------- //
    //                                                   access                                                   //
    // ---------------------------------------------------------------------------------------------------------- //
    /*
     * TODO 2019-11-23 21:40 marcel: ???
     * template <typename T>
       concept String = std::is_same_v<std::decay_t<T>, std::string> ||
                 std::is_same_v<std::decay_t<T>, const char*> ||
                 std::is_same_v<std::decay_t<T>, char*>;
     */
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
    inline info::proxy info::at(T&& key) {
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
        return proxy(this, std::forward<T>(key));
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
    inline info::proxy info::operator[](T&& key) {
        MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling through a \"moved-from\" object is not supported.");
        MPICXX_ASSERT(utility::string_size(key) < MPI_MAX_INFO_KEY,
                      "Info key to long!: max size: %u, provided size (with null terminator): %u",
                      MPI_MAX_INFO_KEY, utility::string_size(key) + 1);

        // create proxy object and forward key
        return proxy(this, std::forward<T>(key));
    }


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
     * @assert{ if called through a moved-from object }
     *
     * @calls{ int MPI_Info_get_nkeys(MPI_Info info, int *nkeys); }
     */
    inline bool info::empty() const {
        MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling through a \"moved-from\" object is not supported.");
        return this->size() == 0;
    }
    /**
     * @brief Returns the number of [key, value]-pairs contained in this info object.
     * @return the number of [key, value]-pairs
     *
     * @pre `this` may **not** be in the moved-from state
     *
     * @assert{ if called through a moved-from object }
     *
     * @calls{ int MPI_Info_get_nkeys(MPI_Info info, int *nkeys); }
     */
    inline info::size_type info::size() const {
        MPICXX_ASSERT(info_ != MPI_INFO_NULL, "Calling through a \"moved-from\" object is not supported.");
        int nkeys;
        MPI_Info_get_nkeys(info_, &nkeys);
        return static_cast<size_type>(nkeys);
    }


    // ---------------------------------------------------------------------------------------------------------- //
    //                                                  modifiers                                                 //
    // ---------------------------------------------------------------------------------------------------------- //
    // TODO 2019-11-23 21:18 marcel: add methods


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
    //                                     proxy class for mpicxx::operator[]                                     //
    // ---------------------------------------------------------------------------------------------------------- //
    /**
     * @brief Constructs a new proxy object.
     * @details Uses perfect forwarding to construct the internal key as `std::string`.
     * @tparam T type of the key
     * @param ptr pointer to the parent info object
     * @param key the provided key
     */
    template <typename T>
    inline info::proxy::proxy(info* ptr, T&& key) : ptr(ptr), key(std::forward<T>(key)) { }
    /**
     * @brief Adds the provided value with the used key to this info object.
     * @details Creates a new [key, value]-pair if key doesn't exist, otherwise overwrites the existing @p value.
     * @param value the value associated with the key
     *
     * @pre the length of the value (including a null terminator) may **not** be greater then *MPI_MAX_INFO_VAL*
     *
     * @assert{ if the value's length (including a null terminator) is greater then *MPI_MAX_INFO_VAL* }
     *
     * @calls{ int MPI_Info_set(MPI_Info info, const char *key, const char *value); }
     */
    inline void info::proxy::operator=(const std::string& value) {
        MPICXX_ASSERT(value.size() < MPI_MAX_INFO_VAL,
                      "Info value to long!: max size: %u, provided size (with null terminator): %u",
                      MPI_MAX_INFO_VAL, value.size() + 1);
        MPI_Info_set(ptr->info_, key.c_str(), value.c_str());
    }
    /**
     * @copydoc info::proxy::operator=(const std::string&)
     */
    inline void info::proxy::operator=(const char* value) {
        MPICXX_ASSERT(std::strlen(value) < MPI_MAX_INFO_VAL,
                      "Info value to long!: max size: %u, provided size (with null terminator): %u",
                      MPI_MAX_INFO_KEY, std::strlen(value) + 1);
        MPI_Info_set(ptr->info_, key.c_str(), value);
    }
    /**
     * @brief Returns the value associated to the provided key.
     * @details If the key doesn't exists yet, it will be inserted with an empty string as value,
     * also returning an empty string.
     * @return the value associated to key
     *
     * @post the info::size() increases iff the requested key did not exist
     * @attention This function returns the associated value *by-value*, i.e. changing the returned value won't alter
     * this object's internal value!
     *
     * @calls{
     * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag)
     * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag)
     * }
     */
    inline info::proxy::operator std::string() const {
        // get the length of the value
        int valuelen = 0, flag;
        MPI_Info_get_valuelen(ptr->info_, key.c_str(), &valuelen, &flag);

        if (flag == 0) {
            // the key doesn't exist yet -> add a new [key, value]-pair
            MPI_Info_set(ptr->info_, key.c_str(), "");
            return std::string();
        }

        // key exists -> get the associated value
        std::string value(valuelen, ' ');
        MPI_Info_get(ptr->info_, key.c_str(), valuelen, value.data(), &flag);
        return value;
    }


}

#endif // MPICXX_INFO_HPP
