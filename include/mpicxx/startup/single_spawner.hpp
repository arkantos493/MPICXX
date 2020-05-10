/**
 * @file include/mpicxx/startup/single_spawner.hpp
 * @author Marcel Breyer
 * @date 2020-05-10
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
#include <mpicxx/startup/spawn_result.hpp>


namespace mpicxx {

    // TODO 2020-03-22 19:04 marcel: change from MPI_Comm to mpicxx equivalent
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
         * @tparam T must meet the requirements of the @p detail::string concept
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
         template <detail::string T>
        single_spawner(T&& command, const int maxprocs) : command_(std::forward<T>(command)), maxprocs_(maxprocs) {
            MPICXX_ASSERT_SANITY(this->legal_command(command_), "No executable name given!");
            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs_),
                    "Can't spawn the given number of processes: 0 < {} <= {}",
                    maxprocs_, single_spawner::universe_size().value_or(std::numeric_limits<int>::max()));
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
        single_spawner(std::pair<T, int> pair) : single_spawner(std::move(pair.first), pair.second) { }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                       getter/setter spawn information                                      //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name modify spawn information
        ///@{
        /**
         * @brief Set the name of the executable to be spawned.
         * @tparam T must meet the requirements of the @p detail::string concept
         * @param[in] command name of executable to be spawned (must meet the requirements of the @p detail::string concept)
         * @return `*this`
         *
         * @pre @p command **must not** be empty.
         *
         * @assert_sanity{ If @p command is empty. }
         */
        template <detail::string T>
        single_spawner& set_command(T&& command) {
            command_ = std::forward<T>(command);

            MPICXX_ASSERT_SANITY(this->legal_command(command_), "No executable name given!");

            return *this;
        }
        /**
         * @brief Returns the name of the executable which should get spawned.
         * @return the executable name (`[[nodiscard]]`)
         */
        [[nodiscard]] const std::string& command() const noexcept { return command_; }

        /**
         * @brief Adds an argument ([key, value]-pair) the the `argv` list which gets passed to the spawned program.
         * @details Adds a leading `-` to @p key if not already present.
         *
         * Tries to convert @p val to a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) using
         * @ref detail::convert_to_string().
         * @tparam ValueType the type of the value
         * @param[in] key the argument key (e.g. `"-gridfile"` or `"gridfile"`)
         * @param[in] value the value associated with @p key
         * @return `*this`
         *
         * @pre @p key **must not** only contain '-' (or '').
         *
         * @assert_sanity{ If @p key only contains '-' (or ''). }
         */
        template <typename ValueType>
        single_spawner& add_argv(std::string key, ValueType&& value) {
            // add leading '-' if necessary
            if (!key.starts_with('-')) {
                key.insert(0, 1, '-');
            }

            MPICXX_ASSERT_SANITY(this->legal_argv_key(key), "Only '-' isn't a valid argument key!");

            // add [key, value]-argv-pair to argv_
            argv_.emplace_back(std::move(key), detail::convert_to_string(std::forward<ValueType>(value)));
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
         * @return `*this`
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
         * @return `*this`
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
         * @brief Set the maximum number of processes to start.
         * @param[in] maxprocs maximum number of processes to start
         * @return `*this`
         *
         * @pre @p maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         *
         * @assert_sanity{ If @p maxprocs is invalid. }
         */
        single_spawner& set_maxprocs(const int maxprocs) {
            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs),
                    "Can't spawn the given number of processes: 0 < {} <= {}",
                    maxprocs, single_spawner::universe_size().value_or(std::numeric_limits<int>::max()));

            maxprocs_ = maxprocs;
            return *this;
        }
        /**
         * @brief Returns the number of processes which should get spawned.
         * @return the number of processes (`[[nodiscard]]`)
         */
        [[nodiscard]] int maxprocs() const noexcept { return maxprocs_; }
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
        single_spawner& set_spawn_info(info additional_info) noexcept {
            info_ = std::move(additional_info);
            return *this;
        }
        /**
         * @brief Returns the info object representing additional information for the runtime system where and how to spawn the processes.
         * @return the info object (`[[nodiscard]]`)
         */
        [[nodiscard]] const info& spawn_info() const noexcept { return info_; }

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
        single_spawner& set_root(const int root) noexcept {
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
        single_spawner& set_communicator(MPI_Comm comm) noexcept {
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
         * @brief Spawns a number of MPI processes according to the previously set options.
         * @details Removes empty values before getting passed to *MPI_COMM_SPAWN*.
         *
         * The returned @ref mpicxx::spawn_result object **only** contains the intercommunicator.
         * @return a @ref mpicxx::spawn_result object holding the result of the @ref spawn() invocation (i.e. communicator and errcodes)
         *
         * @pre `command` **must not** be empty.
         * @pre All keys added to the `argvs` **must not** only contain '-' (or '').
         * @pre `maxprocs` **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         * @pre `root` **must not** be less than `0` and greater or equal than the size of the communicator (set via
         * @ref set_communicator(MPI_Comm) or default *MPI_COMM_WORLD*).
         * @pre `comm` **must not** be *MPI_COMM_NULL*.
         *
         * @assert_precondition{
         * If the `command` is empty. \n
         * If any key only contains '-' (or ''). \n
         * If `maxprocs` is invalid. \n
         * If `root` isn't a legal root. \n
         * If `comm` is the null communicator (*MPI_COMM_NULL*).
         * }
         *
         * @calls{
         * int MPI_Comm_spawn(const char *command, char *argv[], int maxprocs, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *intercomm, int array_of_errcodes[]);       // exactly once
         * }
         */
        spawn_result spawn() {
            return this->spawn_impl<spawn_result>();
        }
        /**
         * @brief Spawns a number of MPI processes according to the previously set options.
         * @details Removes empty values before getting passed to *MPI_COMM_SPAWN*.
         *
         * The returned @ref mpicxx::spawn_result_with_errcodes object contains the intercommunicator **and** information about the
         * possibly occurring error codes.
         * @return a @ref mpicxx::spawn_result_with_errcodes object holding the result of the @ref spawn_with_errcodes() invocation
         * (i.e. communicator and errcodes)
         *
         * @pre `command` **must not** be empty.
         * @pre All keys added to the `argvs` **must not** only contain '-' (or '').
         * @pre `maxprocs` **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         * @pre `root` **must not** be less than `0` and greater or equal than the size of the communicator (set via
         * @ref set_communicator(MPI_Comm) or default *MPI_COMM_WORLD*).
         * @pre `comm` **must not** be *MPI_COMM_NULL*.
         *
         * @assert_precondition{
         * If the `command` is empty. \n
         * If any key only contains '-' (or ''). \n
         * If `maxprocs` is invalid. \n
         * If `root` isn't a legal root. \n
         * If `comm` is the null communicator (*MPI_COMM_NULL*).
         * }
         *
         * @calls{
         * int MPI_Comm_spawn(const char *command, char *argv[], int maxprocs, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *intercomm, int array_of_errcodes[]);       // exactly once
         * }
         */
        spawn_result_with_errcodes spawn_with_errcodes() {
            return this->spawn_impl<spawn_result_with_errcodes>();
        }
        ///@}


    private:

        /*
         * @brief Spawns a number of MPI processes according to the previously set options.
         * @details Removes empty values before getting passed to *MPI_COMM_SPAWN*.
         *
         * Same as @ref spawn() but also returns error codes.
         * @tparam return_type either @ref mpicxx::spawn_result or @ref mpicxx::spawn_result_with_errcodes
         * @return the result of the spawn invocation
         *
         * @pre `command` **must not** be empty.
         * @pre All keys added to the `argvs` **must not** only contain '-' (or '').
         * @pre `maxprocs` **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         * @pre `root` **must not** be less than `0` and greater or equal than the size of the communicator (set via
         * @ref set_communicator(MPI_Comm) or default *MPI_COMM_WORLD*).
         * @pre `comm` **must not** be *MPI_COMM_NULL*.
         *
         * @assert_precondition{
         * If the `command` is empty. \n
         * If any key only contains '-' (or ''). \n
         * If `maxprocs` is invalid. \n
         * If `root` isn't a legal root. \n
         * If `comm` is the null communicator (*MPI_COMM_NULL*).
         * }
         *
         * @calls{
         * int MPI_Comm_spawn(const char *command, char *argv[], int maxprocs, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *intercomm, int array_of_errcodes[]);       // exactly once
         * }
         */
        template <typename return_type>
        return_type spawn_impl() {
            MPICXX_ASSERT_PRECONDITION(this->legal_command(command_), "No executable name given!");
            MPICXX_ASSERT_PRECONDITION(this->legal_argv_keys(argv_).first,
                    "Only '-' isn't a valid argument key!: wrong key at: {}", this->legal_argv_keys(argv_).second);
            MPICXX_ASSERT_PRECONDITION(this->legal_maxprocs(maxprocs_),
                    "Can't spawn the given number of processes: 0 < {} <= {}",
                    maxprocs_, single_spawner::universe_size().value_or(std::numeric_limits<int>::max()));
            MPICXX_ASSERT_PRECONDITION(this->legal_root(root_, comm_),
                    "The previously set root '{}' isn't a valid root in the current communicator!", root_);
            MPICXX_ASSERT_PRECONDITION(this->legal_communicator(comm_), "Can't use null communicator!");

            return_type res(maxprocs_);

            // determine whether the placeholder MPI_ERRCODES_IGNORE shall be used or a "real" std::vector
            auto errcode = [&res]() {
                if constexpr (std::is_same_v<return_type, spawn_result_with_errcodes>) {
                    return res.errcodes_.data();
                } else {
                    return MPI_ERRCODES_IGNORE;
                }
            }();

            if (argv_.empty()) {
                // no additional arguments provided -> use MPI_ARGV_NULL
                MPI_Comm_spawn(command_.c_str(), MPI_ARGV_NULL, maxprocs_, info_.get(),
                               root_, comm_, &res.intercomm_, errcode);
            } else {
                // convert additional arguments to char**
                std::vector<char*> argv_ptr;
                argv_ptr.reserve(argv_.size() * 2 + 1);
                for (auto& [key, value] : argv_) {
                    argv_ptr.emplace_back(key.data());
                    if (!value.empty()) {
                        argv_ptr.emplace_back(value.data());
                    }
                }
                // add null termination
                argv_ptr.emplace_back(nullptr);

                MPI_Comm_spawn(command_.c_str(), argv_ptr.data(), maxprocs_, info_.get(),
                               root_, comm_, &res.intercomm_, errcode);
            }
            return res;
        }

#if ASSERTION_LEVEL > 0
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
         * @brief Check whether all keys in @p argvs are legal, i.e. whether @ref legal_argv_key() const yields `true` for all keys.
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
         * @brief Checks whether @p maxprocs is valid.
         * @details Checks whether @p maxprocs is greater than `0`. In addition, if the universe size could be queried, it's checked
         * whether @p maxprocs is less or equal than the universe size.
         * @param[in] maxprocs the number of processes which should be spawned
         * @return `true` if @p maxprocs is legal, `false` otherwise
         */
        bool legal_maxprocs(const int maxprocs) const {
            std::optional<int> universe_size = single_spawner::universe_size();
            if (universe_size.has_value()) {
                return 0 < maxprocs && maxprocs <= universe_size.value();
            } else {
                return 0 < maxprocs;
            }
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
        std::string command_;
        std::vector<argv_value_type> argv_;
        int maxprocs_;
        info info_ = mpicxx::info::null;
        int root_ = 0;
        MPI_Comm comm_ = MPI_COMM_WORLD;
    };

    /// default spawner is a @ref single_spawner
    using spawner = single_spawner;

}


#endif // MPICXX_SINGLE_SPAWNER_HPP
