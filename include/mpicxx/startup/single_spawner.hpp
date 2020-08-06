/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-24
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Implements wrapper around the [*MPI_Comm_spawn*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node237.htm) function.
 */

#ifndef MPICXX_SINGLE_SPAWNER_HPP
#define MPICXX_SINGLE_SPAWNER_HPP

#include <mpicxx/detail/assert.hpp>
#include <mpicxx/detail/concepts.hpp>
#include <mpicxx/detail/conversion.hpp>
#include <mpicxx/info/info.hpp>
#include <mpicxx/info/runtime_info.hpp>
#include <mpicxx/startup/spawn_result.hpp>

#include <fmt/format.h>
#include <mpi.h>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace mpicxx {

    // TODO 2020-05-10 23:31 breyerml: change from MPI_Comm to mpicxx equivalent -> copy/move constructor/assignment

    /**
     * @nosubgrouping
     * @brief Spawner class which enables to spawn (multiple) MPI processes at runtime.
     */
    class single_spawner {
    public:
        /// Unsigned integer type.
        using argv_size_type = std::size_t;

        // ---------------------------------------------------------------------------------------------------------- //
        //                                               constructor                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name constructor
        ///@{
        /**
         * @brief Construct a new @ref mpicxx::single_spawner object.
         * @tparam T must meet the @p mpicxx::detail::is_string requirements
         * @param[in] command name of the executable
         * @param[in] maxprocs maximum number of processes
         *
         * @pre @p command **must not** be empty.
         * @pre @p maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         *
         * @assert_sanity{ If @p command is empty. \n
         *                 If @p maxprocs is invalid. }
         */
        template <detail::is_string T>
        single_spawner(T&& command, const int maxprocs) : command_(std::forward<T>(command)), maxprocs_(maxprocs) {
            MPICXX_ASSERT_SANITY(this->legal_command(command_), "Attempt to set executable name to the empty string!");
            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs_),
                    "Attempt to set the maxprocs value (which is {}), which falls outside the valid range (0, {}]!",
                    maxprocs, mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));
        }
        /**
         * @brief Construct a new @ref mpicxx::single_spawner object.
         * @tparam T must meet the @p mpicxx::detail::is_string requirements
         * @param[in] pair a [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) containing the executable name and the
         *                 maximum number of processes
         *
         * @pre command (@p pair.first) **must not** be empty.
         * @pre maxprocs (@p pair.second **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         *
         * @assert_sanity{ If command (@p pair.first) is empty. \n
         *                 If maxprocs (@p pair.second) is invalid. }
         */
        template <detail::is_string T>
        explicit single_spawner(std::pair<T, int> pair) : single_spawner(std::move(pair.first), pair.second) { }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                          modify spawn information                                          //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name modify spawn information
        ///@{
        /**
         * @brief Replace the old executable name with the new executable name @p command.
         * @tparam T must meet the @p mpicxx::detail::is_string requirements
         * @param[in] command the new executable name
         * @return `*this`
         *
         * @pre @p command **must not** be empty.
         *
         * @assert_sanity{ If @p command is empty. }
         */
        template <detail::is_string T>
        single_spawner& set_command(T&& command) {
            command_ = std::forward<T>(command);

            MPICXX_ASSERT_SANITY(this->legal_command(command_), "Attempt to set executable name to the empty string!");

            return *this;
        }

        /**
         * @brief Adds all command line arguments in the range [@p first, @p last) to the executable.
         * @tparam InputIt must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator)
         * @param[in] first iterator to the first command line argument in the range
         * @param[in] last iterator one-past the last command line argument in the range
         * @return `*this`
         *
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid range, i.e. @p first must be less or equal than @p last.
         * @pre All command line arguments **must not** be empty.
         *
         * @assert_precondition{ If @p first and @p last don't denote a valid range. }
         * @assert_sanity{ If any command line argument is empty. }
         */
        template <std::input_iterator InputIt>
        single_spawner& add_argv(InputIt first, InputIt last) requires (!detail::is_c_string<InputIt>) {
            MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");

            for (; first != last; ++first) {
                this->add_argv(*first);
            }
            return *this;
        }
        /**
         * @brief Adds all command line arguments in the
         *        [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) @p ilist to the executable.
         * @tparam T must be convertible to [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string)
         *           via @ref mpicxx::detail::convert_to_string
         * @param[in] ilist the (additional) command line arguments
         * @return `*this`
         *
         * @pre All command line arguments **must not** be empty.
         *
         * @assert_sanity{ If any command line argument is empty. }
         */
        template <typename T = std::string>
        single_spawner& add_argv(std::initializer_list<T> ilist) {
            return this->add_argv(ilist.begin(), ilist.end());
        }
        /**
         * @brief Adds all command line arguments in the parameter pack @p args to the executable.
         * @tparam T must be convertible to [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string)
         *           via @ref mpicxx::detail::convert_to_string and must not be empty
         * @param[in] args the (additional) command line arguments
         * @return `*this`
         *
         * @pre All command line arguments in @p args **must not** be empty.
         *
         * @assert_sanity{ If any command line argument is empty. }
         */
        template <typename... T>
        single_spawner& add_argv(T&&... args) requires (sizeof...(T) > 0) {
            ([&](auto&& arg) {
                // convert argument to a std::string
                std::string argv = detail::convert_to_string(std::forward<decltype(arg)>(arg));

                MPICXX_ASSERT_SANITY(this->legal_argv(argv), "Attempt to set an empty command line argument!");

                // add command line argument
                argvs_.emplace_back(std::move(argv));
            }(std::forward<T>(args)), ...);
            return *this;
        }

        /**
         * @brief Replaces the old number of processes with the new number of processes @p maxprocs.
         * @param[in] maxprocs the new number of processes
         * @return `*this`
         *
         * @pre @p maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         *
         * @assert_sanity{ If @p maxprocs is invalid. }
         */
        single_spawner& set_maxprocs(const int maxprocs) {
            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs_),
                    "Attempt to set the maxprocs value (which is {}), which falls outside the valid range (0, {}]!",
                    maxprocs, mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));

            maxprocs_ = maxprocs;
            return *this;
        }

        /**
         * @brief Set the info object representing additional information for the runtime system where and how to spawn the processes.
         * @details As of [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf) reserved keys are:
         * key  | description
         * :----| :--------------------------------------------------------------------------------------------------------------------------------------------------|
         * host | a hostname                                                                                                                                         |
         * arch | an architecture name                                                                                                                               |
         * wdir | a name of a directory on a machine on which the spawned processes execute; this directory is made the working directory of the executing processes |
         * path | a directory or set of directories where the MPI implementation should look for the executable                                                      |
         * file | a name of a file in which additional information is specified                                                                                      |
         * soft | a set of numbers which are allowed for the number of processes that can be spawned                                                                 |
         * @note An implementation is not required to interpret these keys, but if it does interpret the key, it must provide the
         *       functionality described.
         * @param[in] spawn_info copy of the @ref info object
         * @return `*this`
         */
        single_spawner& set_spawn_info(info spawn_info) noexcept {
            info_ = std::move(spawn_info);
            return *this;
        }

        /**
         * @brief Set the rank of the root process (from which the other processes are spawned).
         * @param[in] root the root process
         * @return `*this`
         *
         * @pre @p root **must not** be less than `0` and greater or equal than the size of the communicator (set via
         *      @ref set_communicator(MPI_Comm) or default
         *      [*MPI_COMM_WORLD*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm)).
         *
         * @assert_sanity{ If @p root isn't a legal root. }
         */
        single_spawner& set_root(const int root) noexcept {
            MPICXX_ASSERT_SANITY(this->legal_root(root, comm_),
                    "Attempt to set the root process (which is {}), which falls outside the valid range [0, {})!",
                    root, this->comm_size(comm_));

            root_ = root;;
            return *this;
        }

        /**
         * @brief Intracommunicator containing the group of spawning processes.
         * @param[in] comm an intracommunicator
         * @return `*this`
         *
         * @pre @p comm **must not** be [*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm).
         * @pre The currently specified rank (as returned by @ref root()) **must be** valid in @p comm.
         *
         * @assert_precondition{ If @p comm is the null communicator
         *                       ([*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm)). }
         * @assert_sanity{ If the currently specified root isn't valid in @p comm. }
         */
        single_spawner& set_communicator(MPI_Comm comm) noexcept {
            MPICXX_ASSERT_PRECONDITION(this->legal_communicator(comm), "Attempt to set the communicator to MPI_COMM_NULL!");
            MPICXX_ASSERT_SANITY(this->legal_root(root_, comm),
                    "The previously set root (which is {}) isn't a valid root in the new communicator anymore!", root_);

            comm_ = comm;
            return *this;
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            get spawn information                                           //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name get spawn information
        ///@{
        /**
         * @brief Returns the name of the executable which should get spawned.
         * @return the executable name
         * @nodiscard
         */
        [[nodiscard]]
        const std::string& command() const noexcept { return command_; }

        /**
         * @brief Returns all command line arguments.
         * @return the command line arguments
         * @nodiscard
         */
        [[nodiscard]]
        const std::vector<std::string>& argv() const noexcept { return argvs_; }
        /**
         * @brief Returns the @p i-th command line argument.
         * @param[in] i the index of the command line argument
         * @return the @p i-th command line argument
         * @nodiscard
         *
         * @pre @p i **must not** be greater than `this->argv_size()`.
         *
         * @throws std::out_of_range if the index @p i falls outside the valid range
         */
        [[nodiscard]]
        const std::string& argv_at(const std::size_t i) const {
            if (i >= argvs_.size()) {
                throw std::out_of_range(fmt::format(
                        "single_spawner::argv_at(const std::size_t) range check: i (which is {}) >= argvs_.size() (which is {})",
                        i, argvs_.size()));
            }

            return argvs_[i];
        }
        /**
         * @brief Returns the number of command line arguments.
         * @return the number of command line arguments
         * @nodiscard
         */
        [[nodiscard]]
        argv_size_type argv_size() const noexcept {
            return argvs_.size();
        }

        /**
         * @brief Returns the number of processes.
         * @return the number of processes
         * @nodiscard
         */
        [[nodiscard]]
        int maxprocs() const noexcept { return maxprocs_; }

        /**
         * @brief Returns the info object.
         * @return the info object
         * @nodiscard
         */
        [[nodiscard]]
        const info& spawn_info() const noexcept { return info_; }

        /**
         * @brief Returns the rank of the root process.
         * @return the root rank
         * @nodiscard
         */
        [[nodiscard]]
        int root() const noexcept { return root_; }

        /**
         * @brief Returns the intracommunicator containing the group of spawning processes.
         * @return the intracommunicator
         * @nodiscard
         */
        [[nodiscard]]
        MPI_Comm communicator() const noexcept { return comm_; }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            spawn new process(es)                                           //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name spawn new process(es)
        ///@{
        /**
         * @brief Spawns a number of MPI processes according to the previously set options.
         * @details The returned @ref mpicxx::spawn_result object **only** contains the intercommunicator.
         *
         *    Example: @snippet examples/startup/single_spawner.cpp spawn without error codes
         * @return the result of the spawn invocation
         *
         * @pre The executable name **must not** be empty.
         * @pre All command line arguments **must not** be empty.
         * @pre maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre root **must not** be less than `0` and greater or equal than the size of the communicator (set via
         *      @ref set_communicator(MPI_Comm) or default
         *      [*MPI_COMM_WORLD*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm)).
         * @pre comm **must not** be [*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm).
         *
         * @assert_precondition{ If the executable name is empty. \n
         *                       If any command line argument is empty. \n
         *                       If maxprocs is invalid. \n
         *                       If root isn't a legal root. \n
         *                       If comm is the null communicator
         *                       ([*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm)). }
         *
         * @calls{
         * int MPI_Comm_spawn(const char *command, char *argv[], int maxprocs, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *intercomm, int array_of_errcodes[]);    // exactly once
         * }
         */
        spawn_result spawn() {
            return this->spawn_impl<spawn_result>();
        }
        /**
         * @brief Spawns a number of MPI processes according to the previously set options.
         * @details The returned @ref mpicxx::spawn_result_with_errcodes object contains the intercommunicator **and** information about the
         *          possibly occurring error codes.
         *
         *    Example: @snippet examples/startup/single_spawner.cpp spawn with error codes
         * @return the result of the spawn invocation
         *
         * @pre The executable name **must not** be empty.
         * @pre All command line arguments **must not** be empty.
         * @pre maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre root **must not** be less than `0` and greater or equal than the size of the communicator (set via
         *      @ref set_communicator(MPI_Comm) or default
         *      [*MPI_COMM_WORLD*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm)).
         * @pre comm **must not** be [*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm).
         *
         * @assert_precondition{ If the executable name is empty. \n
         *                       If any command line argument is empty. \n
         *                       If maxprocs is invalid. \n
         *                       If root isn't a legal root. \n
         *                       If comm is the null communicator
         *                       ([*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm)). }
         *
         * @calls{
         * int MPI_Comm_spawn(const char *command, char *argv[], int maxprocs, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *intercomm, int array_of_errcodes[]);    // exactly once
         * }
         */
        spawn_result_with_errcodes spawn_with_errcodes() {
            return this->spawn_impl<spawn_result_with_errcodes>();
        }
        ///@}


    private:
        /*
         * @brief Spawns a number of MPI processes according to the previously set options.
         * @details Removes empty values before getting passed to
         *          [*MPI_Comm_spawn*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node237.htm).
         * @tparam return_type either @ref mpicxx::spawn_result or @ref mpicxx::spawn_result_with_errcodes
         * @return the result of the spawn invocation
         *
         * @pre The executable name **must not** be empty.
         * @pre All command line arguments **must not** be empty.
         * @pre maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre root **must not** be less than `0` and greater or equal than the size of the communicator (set via
         *      @ref set_communicator(MPI_Comm) or default
         *      [*MPI_COMM_WORLD*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm)).
         * @pre comm **must not** be [*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm).
         *
         * @assert_precondition{ If the executable name is empty. \n
         *                       If any command line argument is empty. \n
         *                       If maxprocs is invalid. \n
         *                       If root isn't a legal root. \n
         *                       If comm is the null communicator
         *                       ([*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm)). }
         *
         * @calls{
         * int MPI_Comm_spawn(const char *command, char *argv[], int maxprocs, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *intercomm, int array_of_errcodes[]);    // exactly once
         * }
         */
        template <typename return_type>
        return_type spawn_impl() {
            MPICXX_ASSERT_PRECONDITION(this->legal_command(command_), "Attempt to use the executable name which is only an empty string!");
            MPICXX_ASSERT_PRECONDITION(this->legal_argv(argvs_).first,
                    "Attempt to use the {}-th command line argument which is only an empty string!", this->legal_argv(argvs_).second);
            MPICXX_ASSERT_PRECONDITION(this->legal_maxprocs(maxprocs_),
                    "Attempt to use the maxprocs value (which is {}), which falls outside the valid range (0, {}]!",
                    maxprocs_, mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));
            MPICXX_ASSERT_PRECONDITION(this->legal_root(root_, comm_),
                    "The previously set root '{}' isn't a valid root in the current communicator!", root_);
            MPICXX_ASSERT_PRECONDITION(this->legal_communicator(comm_), "Can't use the null communicator!");

            return_type res(maxprocs_);

            // determine whether the placeholder MPI_ERRCODES_IGNORE shall be used or a "real" std::vector
            auto errcode = [&res]() {
                if constexpr (std::is_same_v<return_type, spawn_result_with_errcodes>) {
                    return res.errcodes_.data();
                } else {
                    return MPI_ERRCODES_IGNORE;
                }
            }();

            if (argvs_.empty()) {
                // no additional arguments provided -> use MPI_ARGV_NULL
                MPI_Comm_spawn(command_.c_str(), MPI_ARGV_NULL, maxprocs_, info_.get(),
                               root_, comm_, &res.intercomm_, errcode);
            } else {
                // convert additional arguments to char**
                std::vector<char*> argvs_ptr;
                argvs_ptr.reserve(argvs_.size() + 1);
                for (auto& str : argvs_) {
                    argvs_ptr.emplace_back(str.data());
                }
                // add null termination
                argvs_ptr.emplace_back(nullptr);

                MPI_Comm_spawn(command_.c_str(), argvs_ptr.data(), maxprocs_, info_.get(),
                               root_, comm_, &res.intercomm_, errcode);
            }
            return res;
        }

