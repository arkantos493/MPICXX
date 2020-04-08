/**
 * @file include/mpicxx/startup/spawner.hpp
 * @author Marcel Breyer
 * @date 2020-04-08
 *
 * @brief Implements wrapper around the MPI spawn functions.
 */

#ifndef MPICXX_SPAWNER_HPP
#define MPICXX_SPAWNER_HPP

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
#include <mpicxx/startup/spawner_base.hpp>

namespace mpicxx {

    // TODO 2020-03-22 19:04 marcel: change from MPI_Comm to mpicxx equivalent
    // TODO 2020-03-23 12:56 marcel: errcode equivalent
    // TODO 2020-03-23 12:56 marcel: change from fmt::format to std::format
    // TODO 2020-03-23 17:37 marcel: copy/move constructor/assignment

    /**
     * @brief Spawner class which enables to spawn MPI processes at runtime.
     */
    class single_spawner {
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
        single_spawner(detail::string auto&& command, const int maxprocs)
            : command_(std::forward<decltype(command)>(command)), maxprocs_(maxprocs)
        { }

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
        void set_spawn_info(info additional_info) noexcept(std::is_nothrow_move_assignable_v<info>) {
            // TODO 2020-03-22 18:39 marcel: asserts?, parameter type?, calls?
            info_ = std::move(additional_info);
        }
        /**
         * @brief Returns the info object representing additional information for the runtime system where and how to spawn the processes.
         * @return the info object
         */
        [[nodiscard]] const info& spawn_info() const noexcept { return info_; }

        /**
         * @brief Adds an argument pair the the `argv` list which gets passed to the spawned program.
         * @details Adds a leading `-` to @p key if not already present.
         *
         * Tries to convert @p val to a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string).
         * @tparam T the type of the value
         * @param[in] key the argument key (e.g. `"-gridfile"` or `"gridfile"`)
         * @param[in] value the value associated with @p key
         */
        template <typename T>
        void add_argv(std::string key, T&& value) {
            // add leading '-' if necessary
            if (!key.starts_with('-')) {
                key.insert(0, 1, '-');
            }
            // add [key, value]-argv-pair to argvs
            argv_.emplace_back(std::move(key), detail::convert_to_string(std::forward<T>(value)));
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
                throw std::out_of_range(fmt::format("Out-of-bounce access!: {} < {}", i, argv_.size()));
            }

            return argv_[i];
        }

        /**
         * @brief Spawns a number of MPI processes according to the previously set options.
         *
         * @calls{
         * int MPI_Comm_spawn(const char *command, char *argv[], int maxprocs, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *intercomm, int array_of_errcodes[]);       // exactly once
         * }
         */
//        void spawn() override {
//            if (argv_.empty()) {
//                MPI_Comm_spawn(command_.c_str(), MPI_ARGV_NULL, maxprocs_, info_.get(), root_, comm_, &intercomm_, errcodes_.data());
//            } else {
//                // convert to char**
//                std::vector<char*> argv_ptr;
//                argv_ptr.reserve(argv_.size() * 2 + 1);
//                for (auto& [key, value] : argv_) {
//                    argv_ptr.emplace_back(key.data());
//                    argv_ptr.emplace_back(value.data());
//                }
//                // add null termination
//                argv_ptr.emplace_back(nullptr);
//
//                MPI_Comm_spawn(command_.c_str(), argv_ptr.data(), maxprocs_, info_.get(), root_, comm_, &intercomm_, errcodes_.data());
//            }
//        }

    private:
        std::string command_;
        int maxprocs_;
        std::vector<argv_type> argv_;
        info info_ = info(MPI_INFO_NULL, false);
    };

}


#endif // MPICXX_SPAWNER_HPP
