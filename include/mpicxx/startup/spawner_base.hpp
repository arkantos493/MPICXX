/**
 * @file include/mpicxx/startup/spawner_base.hpp
 * @author Marcel Breyer
 * @date 2020-04-14
 *
 * @brief Implements all common operations used in all spawner classes,
 * that are @ref mpicxx::single_spawner and @ref mpicxx::multiple_spawner.
 */

#ifndef MPICXX_SPAWNER_BASE_HPP
#define MPICXX_SPAWNER_BASE_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <mpi.h>

#include <mpicxx/detail/assert.hpp>


namespace mpicxx {
    // forward declare all spawner classes
    class single_spawner;
    class multiple_spawner;
}

namespace mpicxx::detail {

    // TODO 2020-03-22 19:04 marcel: change from MPI_Comm to mpicxx equivalent
    // TODO 2020-03-23 12:56 marcel: change from int to mpicxx errcode equivalent
    // TODO 2020-03-23 12:56 marcel: change from fmt::format to std::format
    // TODO 2020-03-23 17:37 marcel: copy/move constructor/assignment

    /**
     * @nosubgrouping
     * @brief This class implements all common operations used in all other spawner classes,
     * that are @ref mpicxx::single_spawner and @ref mpicxx::multiple_spawner.
     */
    class spawner_base {
        /// befriend @ref mpicxx::single_spawner
        friend class mpicxx::single_spawner;
        /// befriend @ref mpicxx::multiple_spawner
        friend class mpicxx::multiple_spawner;

        // ---------------------------------------------------------------------------------------------------------- //
        //                                                constructor                                                 //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name constructor
        ///@{
        /**
         * @brief Construct a new spawner_base object.
         * @param[in] maxprocs the total number of spawned processes.
         *
         * @pre @p maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         *
         * @assert_sanity{ If @p maxprocs is invalid. }
         */
        spawner_base(const int maxprocs) {
            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs),
                    "Can't spawn the given number of processes: 0 < {} <= {}", maxprocs, spawner_base::universe_size());