#if MPICXX_ASSERTION_LEVEL > 0
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
         * @param[in] command the executable name
         * @return `true` if @p command is a executable valid name, `false` otherwise
         */
        bool legal_command(const std::string& command) const noexcept {
            return !command.empty();
        }
        /*
         * @brief Check whether @p argv is legal, i.e. it is **not** empty.
         * @param[in] argv the command line argument
         * @return `true` if @p key is valid, `false` otherwise
         */
        bool legal_argv(const std::string& argv) const noexcept {
            return !argv.empty();
        }
        /*
         * @brief Check whether all command line arguments in @p argvs are legal.
         * @param[in] argvs the list of command line arguments
         * @return `true` if all command line arguments in @p argvs are legal, `false` otherwise
         */
        std::pair<bool, std::size_t> legal_argv(const std::vector<std::string>& argvs) const noexcept {
            for (std::size_t i = 0; i < argvs.size(); ++i) {
                if (!this->legal_argv(argvs[i])) {
                    return std::make_pair(false, i);
                }
            }
            return std::make_pair(true, argvs.size());
        }
        /*
         * @brief Checks whether @p maxprocs is valid.
         * @details Checks whether @p maxprocs is greater than `0`. In addition, if the universe size could be queried, it's checked
         *          whether @p maxprocs is less or equal than the @ref mpicxx::universe_size().
         * @param[in] maxprocs the number of processes which should be spawned
         * @return `true` if @p maxprocs is legal, `false` otherwise
         */
        bool legal_maxprocs(const int maxprocs) const {
            std::optional<int> universe_size = mpicxx::universe_size();
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
         * @brief Checks whether @p comm is valid, i.e. it does **not** refer to
         *        [*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm).
         * @param[in] comm a intercommunicator
         * @return `true` if @p comm is valid, `false` otherwise
         */
        bool legal_communicator(const MPI_Comm comm) const noexcept {
            return comm != MPI_COMM_NULL;
        }
#endif

        std::string command_;
        std::vector<std::string> argvs_;
        int maxprocs_;
        info info_ = mpicxx::info::null;
        int root_ = 0;
        MPI_Comm comm_ = MPI_COMM_WORLD;
    };

    /// The default spawner is a @ref single_spawner.
    using spawner = single_spawner;

}

#endif // MPICXX_SINGLE_SPAWNER_HPP