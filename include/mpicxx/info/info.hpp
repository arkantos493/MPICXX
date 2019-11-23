/**
 * @file info.hpp
 * @author Marcel Breyer
 * @date 2019-11-20
 *
 * @brief Implements a wrapper class around the MPI info object.
 *
 * The @ref mpicxx::info class tries to provide a std::map like interface.
 */

#ifndef MPICXX_INFO_HPP
#define MPICXX_INFO_HPP

#include <cstring>
#include <string>
#include <type_traits>

#include <mpi.h>

#include <mpicxx/utility/assert.hpp>

namespace mpicxx {
    /**
     * This class is a wrapper to the *MPI_Info* object providing a <a href="https://en.cppreference.com/w/cpp/container/map">std::map</a> like interface.
     *
     * TODO: usage example
     */
    class info {
        using size_type = std::size_t;

        class proxy {
        public:
            template <typename T>
            proxy(info* ptr, T&& key) : ptr(ptr), key(std::forward<T>(key)) { }

            void operator=(const std::string& value) {
                MPICXX_ASSERT(value.size() < MPI_MAX_INFO_VAL,
                        "Info value to long!: max size: %i, provided size: %i",
                        MPI_MAX_INFO_VAL, value.size());

                MPI_Info_set(ptr->info_, key.c_str(), value.c_str());
            }

            operator std::string() const {
                // int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag)
                // int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag)
                int valuelen = 0, flag;
                MPI_Info_get_valuelen(ptr->info_, key.c_str(), &valuelen, &flag);

                MPICXX_ASSERT(flag, "Key not found!: %s", key);

                std::string value(valuelen, ' ');
                MPI_Info_get(ptr->info_, key.c_str(), valuelen, value.data(), &flag);

                MPICXX_ASSERT(flag, "Key not found!: %s", key);

                return value;
            }
        private:
            info* ptr;
            const std::string key;
        };

    public:
        // constructors and destructor
        info();
        info(const info& other);
        info(info&& other);
        ~info();

        // assignment operators
        info& operator=(const info& rhs);
        info& operator=(info&& rhs);

        /**
         * Returns whether this info object is empty, i.e. has no [key, value]-pairs, or not.
         * @return true iff ``this->size == 0``
         */
        bool empty() const;
        /**
         * Get the number of [key, value]-pairs in this info object
         * @return the number of keys (= number of values)
         */
        size_type size() const;

        /**
         * Get a proxy object to access the value associated with the given key.
         * @tparam T forwarding reference (in essence: ``std::string``, ``const char*`` or ``const char[]``)
         * @param[in] key the accessed key
         * @return a proxy object (to distinguish between read and write accesses)
         */
        template <typename T>
        proxy operator[](T&& key);



        /**
         * Get the underlying *MPI_Info* object.
         * @return the *MPI_Info* wrapped in this info object
         */
        MPI_Info get() noexcept;
        /**
         * Get the underlying *MPI_Info* object (as const).
         * @return the *MPI_Info* wrapped in this info object
         */
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
     * @details Retains the [key, value]-pair ordering. Does **not** handle self-assignment (as of https://isocpp.org/wiki/faq/assignment-operators).
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


    // ------------------------------------------------------------------------ //
    //                                 capacity                                 //
    // ------------------------------------------------------------------------ //
    /**
     * Calls (indirectly):
     * @code
     * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);
     * @endcode
     */
    inline bool info::empty() const {
        return this->size() == 0;
    }
    /**
     * Calls:
     * @code
     * int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);
     * @endcode
     */
    inline info::size_type info::size() const {
        int nkeys;
        MPI_Info_get_nkeys(info_, &nkeys);
        return static_cast<size_type>(nkeys);
    }

    // ------------------------------------------------------------------------ //
    //                                  access                                  //
    // ------------------------------------------------------------------------ //
    /**
     * Write a value:
     * @code
     * info_object["key"] = "value";
     * @endcode
     * Read a value:
     * @code
     * const std::string value = info_object["key"];
     * @endcode
     * @attention
     * The proxy returns the set value *by-value*, i.e. changing the returned value won't alter
     * this object's internal value!
     * @warning
     * **ASSERT**: if the key's length (including a null terminator) is greater then *MPI_MAX_INFO_KEY*
     *
     * Calls:
     * @code
     * int MPI_Info_set(MPI_Info info, const char *key, const char *value)                     // on write
     * int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag)     // on read
     * int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag)  // on read
     * @endcode
     */
    template <typename T>
    inline info::proxy info::operator[](T&& key) {
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
            // use std::string::size() if the given template type decays to a string
            MPICXX_ASSERT(key.size() < MPI_MAX_INFO_KEY,
                          "Info key to long (std::string)!: max size: %i, provided size (with null terminator): %u",
                          MPI_MAX_INFO_KEY, key.size() + 1);
        } else {
            // use the std::strlen() function otherwise (on c-style strings)
            // requires a null terminator!
            MPICXX_ASSERT(std::strlen(key) < MPI_MAX_INFO_KEY,
                          "Info key to long (char*)!: max size: %i, provided size (with null terminator): %u",
                          MPI_MAX_INFO_KEY, std::strlen(key) + 1);
        }
        // create proxy object and forward key
        return proxy(this, std::forward<T>(key));
    }

    // ------------------------------------------------------------------------ //
    //                                modifiers                                 //
    // ------------------------------------------------------------------------ //

    // ------------------------------------------------------------------------ //
    //                                 lookup                                   //
    // ------------------------------------------------------------------------ //


    inline MPI_Info info::get() noexcept { return info_; }
    inline MPI_Info info::get() const noexcept { return info_; }

}

#endif // MPICXX_INFO_HPP