            errcodes_ = std::vector<int>(maxprocs, -1);
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                      getter/setter spawn information                                       //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name modify spawn information
        ///@{
        /**
         * @brief Set the rank of the root process (from which the other processes are spawned).
         * @param[in] root the root process
         *
         * @pre @p root **must not** be less than `0` and greater or equal than the size of the communicator (set via
         * @ref set_communicator(MPI_Comm) or default *MPI_COMM_WORLD*).
         *
         * @assert_precondition{ If @p root isn't a legal root. }
         */
        void set_root(const int root) noexcept {
            MPICXX_ASSERT_PRECONDITION(this->legal_root(root, comm_),
                    "The root can't be used in the provided communicator!: 0 <= {} < {}", root, this->comm_size(comm_));

            root_ = root;
        }
        /**
         * @brief Returns the rank of the root process.
         * @return the root rank (`[[nodiscard]]`)
         */
        [[nodiscard]] int root() const noexcept { return root_; }
        /**
         * @brief Intracommunicator containing the group of spawning processes.
         * @param[in] comm an intracommunicator
         *
         * @pre @p comm **must not** be *MPI_COMM_NULL*.
         * @pre The currently specified rank (as returned by @ref root()) **must be** valid in @p comm.
         *
         * @assert_precondition{ If @p comm is the null communicator (*MPI_COMM_NULL*). }
         * @assert_sanity{ If the currently specified root isn't valid in @p comm. }
         */
        void set_communicator(MPI_Comm comm) noexcept {
            MPICXX_ASSERT_PRECONDITION(this->legal_communicator(comm), "Can't use null communicator!");
            MPICXX_ASSERT_SANITY(this->legal_root(root_, comm),
                    "The previously set root '{}' isn't a valid root in the new communicator!", root_);

            comm_ = comm;
        }
        /**
         * @brief Returns the intracommunicator containing the group of spawning processes.
         * @return the intracommunicator (`[[nodiscard]]`)
         */
        [[nodiscard]] MPI_Comm communicator() const noexcept { return comm_; }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                   information after spawn has been called                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name lookup (after process spawning)
        /// (only meaningful if called after @ref single_spawner::spawn() resp. @ref multiple_spawner::spawn())
        ///@{
        /**
 * @brief Returns the number of spawned processes.
 * @details Two possible behaviours:
 * 1. **hard** spawn: Either `maxprocs` processes are spawned (returning `maxprocs`) or the call to spawn results in an error
 * (returning `0`).
 * 2. **soft** spawn: The info object may specify an arbitrary set \f$\{m_i : 0 \leq m_i \leq maxprocs \}\f$ of allowed values for
 * the number of spawned processes. If one of these allowed numbers of processes \f$ m_i \f$ can be spawned, the call to spawn
 * succeeds (returning \f$ m_i \f$). If it isn't possible to spawn one of the allowed number of processes, the call to spawn results
 * in an error (returning `0`).
 * @return the number of spawned processes (`[[nodiscard]]`)
 *
 * @pre @ref single_spawner::spawn() resp. @ref multiple_spawner::spawn() **must** already have been called.
 *
 * @assert_sanity{ If @ref single_spawner::spawn() resp. @ref multiple_spawner::spawn() hasn't been called yet. }
 *
 * @calls{
 * int MPI_Comm_remote_size(MPI_Comm comm, int *size);      // at most once
 * }
 */
        [[nodiscard]] int number_of_spawned_processes() const {
            MPICXX_ASSERT_SANITY(this->already_spawned(), "Spawn not called, so can't query the number of spawned processes yet!");

            if (intercomm_ != MPI_COMM_NULL) {
                int size;
                MPI_Comm_remote_size(intercomm_, &size);
                return size;
            } else {
                return 0;
            }
        }
        /**
         * @brief Check whether it was possible to spawn `maxprocs` processes.
         * @return `true` if `maxprocs` processes could be spawned, otherwise `false` (`[[nodiscard]]`)
         *
         * @pre @ref single_spawner::spawn() resp. @ref multiple_spawner::spawn() **must** already have been called.
         *
         * @assert_sanity{ If @ref single_spawner::spawn() resp. @ref multiple_spawner::spawn() hasn't been called yet. }
         *
         * @calls{
         * int MPI_Comm_remote_size(MPI_Comm comm, int *size);      // at most once
         * }
         */
        [[nodiscard]] bool maxprocs_processes_spawned() const {
            MPICXX_ASSERT_SANITY(this->already_spawned(),
                                 "Spawn not called, so can't decide whether 'maxprocs' process have been spawned yet!");

            return errcodes_.size() == static_cast<std::size_t>(this->number_of_spawned_processes());
        }
        /**
         * @brief Returns the intercommunicator between the original group and the newly spawned group.
         * @return the intercommunicator (`[[nodiscard]]`)
         *
         * @pre @ref single_spawner::spawn() resp. @ref multiple_spawner::spawn() **must** already have been called.
         *
         * @assert_sanity{ If @ref single_spawner::spawn() resp. @ref multiple_spawner::spawn() hasn't been called yet. }
         */
        [[nodiscard]] MPI_Comm intercommunicator() const noexcept {
            MPICXX_ASSERT_SANITY(this->already_spawned(), "Spawn not called, so no intercommunicator has been created yet!");

            return intercomm_;
        }
        /**
         * @brief Returns the errcodes (one code per process) returned by the spawn call.
         * @return the errcodes (`[[nodiscard]]`)
         *
         * @pre @ref single_spawner::spawn() resp. @ref multiple_spawner::spawn() **must** already have been called.
         *
         * @assert_sanity{ If @ref single_spawner::spawn() resp. @ref multiple_spawner::spawn() hasn't been called yet. }
         */
        [[nodiscard]] const std::vector<int>& errcodes() const noexcept {
            MPICXX_ASSERT_SANITY(this->already_spawned(), "Spawn not called, so no errcodes available yet!");

            return errcodes_;
        }
        /**
         * @brief Prints the number of failed spawns and the respective errcode messages (including how often the errcode occurred).
         * @param[inout] out the output stream on which the errcodes messages should be written
         *
         * @pre @ref single_spawner::spawn() resp. @ref multiple_spawner::spawn() **must** already have been called.
         *
         * @assert_sanity{ If @ref single_spawner::spawn() resp. @ref multiple_spawner::spawn() hasn't been called yet. }
         *
         * @calls{
         * int MPI_Error_string(int errorcode, char *string, int *resultlen);       // at most 'maxprocs' times
         * }
         */
        void print_errors_to(std::ostream& out = std::cout) const {
            MPICXX_ASSERT_SANITY(this->already_spawned(), "Spawn not called, so no errcodes available yet!");

            // immediately exit if spawn hasn't been called yet
            if (std::all_of(errcodes_.cbegin(), errcodes_.cend(), [](const int err) { return err == -1; })) return;

            // count and display the number of errors
            const auto failed_spawns = std::count_if(errcodes_.cbegin(), errcodes_.cend(),
                    [](const int err) { return err != MPI_SUCCESS; });
            out << fmt::format("{} {} occurred!:\n", failed_spawns, failed_spawns == 1 ? "error" : "errors");

            // count how often each error occurred
            std::map<int, int> counts;
            for (const int err : errcodes_) {
                if (err != MPI_SUCCESS && err != -1) {
                    ++counts[err];
                }
            }

            // retrieve the error string and print it
            for (const auto [err, count] : counts) {
                char error_string[MPI_MAX_ERROR_STRING];
                int resultlen;
                MPI_Error_string(err, error_string, &resultlen);
                out << fmt::format("{:>5}x {}\n", count, std::string(error_string, resultlen));
            }
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                           getter for spawn size                                            //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Returns the maximum possible number of processes.
         * @return the maximum possible number of processes (`[[nodiscard]]`)
         *
         * @note It may be possible that less than `universe_size` processes can be spawned if processes are already running.
         *
         * @calls{
         * int MPI_Comm_get_attr(MPI_Comm comm, int comm_keyval, void *attribute_val, int *flag);       // exactly once
         * }
         */
        [[nodiscard]] static int universe_size() {
            void* ptr;
            int flag;
            MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_UNIVERSE_SIZE, &ptr, &flag);
            if (static_cast<bool>(flag)) {
                return *reinterpret_cast<int*>(ptr);
            } else {
                return 0;
            }
        }


#if ASSERTION_LEVEL > 0
        /*
         * @brief Returns the size of @p comm.
         * @param[in] comm an intracommunicator
         * @return the size of @p comm
         */
        int comm_size(const MPI_Comm comm) const {
            int size;
            MPI_Comm_size(comm, &size);
            return size;
        }
        /*
         * @brief Checks whether @p root is valid in @p comm, i.e. @p root is greater and equal than `0` and less than @p comm's size.
         * @param[in] root the root
         * @param[in] comm the communicator
         * @return `true` if @p root is legal, `false` otherwise
         */
        bool legal_root(const int root, const MPI_Comm comm) const {
            return 0 <= root && root < this->comm_size(comm);
        }
        /*
         * @brief Checks whether @p comm is valid, i.e. it does **not** refer to *MPI_COMM_NULL*.
         * @param[in] comm a intercommunicator
         * @return `true` if @p comm is valid, `false` otherwise
         */
        bool legal_communicator(const MPI_Comm comm) const noexcept {
            return comm != MPI_COMM_NULL;
        }
        /*
         * @brief Checks whether @p maxprocs is valid, i.e. @maxprocs is greater than `0` and less or equal than the current universe size.
         * @param[in] maxprocs the number of processes which should be spawned
         * @return `true` if @p maxprocs is legal, `false` otherwise
         */
        bool legal_maxprocs(const int maxprocs) const {
            return 0 < maxprocs && maxprocs <= spawner_base::universe_size();
        }
        /*
         * @brief Checks whether spawn has already been called.
         * @details Checks whether the errcodes are all `-1` (default initialization).
         * @return `true` if spawn has already been called, otherwise `false`
         */
        bool already_spawned() const {
            return std::none_of(errcodes_.cbegin(), errcodes_.cend(), [](const int count) { return count == -1; });
        }
#endif

        int root_ = 0;
        MPI_Comm comm_ = MPI_COMM_WORLD;

        MPI_Comm intercomm_ = MPI_COMM_NULL;
        std::vector<int> errcodes_;
    };

}

namespace mpicxx {

    /**
     * @brief Returns the parent intracommunicator of the current process if the process was started with *MPI_COMM_SPAWN* or
     * *MPI_COMM_SPAWN_MULTIPLE*.
     * @return a [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional) containing the parent intracommunicator or
     * `std::nullopt` (`[[nodiscard]]`)
     */
    [[nodiscard]] inline std::optional<MPI_Comm> parent_process() {
        MPI_Comm intercomm;
        MPI_Comm_get_parent(&intercomm);
        if (intercomm != MPI_COMM_NULL) {
            return std::make_optional(intercomm);
        } else {
            return std::nullopt;
        }
    }

}


#endif // MPICXX_SPAWNER_HPP
