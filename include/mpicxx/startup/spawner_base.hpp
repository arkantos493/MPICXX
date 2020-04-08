/**
 * @file include/mpicxx/startup/spawner_base.hpp
 * @author Marcel Breyer
 * @date 2020-04-08
 *
 * @brief Implements wrapper around the MPI spawn functions.
 */

#ifndef MPICXX_SPAWNER_BASE_HPP
#define MPICXX_SPAWNER_BASE_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <mpi.h>

#include <mpicxx/detail/concepts.hpp>
#include <mpicxx/detail/conversion.hpp>
#include <mpicxx/info/info.hpp>
#include <mpicxx/detail/assert.hpp>

#include <mpicxx/startup/spawner.hpp>
#include <mpicxx/startup/multi_spawner.hpp>

namespace mpicxx {

    // TODO 2020-03-22 19:04 marcel: change from MPI_Comm to mpicxx equivalent
    // TODO 2020-03-23 12:56 marcel: errcode equivalent
    // TODO 2020-03-23 12:56 marcel: change from fmt::format to std::format
    // TODO 2020-03-23 17:37 marcel: copy/move constructor/assignment

    class single_spawner;
    class multiple_spawner;

    /**
     * @brief Spawner class which enables to spawn MPI processes at runtime.
     */
    template <typename Strategy>
    class spawner {
    private:

        using command_return_type = decltype(std::declval<Strategy>().command());
        static constexpr bool is_command_noexcept = noexcept(std::declval<Strategy>().command());

        using maxprocs_return_type = decltype(std::declval<Strategy>().maxprocs());
        static constexpr bool is_maxprocs_noexcept = noexcept(std::declval<Strategy>().maxprocs());

        using spawn_info_return_type = decltype(std::declval<Strategy>().spawn_info());
        static constexpr bool is_spawn_info_noexcept = noexcept(std::declval<Strategy>().spawn_info());


    public:

        /**
         * @brief Create a new spawner.
         * @param[in] command name of program to be spawned
         * @param[in] maxprocs maximum number of processes to start
         *
         * @pre @p command **must not** be empty.
         * @pre @p maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         *
         * @assert_sanity{
         * If @p command is empty. \n
         * If @p maxprocs is invalid.
         * }
         */
        spawner(detail::string auto&& command, const int maxprocs)
                : strategy_(std::forward<decltype(command)>(command), maxprocs)
        {
            MPICXX_ASSERT_SANITY(!strategy_.command().empty(), "No executable name given!");
            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs),
                                 "Can't spawn the given number of processes: 0 < {} <= {}.", maxprocs, spawner::universe_size());

