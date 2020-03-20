/**
 * @file include/mpicxx/startup/spawn.hpp
 * @author Marcel Breyer
 * @date 2020-03-20
 *
 * @brief Implements wrapper around the MPI spawn functions.
 */

#ifndef MPICXX_SPAWN_HPP
#define MPICXX_SPAWN_HPP

#include <algorithm>
#include <cstring>
#include <iostream>
#include <ostream>
#include <span>
#include <string>
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


    class spawner {
    public:
        spawner(detail::string auto&& command, const int maxprocs)
            : command_(std::forward<decltype(command)>(command)), maxprocs_(maxprocs), errcodes_(maxprocs, -1)
        {
            MPICXX_ASSERT_SANITY(!command_.empty(), "An empty command name isn't executable!");
            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs),
                    "Can't spawn the given number of processes: 0 < {} <= {}.", maxprocs, spawner::spawnable());   // TODO 2020-03-20 19:22 marcel: check <= max
        }

        [[nodiscard]] const std::string& command() const noexcept { return command_; }
        [[nodiscard]] int maxprocs() const noexcept { return maxprocs_; }

        void set_info(info info) noexcept {
            info_ = std::move(info);
        }
        [[nodiscard]] const info& get_info() const noexcept { return info_; } // TODO 2020-03-20 19:26 marcel: name

        template <typename T>
        void add_argv(std::string key, T&& value) {
            // add leading '-' if necessary
            if (!key.starts_with('-')) {
                key.insert(0, 1, '-');
            }
            // add [key, value]-argv-pair to argvs
            argv_.emplace_back(std::move(key), detail::convert_to_string(std::forward<T>(value)));
        }
        [[nodiscard]] const std::vector<std::pair<std::string, std::string>>& argv() const noexcept { return argv_; }

        void set_root(const int root) noexcept {
            MPICXX_ASSERT_PRECONDITION(this->legal_root(root, comm_),
                    "The root can't be used in the provided communicator!: 0 <= {} < {}", root, this->comm_size(comm_));

            root_ = root;
        }
        [[nodiscard]] int root() const noexcept { return root_; }

        void set_comm(MPI_Comm comm) noexcept {
            MPICXX_ASSERT_PRECONDITION(comm != MPI_COMM_NULL, "Can't use null communicator!");
            MPICXX_ASSERT_SANITY(this->legal_root(root_, comm),
                    "The previously set root {} isn't a valid root in the new communicator!", root_); // TODO 2020-03-20 17:51 marcel: or PRECONDITION?

            comm_ = comm;
        }
        [[nodiscard]] MPI_Comm communicator() const noexcept { return comm_; }


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


        bool all_spawned() const {
            return std::all_of(errcodes_.cbegin(), errcodes_.cend(), [](const int err) { return err == MPI_SUCCESS; });
        }
        int spawned() const {
            if (intercomm_ == MPI_COMM_NULL) {
                return 0;
            } else {
                int size;
                MPI_Comm_remote_size(intercomm_, &size);
                return size;
            }
        }

        static int spawnable() {
            void* ptr;
            int flag;
            MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_UNIVERSE_SIZE, &ptr, &flag);
            if (static_cast<bool>(flag)) {
                int size;
                MPI_Comm_size(MPI_COMM_WORLD, &size);
                return *reinterpret_cast<int*>(ptr) - size;
            } else {
                return 0;
            }
        }


    private:
#if ASSERTION_LEVEL > 0
        int comm_size(const MPI_Comm comm) const {
            int size;
            MPI_Comm_size(comm, &size);
            return size;
        }
        bool legal_root(const int root, const MPI_Comm comm) const {
            return 0 <= root && root < this->comm_size(comm);
        }
        bool legal_maxprocs(const int maxprocs) const {
            return 0 < maxprocs && maxprocs <= spawner::spawnable();
        }
#endif

        const std::string command_;
        const int maxprocs_;
        std::vector<std::pair<std::string, std::string>> argv_;
        info info_ = info::null;
        int root_ = 0;
        MPI_Comm comm_ = MPI_COMM_WORLD;

        MPI_Comm intercomm_ = MPI_COMM_NULL;
        std::vector<int> errcodes_;
    };

    // return intercomm
    MPI_Comm parent_process() {
        MPI_Comm intercomm;
        MPI_Comm_get_parent(&intercomm);
        // if (intercomm == MPI_COMM_NULL) ...
        return intercomm;
    }

}


#endif // MPICXX_SPAWN_HPP
