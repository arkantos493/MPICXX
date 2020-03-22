/**
 * @file include/mpicxx/startup/spawn.hpp
 * @author Marcel Breyer
 * @date 2020-03-22
 *
 * @brief Implements wrapper around the MPI spawn functions.
 */

#ifndef MPICXX_SPAWN_HPP
#define MPICXX_SPAWN_HPP

#include <algorithm>
#include <cstring>
#include <iostream>
#include <initializer_list>
#include <numeric>
#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <mpi.h>

#include <mpicxx/detail/concepts.hpp>
#include <mpicxx/detail/conversion.hpp>
#include <mpicxx/info/info.hpp>
#include <mpicxx/detail/assert.hpp>


namespace mpicxx {

//    class spawner {
//        using arg_t = std::pair<std::string, std::string>;
//    public:
//        // TODO 2020-03-19 17:25 marcel: MPI_Comm_spawn_multiple: MPI_ARGVS_NULL (S!)
//        // TODO 2020-03-19 17:15 marcel: maxproc range? (non-negative) -> ASSERT
////        spawner(detail::string auto&& command, const int maxprocs, const int root)
////            : command_(std::forward<decltype(command)>(command)), maxprocs_(maxprocs), root_(root), errcodes_(maxprocs, -1) { }
//
//        spawner(std::string command) : count_(1), command_(std::move(command)) { }
//        spawner(std::span<std::string> commands) : count_(static_cast<int>(commands.size())) { }
//
////        template <detail::string T>
////        spawner(const int count, std::span<T> commands, const std::span<int> maxprocs, const int root, const MPI_Comm comm)
////            :
//
//        void spawn() {
//            // TODO 2020-03-19 17:15 marcel: revise: MPI_ERRCODES_IGNORE, return this->successful()?
//            std::vector<char*> argv;
//            argv.reserve(argv_.size() * 2);
//            for (auto& [key, value] : argv_) {
//                argv.emplace_back(key.data());
//                argv.emplace_back(value.data());
//            }
//            // add NULL termination
//            argv.emplace_back(nullptr);
//            MPI_Comm_spawn(command_.c_str(), argv.empty() ? MPI_ARGV_NULL : argv.data(), maxprocs_, info_.get(), root_, comm_, &intercomm_, errcodes_.data());
//        }
//
//        template <typename T>
//        void add_argv(std::string key, T&& value) {
//            // add leading '-' to the argument key if not already present
//            if (!key.starts_with('-')) {
//                key.insert(0, "-");
//            }
//
//            // add argument
//            argv_.emplace_back(std::move(key), detail::convert_to_string(std::forward<T>(value)));
//        }
//
//        std::size_t number_of_remaining_processes() const {
//            // TODO 2020-03-19 16:42 marcel: revise
//            void* ptr;
//            int flag;
//            MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_UNIVERSE_SIZE, &ptr, &flag);
//            if (static_cast<bool>(flag)) {
//                int size;
//                MPI_Comm_size(MPI_COMM_WORLD, &size);
//                return *reinterpret_cast<int*>(ptr) - size;
//            } else {
//                return 0;
//            }
//        }
//
//        std::size_t number_of_spawned_processes() const {
//            // TODO 2020-03-19 17:02 marcel: revise (nullptr): hard <-> soft
//            int size;
//            MPI_Comm_remote_size(intercomm_, &size);
//            return static_cast<std::size_t>(size);
//        }
//
//        void add_info(const info& info) {
//            // TODO 2020-03-19 17:06 marcel: revise
//            info_ = info;
//        }
//
//        bool successful() const {
//            // TODO 2020-03-19 17:12 marcel: revise
//            return std::all_of(errcodes_.begin(), errcodes_.end(), [](int err) { return err == MPI_SUCCESS; });
//        }
//
//        const std::vector<int>& errcodes() const noexcept { return errcodes_; }
//        MPI_Comm intercomm() const noexcept { return intercomm_; }