            errcodes_ = std::vector<int>(strategy_.maxprocs(), -1);
        }
        template <typename... Args>
        requires ( (detail::string<typename Args::first_type> && std::is_same_v<std::decay_t<typename Args::second_type>, int>) && ...)
        spawner(Args&&... args) : strategy_(std::forward<Args>(args)...) {
            errcodes_ = std::vector<int>(std::reduce(strategy_.maxprocs().cbegin(), strategy_.maxprocs().cend()));
        }

        /**
 * @brief Returns the name of the executable which should get spawned.
 * @return the executable name
 */
        [[nodiscard]] command_return_type command() const noexcept(is_command_noexcept) { return strategy_.command(); }
        [[nodiscard]] const std::string& command(const std::size_t i) const noexcept(noexcept(std::declval<multiple_spawner>().command(0))) {
            static_assert(std::is_same_v<Strategy, multiple_spawner>, "Can't call member function with a single spawner!");
            return strategy_.command(i);
        }

        /**
         * @brief Returns the number of processes which should get spawned.
         * @return the number of processes
         */
        [[nodiscard]] maxprocs_return_type maxprocs() const noexcept(is_maxprocs_noexcept) { return strategy_.maxprocs(); }
        [[nodiscard]] int maxprocs(const std::size_t i) const noexcept(noexcept(std::declval<multiple_spawner>().maxprocs(0))) {
            static_assert(std::is_same_v<Strategy, multiple_spawner>, "Can't call member function with a single spawner!");
            return strategy_.maxprocs(i);
        }

        /**
         * @brief Returns the number of spawned processes.
         * @details Two possible behaviours:
         * 1. **hard** spawn: Either `maxprocs` processes are spawned (returning `maxprocs`) or the call to spawn results in an error
         * (returning `0`).
         * 2. **soft** spawn: The info object may specify an arbitrary set \f$\{m_i : 0 \leq m_i \leq maxprocs \}\f$ of allowed values for
         * the number of spawned processes. If one of these allowed numbers of processes \f$ m_i \f$ can be spawned, the call to spawn
         * succeeds (returning \f$ m_i \f$). If it isn't possible to spawn one of the allowed number of processes, the call to spawn results
         * in an error (returning `0`).
         * @return the number of spawned processes
         *
         * @pre @ref spawn() **must** already have been called.
         *
         * @assert_sanity{ If @ref spawn() hasn't been called yet. }
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
         * @return `true` if `maxprocs` processes could be spawned, otherwise `false`
         *
         * @pre @ref spawn() **must** already have been called.
         *
         * @assert_sanity{ If @ref spawn() hasn't been called yet. }
         */
        [[nodiscard]] bool maxprocs_processes_spanwed() const {
            MPICXX_ASSERT_SANITY(this->already_spawned(),
                    "Spawn not called, so can't decide whether 'maxprocs' process have been spawned yet!");

            return errcodes_.size() == static_cast<std::size_t>(this->number_of_spawned_processes());
        }
        /**
         * @brief Returns the maximum possible number of processes.
         * @return the maximum possible number of processes.
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

        /**
         * @brief Set the info object representing additional information for the runtime system where and how to spawn the processes.
         * @details As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) reserved keys are:
         *
         *  key | description
         * :----| :--------------------------------------------------------------------------------------------------------------------------------------------------|
         * host | a hostname                                                                                                                                         |
         * arch | an architecture name                                                                                                                               |
         * wdir | a name of a directory on a machine on which the spawned processes execute; this directory is made the working directory of the executing processes |
         * path | a directory or set of directories where the MPI implementation should look for the executable                                                      |
         * file | a name of a file in which additional information is specified                                                                                      |
         * soft | a set of numbers which are allowed for the number of processes that can be spawned                                                                 |
         *
         * @note An implementation is not required to interpret these keys, but if it does interpret the key, it must provide the
         * functionality described.
         * @param[in] additional_info copy of the info object
         */
        template <typename... Infos>
        requires (std::is_same_v<std::decay_t<Infos>, info> && ...)
        void set_spawn_info(Infos... infos) noexcept(std::is_nothrow_copy_assignable_v<info>) {
            if constexpr (std::is_same_v<Strategy, single_spawner>) {
                MPICXX_ASSERT_PRECONDITION(sizeof...(infos) == 1, "Exactly 1 info object needed!");
            } else {
                MPICXX_ASSERT_PRECONDITION(sizeof...(infos) == strategy_.command().size(), "Exactly n info objects needed!");
            }
            strategy_.set_spawn_info(std::forward<Infos>(infos)...);
        }
        /**
 * @brief Returns the info object representing additional information for the runtime system where and how to spawn the processes.
 * @return the info object
 */
        [[nodiscard]] spawn_info_return_type spawn_info() const noexcept(is_spawn_info_noexcept) { return strategy_.spawn_info(); }
        [[nodiscard]] const info& spawn_info(const std::size_t i) const noexcept(noexcept(std::declval<multiple_spawner>().spawn_info(0))) {
            static_assert(std::is_same_v<Strategy, multiple_spawner>, "Can't call member function with a single spawner!");
            return strategy_.spawn_info(i);
        }


        /**
         * @brief Set the rank of the root process (from which the other processes are spawned).
         * @param[in] root the root process
         *
         * @pre @p root **must not** be less than `0` and greater or equal than the size of the communicator (set via
         * @ref set_communicator(MPI_Comm) or default *MPI_COMM_NULL*).
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
         * @return the root rank
         */
        [[nodiscard]] int root() const noexcept { return root_; }

        /**
         * @brief Intracommunicator containing the group of spawning processes.
         * @param[in] comm an intracommunicator
         *
         * @pre @p comm **must not** be *MPI_COMM_NULL*.
         * @pre The set rank **must be** valid in @p comm.
         *
         * @assert_precondition{ If @p comm is the null communicator. }
         * @assert_sanity{ If the specified root isn't valid in @p comm. }
         */
        void set_communicator(MPI_Comm comm) noexcept {
            MPICXX_ASSERT_PRECONDITION(comm != MPI_COMM_NULL, "Can't use null communicator!");
            MPICXX_ASSERT_SANITY(this->legal_root(root_, comm),
                    "The previously set root '{}' isn't a valid root in the new communicator!", root_);

            comm_ = comm;
        }
        /**
         * @brief Returns the intracommunicator containing the group of spawning processes.
         * @return the intracommunicator
         */
        [[nodiscard]] MPI_Comm communicator() const noexcept { return comm_; }

        /**
         * @brief Returns the intercommunicator between the original group and the newly spawned group.
         * @return the intercommunicator
         *
         * @pre @ref spawn() **must** already have been called.
         *
         * @assert_precondition{ If spawn hasn't been called yet. }
         */
        [[nodiscard]] MPI_Comm intercommunicator() const noexcept {
            MPICXX_ASSERT_SANITY(this->already_spawned(), "Spawn not called, so no intercommunicator has been created yet!");

            return intercomm_;
        }
        /**
         * @brief Returns the error codes (one code per process) returned by the spawn call.
         * @return the error codes
         *
         * @pre @ref spawn() **must** already have been called.
         *
         * @assert_precondition{ If spawn hasn't been called yet. }
         */
        [[nodiscard]] const std::vector<int>& errcodes() const noexcept {
            MPICXX_ASSERT_SANITY(this->already_spawned(), "Spawn not called, so no errcodes available yet!");

            return errcodes_;
        }
        /**
         * @brief Prints the number of failed spawns and the respective errcode message (including how often the errcode occurred).
         * @param[inout] out the output stream on which the errcodes messages should be written
         *
         * @pre @ref spawn() **must** already have been called.
         *
         * @assert_precondition{ If spawn hasn't been called yet. }
         *
         * @calls{
         * int MPI_Error_string(int errorcode, char *string, int *resultlen);       // at most 'maxprocs' times
         * }
         */
        void print_errors_to(std::ostream& out = std::cout) const {
            MPICXX_ASSERT_PRECONDITION(this->already_spawned(), "Spawn not called, so no errcodes available yet!");

            // count and display the number of errors
            const auto failed_spawns = std::count_if(errcodes_.cbegin(), errcodes_.cend(),
                    [](const int err) { return err != MPI_SUCCESS; });
            out << fmt::format("{} {} occurred!:\n", failed_spawns, failed_spawns == 1 ? "error" : "errors");

            // count how often each error occurred
            std::map<int, int> counts;
            for (const int err : errcodes_) {
                if (err != MPI_SUCCESS) {
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

        void spawn() {
            if constexpr (std::is_same_v<Strategy, single_spawner>) {

                if (strategy_.argv().empty()) {
                    MPI_Comm_spawn(
                            strategy_.command().c_str(),
                            MPI_ARGV_NULL,
                            strategy_.maxprocs(),
                            strategy_.spawn_info().get(),
                            root_, comm_, &intercomm_, errcodes_.data());
                } else {
                    // convert to char**
                    std::vector<char*> argv_ptr;
                    argv_ptr.reserve(strategy_.argv().size() * 2 + 1);
                    for (auto& [key, value] : strategy_.argv()) {
                        argv_ptr.emplace_back(key.data());
                        argv_ptr.emplace_back(value.data());
                    }
                    // add null termination
                    argv_ptr.emplace_back(nullptr);

                    MPI_Comm_spawn(
                            strategy_.command().c_str(),
                            argv_ptr.data(),
                            strategy_.maxprocs(),
                            strategy_.spawn_info().get(),
                            root_, comm_, &intercomm_, errcodes_.data());
                }

            } else {
                // TODO 2020-04-08 18:41 marcel: add
                std::vector<char*> command_ptr;
                command_ptr.reserve(strategy_.command().size());
                for (auto& command : strategy_.command()) {
                    command_ptr.emplace_back(command.data());
                }
                std::vector<MPI_Info> infos;
                infos.reserve(strategy_.spawn_info().size());
                for (auto& info : strategy_.spawn_info()) {
                    infos.emplace_back(info.get());
                }
                if (strategy_.argv().empty()) {
                    MPI_Comm_spawn_multiple(
                            static_cast<int>(strategy_.command().size()),
                            command_ptr.data(),
                            MPI_ARGVS_NULL,
                            strategy_.maxprocs().data(),
                            infos.data(),
                            root_, comm_, &intercomm_, errcodes_.data());
                } else {

                }
            }
        }

    private:
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
         * @return `true` if @p root is legal, otherwise `false`
         */
        bool legal_root(const int root, const MPI_Comm comm) const {
            return 0 <= root && root < this->comm_size(comm);
        }
        /*
         * @brief Checks whether @p maxprocs is valid, i.e. @maxprocs is greater than `0` and less or equal than the current universe size.
         * @param[in] maxprocs the number of processes which should be spawned
         * @return `true` if @p maxprocs is legal, otherwise `false`
         */
        bool legal_maxprocs(const int maxprocs) const {
            return 0 < maxprocs && maxprocs <= spawner::universe_size();
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

        Strategy strategy_;
        int root_ = 0;
        MPI_Comm comm_ = MPI_COMM_WORLD;

        MPI_Comm intercomm_ = MPI_COMM_NULL;
        std::vector<int> errcodes_;
    };


    spawner(detail::string auto&&, const int) -> spawner<single_spawner>;
    template <typename... Args>
    spawner(Args&&...) -> spawner<multiple_spawner>;


    // TODO 2020-03-22 19:28 marcel: return type, docu
    /**
     * @brief Returns the parent intracommunicator of the current process if the process was started with *MPI_COMM_SPAWN* or
     * *MPI_COMM_SPAWN_MULTIPLE*.
     * @return a [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional) containing the parent intracommunicator or
     * `std::nullopt`.
     */
    inline std::optional<MPI_Comm> parent_process() {
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
