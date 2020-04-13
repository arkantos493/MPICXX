/**
 * @file include/mpicxx/startup/single_spawner.hpp
 * @author Marcel Breyer
 * @date 2020-04-13
 *
 * @brief Implements wrapper around the *MPI_COMM_SPAWN* function.
 */

#ifndef MPICXX_SINGLE_SPAWNER_HPP
#define MPICXX_SINGLE_SPAWNER_HPP

#include <iostream>
#include <iterator>
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
#include <mpicxx/startup/spawner_base.hpp>


namespace mpicxx {

    // TODO 2020-03-22 19:04 marcel: change from MPI_Comm to mpicxx equivalent
    // TODO 2020-03-23 12:56 marcel: change from int to mpicxx errcode equivalent
    // TODO 2020-03-23 12:56 marcel: change from fmt::format to std::format
    // TODO 2020-03-23 17:37 marcel: copy/move constructor/assignment

    // TODO 2020-04-13 22:29 breyerml: PRECONDITION or SANITY checks???

    /**
     * @nosubgrouping
     * @brief Spawner class which enables to spawn (multiple) MPI processes at runtime.
     */
    class single_spawner {
    public:
        /// The type of a single argv argument (represented by  a key and value).
        using argv_value_type = std::pair<std::string, std::string>;
        /// Unsigned integer type.
        using argv_size_type = std::size_t;

        // ---------------------------------------------------------------------------------------------------------- //
        //                                               constructor                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name constructor
        ///@{
        /**
         * @brief Construct a new single_spawner object.
         * @param[in] command name of program to be spawned (must meet the requirements of the @p detail::string concept)
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
            : base_(maxprocs), command_(std::forward<decltype(command)>(command)), maxprocs_(maxprocs)
        {
            MPICXX_ASSERT_SANITY(this->legal_command(command_), "No executable name given!");
        }
        /**
         * @brief Construct a new single_spawner object.
         * @tparam T must meet the requirements of the @p detail::string concept
         * @param[in] pair a [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) containing the name of the program to be spawned
         * and the maximum number of processes to start
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
        template <detail::string T>
        single_spawner(std::pair<T, int> pair) : single_spawner(std::forward<T>(pair.first), pair.second) { }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                       getter/setter spawn information                                      //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name modify spawn information
        ///@{
        /**
         * @brief Set the name of the program to be spawned,
         * @param[in] command name of program to be spawned (must meet the requirements of the @p detail::string concept)
         *
         * @pre @p command **must not** be empty.
         *
         * @assert_sanity{ If @p command is empty. }
         */
        single_spawner& set_command(detail::string auto&& command) {
            command_ = std::forward<decltype(command)>(command);

            MPICXX_ASSERT_SANITY(this->legal_command(command_), "No executable name given!");

            return *this;
        }
        /**
         * @brief Returns the name of the executable which should get spawned.
         * @return the executable name (`[[nodiscard]]`)
         */
        [[nodiscard]] const std::string& command() const noexcept { return command_; }

        /**
         * @brief Set the maximum number of processes to start.
         * @param[in] maxprocs maximum number of processes to start.
         *
         * @pre @p maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         *
         * @assert_sanity{ If @p maxprocs is invalid. }
         */
        single_spawner& set_maxprocs(const int maxprocs) {
            MPICXX_ASSERT_SANITY(base_.legal_maxprocs(maxprocs),
                    "Can't spawn the given number of processes: 0 < {} <= {}", maxprocs, single_spawner::universe_size());

            maxprocs_ = maxprocs;
            return *this;
        }
        /**
         * @brief Returns the number of processes which should get spawned.
         * @return the number of processes (`[[nodiscard]]`)
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
         *
         * @attention The user **has to ensure** that @p additional_info doesn't go out-of-scope before @ref spawn() gets called.
         */
        single_spawner& set_spawn_info(const info& additional_info) noexcept {
            info_ = &additional_info;
            return *this;
        }
        /**
         * @brief Delete the r-value overload to prevent some bugs in passing a temporary to @ref set_spawn_info().
         */
        single_spawner& set_spawn_info(info&&) = delete;
        /**
         * @brief Returns the info object representing additional information for the runtime system where and how to spawn the processes.
         * @return the info object (`[[nodiscard]]`)
         */
        [[nodiscard]] const info& spawn_info() const noexcept { return *info_; }

