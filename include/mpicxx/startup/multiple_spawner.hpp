/**
 * @file include/mpicxx/startup/multiple_spawner.hpp
 * @author Marcel Breyer
 * @date 2020-04-16
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
        template <std::input_iterator InputIt>
        multiple_spawner(InputIt first, InputIt last) {
            const auto size = std::distance(first, last);
            // set command and maxprocs according to passed values
            commands_.reserve(size);
            maxprocs_.reserve(size);
            for (; first != last; ++first) {
                const auto& pair = *first;
                commands_.emplace_back(pair.first);
                maxprocs_.emplace_back(pair.second);
            }
            // set info objects to default values
            infos_.assign(size, mpicxx::info::null);
        }
        multiple_spawner(std::initializer_list<std::pair<std::string, int>> ilist) : multiple_spawner(ilist.begin(), ilist.end()) { }


        // ---------------------------------------------------------------------------------------------------------- //
        //                                       getter/setter spawn information                                      //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name modify spawn information
        ///@{
        /**
         * @brief Replaces the old executable names with the from the range [@p first, @p last).
         * @param[in] first iterator to the first executable name in the range
         * @param[in] last iterator one-past the last executable name in the range
         * @return `*this`
         *
         * @pre The size of the range [@p first, @p last) **must** match the size of the constructed range.
         * @pre All commands in the range [@p first, @p last) **must not** be empty.
         *
         * @assert_sanity{
         * If the sizes mismatch. \n
         * If any new command is empty.
         * }
         */
        template <std::input_iterator InputIt>
        multiple_spawner& set_command(InputIt first, InputIt last) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(first, last),
                    "Illegal number of values! {} == {}", std::distance(first, last), commands_.size());

            commands_.assign(first, last);

            MPICXX_ASSERT_SANITY(this->legal_commands(commands_).first,
                    "No executable name given at {}!", this->legal_commands(commands_).second);

            return *this;
        }
        /**
         * @brief Replaces the old executable names with the from the initializer list @p ilist.
         * @param[in] ilist the new executable names
         * @return `*this`
         *
         * @pre The size of @p ilist **must** match the size of the constructed range.
         * @pre All commands in @p ilist **must not** be empty.
         *
         * @assert_sanity{
         * If the sizes mismatch. \n
         * If any new command is empty.
         * }
         */
        multiple_spawner& set_command(std::initializer_list<std::string> ilist) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(ilist),
                    "Illegal number of values! {} == {}", ilist.size(), commands_.size());

            commands_.assign(ilist);

            MPICXX_ASSERT_SANITY(this->legal_commands(commands_).first,
                    "No executable name given at {}!", this->legal_commands(commands_).second);

            return *this;
        }
        /**
         * @brief Set the i-th name of the executable to be spawned.
         * @param[in] i the i-th executable name
         * @param[in] command name of executable to be spawned (must meet the requirements of the @p detail::string concept)
         * @return `*this`
         *
         * @pre @p command **must not** be empty.
         *
         * @assert_sanity{ If @p command is empty. }
         *
         * @throws std::out_of_range if the index @p i is an out-of-bounce access
         */
        multiple_spawner& set_command(const std::size_t i, detail::string auto&& command) {
            if (i >= commands_.size()) {
                throw std::out_of_range(fmt::format("Out-of-bounce access!: {} < {}", i, commands_.size()));
            }

            commands_[i] = std::forward<decltype(command)>(command);

            MPICXX_ASSERT_SANITY(this->legal_command(commands_[i]), "No executable name given at {}!", i);

            return *this;
        }
        /**
         * @brief Returns the name of the executables which should get spawned.
         * @return the executable names (`[[nodiscard]]`)
         */
        [[nodiscard]] const std::vector<std::string>& command() const noexcept { return commands_; }
        /**
         * @brief Returns the name of the i-th executable which should get spawned.
         * @param[in] i the i-th command
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


        /**
         * @brief Replaces the old number of processes with the ones from the range [@p first, @p last).
         * @param[in] first iterator to the first number of processes in the range
         * @param[in] last iterator one-past the last number of processes in the range
         * @return `*this`
         *
         * @pre The size of the range [@p first, @p last) **must** match the size of the constructed range.
         * @pre All number of processes in the range [@p first, @p last) **must not** be less or equal than `0` or greater than the maximum
         * possible number of processes (@ref universe_size()).
         *
         * @assert_sanity{
         * If the sizes mismatch. \n
         * If any new number of processes is invalid.
         * }
         */
        template <std::input_iterator InputIt>
        multiple_spawner& set_maxprocs(InputIt first, InputIt last) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(first, last),
                    "Illegal number of values! {} == {}", std::distance(first, last), commands_.size());

            maxprocs_.assign(first, last);

            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs_).first,
                    "Can't spawn the given number of processes at {}!: 0 < {} <= {}",
                    this->legal_maxprocs(maxprocs_).second, maxprocs_[this->legal_maxprocs(maxprocs_).second],
                    multiple_spawner::universe_size().value_or(std::numeric_limits<int>::max()));

            return *this;
        }
        /**
         * @brief Replaces the old number of processes with the ones from the initializer list @p ilist.
         * @param[in] ilist the new number of processes
         * @return `*this`
         *
         * @pre The size of @p ilist **must** match the size of the constructed range.
         * @pre All number of processes in i@p ilist **must not** be less or equal than `0` or greater than the maximum possible number
         * of processes (@ref universe_size()).
         *
         * @assert_sanity{
         * If the sizes mismatch. \n
         * If any new number of processes is invalid.
         * }
         */
        multiple_spawner& set_maxprocs(std::initializer_list<int> ilist) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(ilist),
                    "Illegal number of values! {} == {}", ilist.size(), commands_.size());

            maxprocs_.assign(ilist);

            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs_).first,
                     "Can't spawn the given number of processes at {}!: 0 < {} <= {}",
                     this->legal_maxprocs(maxprocs_).second, maxprocs_[this->legal_maxprocs(maxprocs_).second],
                     multiple_spawner::universe_size().value_or(std::numeric_limits<int>::max()));

            return *this;
        }
        /**
         * @brief Set the i-th number of processes to be spawned.
         * @param[in] i the i-th number of processes
         * @param[in] maxprocs maximum number of processes to start
         * @return `*this`
         *
         * @pre @p maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         *
         * @assert_sanity{ If @p maxprocs is invalid. }
         *
         * @throws std::out_of_range if the index @p i is an out-of-bounce access
         */
        multiple_spawner& set_maxprocs(const std::size_t i, const int maxprocs) {
            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs),
                    "Can't spawn the given number of processes!:  0 < {} <= {}",
                    maxprocs, multiple_spawner::universe_size().value_or(std::numeric_limits<int>::max()));

            if (i >= maxprocs_.size()) {
                throw std::out_of_range(fmt::format("Out-of-bounce access!: {} < {}", i, maxprocs_.size()));
            }

            maxprocs_[i] = maxprocs;
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
         *
         * @throws std::out_of_range if the index @p i is an out-of-bounce access
         */
        [[nodiscard]] int maxprocs(const std::size_t i) const {
            if (i >= maxprocs_.size()) {
                throw std::out_of_range(fmt::format("Out-of-bounce access!: {} < {}", i, maxprocs_.size()));
            }

            return maxprocs_[i];
        }
        /**
         * @brief Returns the maximum possible number of processes.
         * @return an optional containing the maximum possible number of processes or `std::nullopt` if no value could be retrieved
         * (`[[nodiscard]]`)
         *
         * @note It may be possible that less than `universe_size` processes can be spawned if processes are already running.
         *
         * @calls{
         * int MPI_Comm_get_attr(MPI_Comm comm, int comm_keyval, void *attribute_val, int *flag);       // exactly once
         * }
         */
        [[nodiscard]] static std::optional<int> universe_size() {
            void* ptr;
            int flag;
            MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_UNIVERSE_SIZE, &ptr, &flag);
            if (static_cast<bool>(flag)) {
                return std::make_optional(*reinterpret_cast<int*>(ptr));
            } else {
                return std::nullopt;
            }
        }


        // TODO 2020-04-15 22:14 breyerml: infos

        template <std::input_iterator InputIt>
        multiple_spawner& set_spawn_info(InputIt first, InputIt last) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(first, last),
                    "Illegal number of values! {} == {}", std::distance(first, last), commands_.size());

            infos_.clear();
            infos_.insert(infos_.cbegin(), first, last);
            return *this;
        }
        multiple_spawner& set_spawn_info(std::initializer_list<info> ilist) noexcept { // TODO 2020-04-16 21:51 breyerml: noexcept
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(ilist), "Illegal number of values! {} == {}", ilist.size(), commands_.size());

            infos_.clear();
            infos_.insert(infos_.cbegin(), ilist);
            return *this;
        }
        [[nodiscard]] const std::vector<info>& spawn_info() const noexcept { return infos_; }
        [[nodiscard]] const info& spawn_info(const std::size_t i) const {
            if (i >= infos_.size()) {
                throw std::out_of_range(fmt::format("Out-of-bounce access!: {} < {}", i, infos_.size()));
            }

            return infos_[i];
        }
        // TODO 2020-04-17 00:11 breyerml: single spawner info same ???

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

        template <std::input_iterator InputIt>
        bool legal_number_of_values(InputIt first, InputIt last) {
            using difference_type = typename std::iterator_traits<InputIt>::difference_type;
            return std::distance(first, last) == static_cast<difference_type>(commands_.size());
        }
        template <typename T>
        bool legal_number_of_values(const std::initializer_list<T> ilist) const {
            return ilist.size() == commands_.size();
        }

        /*
         * @brief Check whether @p command is legal, i.e. it is **not** empty.
         * @param[in] command the command name
         * @return `true` if @p command is a valid name, `false` otherwise
         */
        bool legal_command(const std::string& command) const noexcept {
            return !command.empty();
        }
        std::pair<bool, std::size_t> legal_commands(const std::vector<std::string>& commands) const noexcept {
            for (std::size_t i = 0; i < commands.size(); ++i) {
                if (!this->legal_command(commands[i])) {
                    return std::make_pair(false, i);
                }
            }
            return std::make_pair(true, commands.size());
        }
        /*
         * @brief Checks whether @p maxprocs is valid.
         * @details Checks whether @p maxprocs is greater than `0`. In addition, if the universe size could be queried, it's checked
         * whether @p maxprocs is less or equal than the universe size.
         * @param[in] maxprocs the number of processes which should be spawned
         * @return `true` if @p maxprocs is legal, `false` otherwise
         */
        bool legal_maxprocs(const int maxprocs) const {
            std::optional<int> universe_size = multiple_spawner::universe_size();
            if (universe_size.has_value()) {
                return 0 < maxprocs && maxprocs <= universe_size.value();
            } else {
                return 0 < maxprocs;
            }
        }
        std::pair<bool, std::size_t> legal_maxprocs(const std::vector<int>& maxprocs) const {
            for (std::size_t i = 0; i < maxprocs.size(); ++i) {
                if (!this->legal_maxprocs(maxprocs[i])) {
                    return std::make_pair(false, i);
                }
            }
            return std::make_pair(true, maxprocs.size());
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
        std::vector<info> infos_;
        int root_ = 0;
        MPI_Comm comm_ = MPI_COMM_WORLD;
    };

}


#endif // MPICXX_MULTIPLE_SPAWNER_HPP
