/**
 * @file include/mpicxx/startup/multiple_spawner.hpp
 * @author Marcel Breyer
 * @date 2020-04-15
 *
 * @brief Implements wrapper around the *MPI_COMM_SPAWN_MULTIPLE* function.
 */

#ifndef MPICXX_MULTIPLE_SPAWNER_HPP
#define MPICXX_MULTIPLE_SPAWNER_HPP

#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <mpi.h>

#include <mpicxx/detail/assert.hpp>
#include <mpicxx/detail/concepts.hpp>
#include <mpicxx/detail/conversion.hpp>
#include <mpicxx/info/info.hpp>
#include <mpicxx/startup/spawn_result.hpp>


namespace mpicxx {

    // TODO 2020-03-22 19:04 marcel: change from MPI_Comm to mpicxx equivalent
    // TODO 2020-03-23 12:56 marcel: errcode equivalent
    // TODO 2020-03-23 12:56 marcel: change from fmt::format to std::format
    // TODO 2020-03-23 17:37 marcel: copy/move constructor/assignment

    /**
     * @nosubgrouping
     * @brief Spawner class which enables to spawn (multiple) **different** MPI processes at runtime.
     */
    class multiple_spawner {
    public:
        /// The type of a single argv argument (represented by  a key and value).
        using argv_value_type = std::pair<std::string, std::string>;
        /// Unsigned integer type.
        using argv_size_type = std::size_t;

        // ---------------------------------------------------------------------------------------------------------- //
        //                                               constructor                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        template <typename... Args>
        requires ( (detail::string<typename Args::first_type> && std::is_same_v<std::decay_t<typename Args::second_type>, int>) && ...)
        multiple_spawner(Args&&... args) {
            commands_.reserve(sizeof...(Args));
            maxprocs_.reserve(sizeof...(Args));

            const auto add_to = [&]<typename T>(T&& arg) {
                commands_.emplace_back(std::forward<typename T::first_type>(arg.first));
                maxprocs_.emplace_back(arg.second);
            };

            (add_to(std::forward<Args>(args)), ...);

      }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                       getter/setter spawn information                                      //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name modify spawn information
        ///@{
        multiple_spawner& set_command() {
            // TODO 2020-04-15 22:15 breyerml: implement
            return *this;
        }
        /**
         * @brief Returns the name of the executables which should get spawned.
         * @return the executable names (`[[nodiscard]]`)
         */
        [[nodiscard]] const std::vector<std::string>& command() const noexcept { return commands_; }
        /**
         * @brief Returns the name of the i-th executable which should get spawned.
         * @return the i-th executable name  (`[[nodiscard]]`)
         *
         * @throws std::out_of_range if the index @p i is an out-of-bounce access
         */
        [[nodiscard]] const std::string& command(const std::size_t i) const {
            if (i >= commands_.size()) {
                throw std::out_of_range(fmt::format("Out-of-bounce access!: {} < {}", i, commands_.size()));
            }

            return commands_[i];
        }

        // TODO 2020-04-15 22:14 breyerml: argvs

        multiple_spawner& set_maxprocs() {
            // TODO 2020-04-15 22:15 breyerml: implement
            return *this;
        }
        /**
         * @brief Returns the number of processes which should get spawned.
         * @return the number of processes (`[[nodiscard]]`)
         */
        [[nodiscard]] const std::vector<int>& maxprocs() const noexcept { return maxprocs_; }
        /**
         * @brief Returns the i-th number of processes which should get spawned.
         * @return the i-th number of processes (`[[nodiscard]]`)
         */
        [[nodiscard]] int maxprocs(const std::size_t i) const {
            if (i >= maxprocs_.size()) {
                throw std::out_of_range(fmt::format("Out-of-bounce access!: {} < {}", i, maxprocs_.size()));
            }

            return maxprocs_[i];
        }