        /**
         * @brief Adds an argument ([key, value]-pair) the the `argv` list which gets passed to the spawned program.
         * @details Adds a leading `-` to @p key if not already present.
         *
         * Tries to convert @p val to a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) using
         * @ref detail::convert_to_string().
         * @tparam T the type of the value
         * @param[in] key the argument key (e.g. `"-gridfile"` or `"gridfile"`)
         * @param[in] value the value associated with @p key
         *
         * @pre @p key **must not** only contain '-' (or '').
         *
         * @assert_sanity{ If @p key only contains '-' (or ''). }
         */
        template <typename T>
        single_spawner& add_argv(std::string key, T&& value) {
            // add leading '-' if necessary
            if (!key.starts_with('-')) {
                key.insert(0, 1, '-');
            }

            MPICXX_ASSERT_SANITY(this->legal_argv_key(key), "Only '-' isn't a valid argument key!");

            // add [key, value]-argv-pair to argv_
            argv_.emplace_back(std::move(key), detail::convert_to_string(std::forward<T>(value)));
            return *this;
        }
        /**
         * @brief Adds all arguments ([key, value]-pairs) from the range [@p first, @p last)  to the `argvs` list which gets passed to the
         * spawned program.
         * @details Adds a leading `-` to all keys if not already present.
         *
         * Tries to convert all values to a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) using
         * @ref detail::convert_to_string().
         * @tparam InputIt must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator).
         * @param[in] first iterator to the first `argv` in the range
         * @param[in] last iterator one-past the last `argv` in the range
         *
         * @pre All keys **must not** only contain '-' (or '').
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid range, i.e. @p first must be less or equal than @p last.
         *
         * @assert_precondition{ If @p first and @p last don't denote a valid range. }
         * @assert_sanity{ If any key only contains '-' (or ''). }
         */
        template <std::input_iterator InputIt>
        single_spawner& add_argv(InputIt first, InputIt last) requires (!std::is_constructible_v<std::string, InputIt>) {
            MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");

            for (; first != last; ++first) {
                const auto& pair = *first;
                this->add_argv(pair.first, pair.second);
            }
            return *this;
        }
        /**
         * @brief Adds all arguments ([key, value]-pairs) from the initializer list @p ilist to the `argvs` list which gets passed to the
         * spawned program.
         * @details Adds a leading `-` to all keys if not already present.
         *
         * Tries to convert all values to a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) using
         * @ref detail::convert_to_string().
         * @tparam ValueType the type of the value
         * @param[in] ilist initializer list to insert the `argvs` from
         *
         * @pre All keys **must not** only contain '-' (or '').
         *
         * @assert_sanity{ If any key only contains '-' (or ''). }
         */
        template <typename ValueType>
        single_spawner& add_argv(std::initializer_list<std::pair<std::string, ValueType>> ilist) {
            return this->add_argv(ilist.begin(), ilist.end());
        }
        /**
         * @brief Returns the arguments which will be passed to `command`.
         * @return the arguments passed to `command` (`[[nodiscard]]`)
         */
        [[nodiscard]] const std::vector<argv_value_type>& argv() const noexcept { return argv_; }
        /**
         * @brief Returns the i-th argument which will be passed to `command`.
         * @param[in] i the argv to return
         * @return the i-th argument (`[[nodiscard]]`)
         *
         * @throws std::out_of_range if the index @p i is an out-of-bounce access
         */
        [[nodiscard]] const argv_value_type& argv(const std::size_t i) const {
            if (i >= argv_.size()) {
                throw std::out_of_range(fmt::format("Out-of-bounce access!: {} < {}", i, argv_.size()));
            }

            return argv_[i];
        }
        /**
         * @brief Returns the number of added arguments ([key, value]-pairs).
         * @return the number of added `argvs` (`[[nodiscard]]`)
         */
        [[nodiscard]] argv_size_type argv_size() const noexcept {
            return argv_.size();
        }

        /**
         * @copydoc detail::spawner_base::set_root(const int)
         */
        single_spawner& set_root(const int root) noexcept {
            base_.set_root(root);
            return *this;
        }
        /**
         * @copydoc detail::spawner_base::root()
         */
        [[nodiscard]] int root() const noexcept { return base_.root(); }

