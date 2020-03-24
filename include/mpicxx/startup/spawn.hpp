/**
 * @file include/mpicxx/startup/spawn.hpp
 * @author Marcel Breyer
 * @date 2020-03-24
 *
 * @brief Implements wrapper around the MPI spawn functions.
 */

#ifndef MPICXX_SPAWN_HPP
#define MPICXX_SPAWN_HPP

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


namespace mpicxx {

    // TODO 2020-03-22 19:04 marcel: change from MPI_Comm to mpicxx equivalent
    // TODO 2020-03-23 12:56 marcel: errcode equivalent
    // TODO 2020-03-23 12:56 marcel: change from fmt::format to std::format
    // TODO 2020-03-23 17:37 marcel: copy/move constructor/assignment

    /**
     * @brief Spawner class which enables to spawn MPI processes at runtime.
     */
    class spawner {
        /// the type of a single argv argument (including a key and a value)
        using argv_type = std::pair<std::string, std::string>;
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
            : command_(std::forward<decltype(command)>(command)), maxprocs_(maxprocs)
        {
            MPICXX_ASSERT_SANITY(!command_.empty(), "No executable name given!");
            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs),
                    "Can't spawn the given number of processes: 0 < {} <= {}.", maxprocs, spawner::universe_size());

            errcodes_ = std::vector<int>(maxprocs, -1);
        }

        /**
         * @brief Returns the name of the executable which should get spawned.
         * @return the executable name
         */
        [[nodiscard]] const std::string& command() const noexcept { return command_; }

        /**
         * @brief Returns the number of processes which should get spawned.
         * @return the number of processes
         */
        [[nodiscard]] int maxprocs() const noexcept { return maxprocs_; }
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

            return maxprocs_ == this->number_of_spawned_processes();
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
         * @return `*this`
         */
        spawner& set_spawn_info(info additional_info) noexcept(std::is_nothrow_move_assignable_v<info>) {
            // TODO 2020-03-22 18:39 marcel: asserts?, parameter type?, calls?
            info_ = std::move(additional_info);
            return *this;
        }
        /**
         * @brief Returns the info object representing additional information for the runtime system where and how to spawn the processes.
         * @return the info object
         */
        [[nodiscard]] const info& spawn_info() const noexcept { return info_; }

        /**
         * @brief Set the rank of the root process (from which the other processes are spawned).
         * @param[in] root the root process
         * @return `*this`
         *
         * @pre @p root **must not** be less than `0` and greater or equal than the size of the communicator (set via
         * @ref set_communicator(MPI_Comm) or default *MPI_COMM_NULL*).
         *
         * @assert_precondition{ If @p root isn't a legal root. }
         */
        spawner& set_root(const int root) noexcept {
            MPICXX_ASSERT_PRECONDITION(this->legal_root(root, comm_),
                    "The root can't be used in the provided communicator!: 0 <= {} < {}", root, this->comm_size(comm_));

            root_ = root;
            return *this;
        }
        /**
         * @brief Returns the rank of the root process.
         * @return the root rank
         */
        [[nodiscard]] int root() const noexcept { return root_; }

        /**
         * @brief Intracommunicator containing the group of spawning processes.
         * @param[in] comm an intracommunicator
         * @return `*this`
         *
         * @pre @p comm **must not** be *MPI_COMM_NULL*.
         * @pre The set rank **must be** valid in @p comm.
         *
         * @assert_precondition{ If @p comm is the null communicator. }
         * @assert_sanity{ If the specified root isn't valid in @p comm. }
         */
        spawner& set_communicator(MPI_Comm comm) noexcept {
            MPICXX_ASSERT_PRECONDITION(comm != MPI_COMM_NULL, "Can't use null communicator!");
            MPICXX_ASSERT_SANITY(this->legal_root(root_, comm),
                                 "The previously set root '{}' isn't a valid root in the new communicator!", root_);

            comm_ = comm;
            return *this;
        }
        /**
         * @brief Returns the intracommunicator containing the group of spawning processes.
         * @return the intracommunicator
         */
        [[nodiscard]] MPI_Comm communicator() const noexcept { return comm_; }

        /**
         * @brief Adds an argument pair the the `argv` list which gets passed to the spawned program.
         * @details Adds a leading `-` to @p key if not already present.
         *
         * Tries to convert @p val to a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string).
         * @tparam T the type of the value
         * @param[in] key the argument key (e.g. `"-gridfile"` or `"gridfile"`)
         * @param[in] value the value associated with @p key
         * @return `*this`
         */
        template <typename T>
        spawner& add_argv(std::string key, T&& value) {
            // add leading '-' if necessary
            if (!key.starts_with('-')) {
                key.insert(0, 1, '-');
            }
            // add [key, value]-argv-pair to argvs
            argv_.emplace_back(std::move(key), detail::convert_to_string(std::forward<T>(value)));
            return *this;
        }
        /**
         * @brief Returns the arguments which will be passed to `command`.
         * @return the arguments passed to `command`
         */
        [[nodiscard]] const std::vector<argv_type>& argv() const noexcept { return argv_; }
        /**
         * @brief Returns the i-th argument which will be passed to `command`.
         * @param[in] i the argv to return
         * @return the i-th argument
         *
         * @throws std::out_of_range if the index @p i is an out-of-bounce access
         */
        [[nodiscard]] const argv_type& argv(const std::size_t i) const {
            if (i >= argv_.size()) {
                throw std::out_of_range(fmt::format("Out-of-bounce access! doesn't exist!: {} < {}", i, argv_.size()));
            }

            return argv_[i];
        }

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

        /**
         * @brief Spawns a number of MPI processes according to the previously set options.
         *
         * @calls{
         * int MPI_Comm_spawn(const char *command, char *argv[], int maxprocs, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *intercomm, int array_of_errcodes[]);       // exactly once
         * }
         */
        void spawn() {
            if (argv_.empty()) {
                MPI_Comm_spawn(command_.c_str(), MPI_ARGV_NULL, maxprocs_, info_.get(), root_, comm_, &intercomm_, errcodes_.data());
            } else {
                // convert to char**
                std::vector<char*> argv_ptr;
                argv_ptr.reserve(argv_.size() * 2 + 1);
                for (auto& [key, value] : argv_) {
                    argv_ptr.emplace_back(key.data());
                    argv_ptr.emplace_back(value.data());
                }
                // add null termination
                argv_ptr.emplace_back(nullptr);

                MPI_Comm_spawn(command_.c_str(), argv_ptr.data(), maxprocs_, info_.get(), root_, comm_, &intercomm_, errcodes_.data());
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

        std::string command_;
        int maxprocs_;
        std::vector<argv_type> argv_;
        info info_ = info(MPI_INFO_NULL, false);
        int root_ = 0;
        MPI_Comm comm_ = MPI_COMM_WORLD;

        MPI_Comm intercomm_ = MPI_COMM_NULL;
        std::vector<int> errcodes_;
    };


    // TODO 2020-03-24 17:32 marcel: class hierarchy?
    /**
     * @brief Spawner class which enables to spawn multiple MPI processes at runtime.
     */
    class multi_spawner {
        using argv_type = std::pair<std::string, std::string>;

    public:
        template <typename... Args>
        requires ( (detail::string<typename Args::first_type> && std::is_same_v<std::decay_t<typename Args::second_type>, int>) && ...)
        multi_spawner(Args&&... args) {
            commands_.reserve(sizeof...(Args));
            maxprocs_.reserve(sizeof...(Args));

            const auto add_to = [&]<typename T>(T&& arg) {
                commands_.emplace_back(std::forward<typename T::first_type>(arg.first));
                maxprocs_.emplace_back(arg.second);
            };

            (add_to(std::forward<Args>(args)), ...);

            errcodes_ = std::vector<int>(std::reduce(maxprocs_.cbegin(), maxprocs_.cend()));
        }

        [[nodiscard]] const std::vector<std::string>& command() const noexcept { return commands_; }
        [[nodiscard]] const std::string& command(const std::size_t i) const noexcept { return commands_[i]; } // TODO 2020-03-24 17:09 marcel: exceptions?

        [[nodiscard]] const std::vector<int>& maxprocs() const noexcept { return maxprocs_; }
        [[nodiscard]] int maxprocs(const std::size_t i) const noexcept { return maxprocs_[i]; }

        [[nodiscard]] int number_of_spawned_processes() const {
            if (intercomm_ != MPI_COMM_NULL) {
                int size;
                MPI_Comm_remote_size(intercomm_, &size);
                return size;
            } else {
                return 0;
            }
        }
        [[nodiscard]] bool maxprocs_processes_spanwed() const {
            return errcodes_.size() == static_cast<std::size_t>(this->number_of_spawned_processes()); // TODO 2020-03-24 17:14 marcel: revise
        }
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

        template <typename... Infos>
        requires (std::is_same_v<std::decay_t<Infos>, info> && ...)
        multi_spawner& set_spawn_info(Infos... infos) noexcept(std::is_nothrow_copy_assignable_v<info>) {
            // TODO 2020-03-24 17:29 marcel: asserts?, parameter type?, calls?
            const auto add_to = [&]<typename T>(T&& arg) {
                infos_.emplace_back(std::forward<T>(arg));
            };
            (add_to(std::forward<Infos>(infos)), ...);
            return *this;
        }
        [[nodiscard]] const std::vector<info>& spawn_info() const noexcept { return infos_; }
        [[nodiscard]] const info& spawn_info(const std::size_t i) const noexcept { return infos_[i]; }

        multi_spawner& set_root(const int root) noexcept {
            root_ = root;
            return *this;
        }
        [[nodiscard]] int root() const noexcept { return root_; }

        multi_spawner& set_communicator(MPI_Comm comm) noexcept {
            comm_ = comm;
            return *this;
        }
        [[nodiscard]] MPI_Comm communicator() const noexcept { return comm_; }

    private:
        std::vector<std::string> commands_;
        std::vector<int> maxprocs_;
        std::vector<info> infos_;
        int root_;
        MPI_Comm comm_;

        MPI_Comm intercomm_;
        std::vector<int> errcodes_;
    };


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


#endif // MPICXX_SPAWN_HPP