        // TODO 2020-04-15 22:14 breyerml: infos
        template <typename... Infos>
        requires (std::is_same_v<std::decay_t<Infos>, info> && ...)
        void set_spawn_info(Infos... infos) noexcept(std::is_nothrow_copy_assignable_v<info>) {
            // TODO 2020-03-24 17:29 marcel: asserts?, parameter type?, calls?
            const auto add_to = [&]<typename T>(T&& arg) {
                infos_.emplace_back(std::forward<T>(arg));
            };
            (add_to(std::forward<Infos>(infos)), ...);
        }
//        [[nodiscard]] const std::vector<info>& spawn_info() const noexcept { return infos_; }
//        [[nodiscard]] const info& spawn_info(const std::size_t i) const noexcept { return infos_[i]; }


        /**
         * @brief Set the rank of the root process (from which the other processes are spawned).
         * @param[in] root the root process
         * @return `*this`
         *
         * @pre @p root **must not** be less than `0` and greater or equal than the size of the communicator (set via
         * @ref set_communicator(MPI_Comm) or default *MPI_COMM_WORLD*).
         *
         * @assert_precondition{ If @p root isn't a legal root. }
         */
        multiple_spawner& set_root(const int root) noexcept {
            MPICXX_ASSERT_PRECONDITION(this->legal_root(root, comm_),
                    "The root can't be used in the provided communicator!: 0 <= {} < {}", root, this->comm_size(comm_));

            root_ = root;;
            return *this;
        }
        /**
         * @brief Returns the rank of the root process.
         * @return the root rank (`[[nodiscard]]`)
         */
        [[nodiscard]] int root() const noexcept { return root_; }

        /**
         * @brief Intracommunicator containing the group of spawning processes.
         * @param[in] comm an intracommunicator
         * @return `*this`
         *
         * @pre @p comm **must not** be *MPI_COMM_NULL*.
         * @pre The currently specified rank (as returned by @ref root()) **must be** valid in @p comm.
         *
         * @assert_precondition{ If @p comm is the null communicator (*MPI_COMM_NULL*). }
         * @assert_sanity{ If the currently specified root isn't valid in @p comm. }
         */
        multiple_spawner& set_communicator(MPI_Comm comm) noexcept {
            MPICXX_ASSERT_PRECONDITION(this->legal_communicator(comm), "Can't use null communicator!");
            MPICXX_ASSERT_SANITY(this->legal_root(root_, comm),
                                 "The previously set root '{}' isn't a valid root in the new communicator!", root_);

            comm_ = comm;
            return *this;
        }
        /**
         * @brief Returns the intracommunicator containing the group of spawning processes.
         * @return the intracommunicator (`[[nodiscard]]`)
         */
        [[nodiscard]] MPI_Comm communicator() const noexcept { return comm_; }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            spawn new process(es)                                           //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name spawn new process(es)
        ///@{
        /**
         * @brief TODO
         */
        spawn_result spawn() {
            return this->spawn_impl<spawn_result>();
        }
        /**
         * @brief @brief TODO
         * @return
         */
        spawn_result_with_errcodes spawn_with_errcodes() {
            return this->spawn_impl<spawn_result_with_errcodes>();
        }
        ///@}


    private:

        /*
         * @brief
         */
        template <typename return_type>
        return_type spawn_impl() {

            return_type res(std::reduce(maxprocs_.cbegin(), maxprocs_.cend()));

            return res;
        }

#if ASSERTION_LEVEL > 0
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
         * @brief Checks whether @p comm is valid, i.e. it does **not** refer to *MPI_COMM_NULL*.
         * @param[in] comm a intercommunicator
         * @return `true` if @p comm is valid, `false` otherwise
         */
        bool legal_communicator(const MPI_Comm comm) const noexcept {
            return comm != MPI_COMM_NULL;
        }
#endif

        std::vector<std::string> commands_;
        std::vector<std::vector<argv_value_type>> argvs_;
        std::vector<int> maxprocs_;
        std::vector<const info*> infos_;
        int root_ = 0;
        MPI_Comm comm_ = MPI_COMM_WORLD;
    };

}


#endif // MPICXX_MULTIPLE_SPAWNER_HPP
