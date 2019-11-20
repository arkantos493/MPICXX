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

#include <optional>
#include <string>

#include <mpi.h>

#include <mpicxx/utility/assert.hpp>

namespace mpicxx {
    /**
     * This class is a wrapper to the MPI_Info object providing a std::map like interface.
     *
     * TODO: usage example
     */
    class info {
    public:
        /**
         * Default constructor: create a new empty info object.
         */
        info();
        /**
         * Copy constructor: construct this info object with a copy of the given info object.
         * Retains the [key, value]-pair ordering.
         * @param[in] other the copied info object
         */
        info(const info& other);
        /**
         * Delete move construction.
         */
        info(info&&) = delete;
        /**
         * Destruct this info object.
         */
        ~info();

        /**
         * Copy assignment operator: assign the copy of the given info object to this info object.
         * Retains the [key, value]-pair ordering.
         * @param[in] rhs the copied info object
         * @return the newly created info object
         */
        info& operator=(const info& rhs);
        /**
         * Delete move assignment operator.
         */
        info& operator=(info&&) = delete;

        /**
         * Get the underlying MPI_Info object.
         * @return a reference to the MPI_Info wrapped in this info object
         */
        MPI_Info& get() noexcept;
        /**
         * Get the underlying MPI_Info object (as const).
         * @return a const reference to the MPI_Info wrapped in this info object
         */
        const MPI_Info& get() const noexcept;

    private:
        MPI_Info info_;
    };

    // ------------------------------------------------------------------------ //
    //                        constructor and destructor                        //
    // ------------------------------------------------------------------------ //
    /**
     * Calls:
     * @code
     * int MPI_Info_create(MPI_Info *info)
     * @endcode
     */
    info::info() {
        MPI_Info_create(&info_);
    }
    /**
     * Calls:
     * @code
     * int MPI_Info_dup(MPI_info info, MPI_info *newinfo);
     * @endcode
     */
    info::info(const info& other) {
        MPI_Info_dup(other.info_, &info_);
    }
    /**
     * Calls:
     * @code
     * int MPI_Info_free(MPI_info *info);
     * @endcode
     */
    info::~info() {
        MPI_Info_free(&info_);
    }

    // ------------------------------------------------------------------------ //
    //                           assignment operator                            //
    // ------------------------------------------------------------------------ //
    /**
     * Calls:
     * @code
     * int MPI_Info_free(MPI_info *info);
     * int MPI_Info_dup(MPI_info info, MPI_info *newinfo);
     * @endcode
     */
    info& info::operator=(const info& rhs) {
        if (this != &rhs) {
            // delete current MPI_Info object
            MPI_Info_free(&info_);
            // copy rhs info object
            MPI_Info_dup(rhs.info_, &info_);
        }
        return *this;
    }


    MPI_Info& info::get() noexcept { return info_; }
    const MPI_Info& info::get() const noexcept { return info_; }

}

#endif //MPICXX_INFO_HPP