    // TODO 2020-03-22 19:04 marcel: change from MPI_Comm to mpicxx equivalent

    /**
     * @brief Spawner class which enables to spawn MPI processes at runtime.
     */
    class spawner {
        using argv_type = std::pair<std::string, std::string>;
    public:
        spawner(detail::string auto&& command, const int maxprocs)
            : command_(std::forward<decltype(command)>(command)), maxprocs_(maxprocs), errcodes_(maxprocs, -1)
        {
            MPICXX_ASSERT_SANITY(!command_.empty(), "No executable name given!");
            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs),
                    "Can't spawn the given number of processes: 0 < {} <= {}.", maxprocs, spawner::universe_size());
        }
//        spawner(std::initializer_list<std::pair<std::string, int>> ilist) {
//            for (auto&& pair : ilist) {
//                commands_.emplace_back(std::move(pair.first));
//                maxprocs_.emplace_back(pair.second);
//            }
//            errcodes_ = std::vector<int>(std::reduce(maxprocs_.cbegin(), maxprocs_.cend()), -1);
//        }

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
         * @calls{
         * int MPI_Comm_remote_size(MPI_Comm comm, int *size);      // at most once
         * }
         */
        [[nodiscard]] int number_of_spawned_processes() const {
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
         */
        [[nodiscard]] bool maxprocs_processes_spanwed() const {
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
         * @brief Set the spawn info object representing additional information for the runtime system where and how to spawn the processes.
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
         * @note An implementation is not required to interpret these keys, bit if it does interpret the key, it must provide the
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
                                 "The previously set root {} isn't a valid root in the new communicator!", root_);

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



        [[nodiscard]] MPI_Comm intercommunicator() const noexcept { return intercomm_; }
        [[nodiscard]] const std::vector<int>& errcodes() const noexcept { return errcodes_; }


        // TODO 2020-03-20 18:36 marcel: where to ignore?
        void spawn(const bool ignore = false) {
            std::vector<char*> argv_ptr;
            argv_ptr.reserve(argv_.size() * 2 + 1);
            for (auto& [key, value] : argv_) {
                argv_ptr.emplace_back(key.data());
                argv_ptr.emplace_back(value.data());
            }
            // add NULL termination
            argv_ptr.emplace_back(nullptr);

            MPI_Comm_spawn(command_.c_str(), argv_ptr.size() == 1 ? MPI_ARGV_NULL : argv_ptr.data(),
                           maxprocs_, info_.get(), root_, comm_, &intercomm_, ignore ? MPI_ERRCODES_IGNORE : errcodes_.data());
        }




        friend std::ostream& operator<<(std::ostream& out, const spawner& sp) {
            out << "command: " << sp.command_ << std::endl;
            out << "maxprocs: " << sp.maxprocs_ << std::endl;
            out << "root: " << sp.root_ << std::endl;

            for (const auto& a : sp.argv_) {
                std::cout << a.first << " " << a.second << std::endl;
            }

            return out;
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
#endif

        const std::string command_;
        const int maxprocs_;
//        std::vector<std::string> commands_;
//        std::vector<int> maxprocs_;
        std::vector<argv_type> argv_;
        info info_ = info(MPI_INFO_NULL, false);
        int root_ = 0;
        MPI_Comm comm_ = MPI_COMM_WORLD;

        MPI_Comm intercomm_ = MPI_COMM_NULL;
        std::vector<int> errcodes_;
    };

    // TODO 2020-03-22 19:28 marcel: return type, docu
    /**
     * @brief Returns the parent intracommunicator of the current process if the process was started with *MPI_COMM_SPAWN* or
     * *MPI_COMM_SPAWN_MULTIPLE*.
     * @return a [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional) containing the parent intracommunicator or
     * `std::nullopt`.
     */
    std::optional<MPI_Comm> parent_process() {
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