        /**
         * @copydoc detail::spawner_base::set_communicator(MPI_Comm)
         */
        single_spawner& set_communicator(MPI_Comm comm) noexcept {
            base_.set_communicator(comm);
            return *this;
        }
        /**
         * @copydoc detail::spawner_base::communicator()
         */
        [[nodiscard]] MPI_Comm communicator() const noexcept { return base_.communicator(); }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            spawn new process(es)                                           //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name spawn new process(es)
        ///@{
        /**
         * @brief Spawns a number of MPI processes according to the previously set options.
         *
         * @pre The set info object **should not** be out-of-scope.
         *
         * @assert_precondition{ If the info object is out-of-scope. }
         *
         * @calls{
         * int MPI_Comm_spawn(const char *command, char *argv[], int maxprocs, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *intercomm, int array_of_errcodes[]);       // exactly once
         * }
         */
        void spawn() {
            // TODO 2020-04-13 16:47 breyerml: check all preconditions once again
            MPICXX_ASSERT_PRECONDITION(this->legal_command(command_), "No executable name given!");
            MPICXX_ASSERT_PRECONDITION(this->legal_argv_keys(argv_).first,
                    "Only '-' isn't a valid argument key!: wrong key at: {}", this->legal_argv_keys(argv_).second);
            MPICXX_ASSERT_PRECONDITION(base_.legal_maxprocs(maxprocs_),
                    "Can't spawn the given number of processes: 0 < {} <= {}", maxprocs_, single_spawner::universe_size());
            MPICXX_ASSERT_PRECONDITION(this->legal_spawn_info(info_), "Can't use nullptr!");
            MPICXX_ASSERT_PRECONDITION(base_.legal_root(base_.root_, base_.comm_),
                    "The previously set root '{}' isn't a valid root in the current communicator!", base_.root_);
            MPICXX_ASSERT_PRECONDITION(base_.legal_communicator(base_.comm_), "Can't use null communicator!");

            if (argv_.empty()) {
                // no additional arguments provided -> use MPI_ARGV_NULL
                MPI_Comm_spawn(command_.c_str(), MPI_ARGV_NULL, maxprocs_, info_->get(),
                               base_.root_, base_.comm_, &base_.intercomm_, base_.errcodes_.data());
            } else {
                // convert additional arguments to char**
                std::vector<char*> argv_ptr;
                argv_ptr.reserve(argv_.size() * 2 + 1);
                for (auto& [key, value] : argv_) {
                    argv_ptr.emplace_back(key.data());
                    argv_ptr.emplace_back(value.data());
                }
                // add null termination
                argv_ptr.emplace_back(nullptr);

                MPI_Comm_spawn(command_.c_str(), argv_ptr.data(), maxprocs_, info_->get(),
                               base_.root_, base_.comm_, &base_.intercomm_, base_.errcodes_.data());
            }
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                   information after spawn has been called                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name lookup (after process spawning)
        /// (only meaningful if called after @ref single_spawner::spawn())
        ///@{
        /**
         * @copydoc detail::spawner_base::number_of_spawned_processes()
         */
        [[nodiscard]] int number_of_spawned_processes() const {
            return base_.number_of_spawned_processes();
        }
        /**
         * @copydoc detail::spawner_base::maxprocs_processes_spanwed()
         */
        [[nodiscard]] bool maxprocs_processes_spanwed() const {
            return base_.maxprocs_processes_spanwed();
        }

        /**
         * @copydoc detail::spawner_base::intercommunicator()
         */
        [[nodiscard]] MPI_Comm intercommunicator() const noexcept { return base_.intercommunicator(); }
        /**
         * @copydoc detail::spawner_base::errcodes()
         */
        [[nodiscard]] const std::vector<int>& errcodes() const noexcept {
            return base_.errcodes();
        }
        /**
         * @copydoc detail::spawner_base::print_errors_to()
         */
        void print_errors_to(std::ostream& out = std::cout) const {
            base_.print_errors_to(out);
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                           getter for spawn size                                            //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @copydoc detail::spawner_base::universe_size()
         */
        [[nodiscard]] static int universe_size() {
            return detail::spawner_base::universe_size();
        }


    private:
#if ASSERTION_LEVEL > 0
        /*
         * @brief Check whether @p first and @p last denote a valid range, i.e. @p first is less or equal than @p last.
         * @details Checks whether the distance bewteen @p first and @p last is not negative.
         * @param[in] first iterator to the first element of the range
         * @param[in] last iterator to one-past the last element of the range
         * @return `true` if @p first and @p last denote a valid range, `false` otherwise
         */
        template <std::input_iterator InputIt>
        bool legal_iterator_range(InputIt first, InputIt last) const {
            return std::distance(first, last) >= 0;
        }
        /*
         * @brief Check whether @p command is legal, i.e. it is **not** empty.
         * @param[in] command the command name
         * @return `true` if @p command is a valid name, `false` otherwise
         */
        bool legal_command(const std::string& command) const noexcept {
            return !command.empty();
        }
        /*
         * @brief Check whether @p key is legal, i.e. it does **not** only contain a '-'.
         * @param[in] key the argv key
         * @return `true` if @p key is valid, `false` otherwise
         */
        bool legal_argv_key(const std::string& key) const noexcept {
            return key.size() > 1;
        }
        /*
         * @brief Check whether all keys in @p argvs are legal, i.e. whether @ref legal_argv_key() yields `true` for all keys.
         * @param[in] argvs a vector of multiple argument [key, value]-pairs
         * @return `true` if all keys in @p argvs are valid, `false` otherwise
         */
        std::pair<bool, std::size_t> legal_argv_keys(const std::vector<argv_value_type>& argvs) const noexcept {
            for (std::size_t i = 0; i < argvs.size(); ++i) {
                if (!this->legal_argv_key(argvs[i].first)) {
                    return std::make_pair(false, i);
                }
            }
            return std::make_pair(true, argvs.size());
        }
        /*
         * @brief Check whether @p info is a valid pointer to an @ref mpicxx::info object, i.e. it does **not** refer to nullptr.
         * @param[in] info pointer to an @ref mpicxx::info object
         * @return `true` if info is valid, `false` otherwise
         */
        bool legal_spawn_info(const info* info) const noexcept {
            return info != nullptr;
        }
#endif
        detail::spawner_base base_;

        std::string command_;
        int maxprocs_;
        std::vector<argv_value_type> argv_;
        const info* info_ = &mpicxx::info::null;
    };

    /// default spawner is a @ref single_spawner
    using spawner = single_spawner;

}


#endif // MPICXX_SINGLE_SPAWNER_HPP
