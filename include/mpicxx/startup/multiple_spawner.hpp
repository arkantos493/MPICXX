/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-24
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Implements wrapper around the
 *        [*MPI_Comm_spawn_multiple*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node238.htm) function.
 */

#ifndef MPICXX_MULTIPLE_SPAWNER_HPP
#define MPICXX_MULTIPLE_SPAWNER_HPP

#include <mpicxx/detail/assert.hpp>
#include <mpicxx/detail/concepts.hpp>
#include <mpicxx/detail/conversion.hpp>
#include <mpicxx/detail/utility.hpp>
#include <mpicxx/info/info.hpp>
#include <mpicxx/info/runtime_info.hpp>
#include <mpicxx/startup/single_spawner.hpp>
#include <mpicxx/startup/spawn_result.hpp>

#include <fmt/format.h>
#include <mpi.h>

#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace mpicxx {

    // TODO 2020-05-17 23:36 breyerml: change from MPI_Comm to mpicxx equivalent -> copy/move constructor/assignment

    /**
     * @nosubgrouping
     * @brief Spawner class which enables to spawn (multiple) **different** MPI processes at runtime.
     */
    class multiple_spawner {
        // TODO 2020-05-13 00:21 breyerml: remove as soon as GCC bug has been fixed
        template <typename SpawnerType>
        static constexpr bool is_spawner_v = std::is_same_v<std::remove_cvref_t<SpawnerType>, single_spawner>
                || std::is_same_v<std::remove_cvref_t<SpawnerType>, multiple_spawner>;

    public:
        /// Unsigned integer type for argv size.
        using argv_size_type = std::size_t;
        /// Unsigned integer type.
        using size_type = std::size_t;


        // ---------------------------------------------------------------------------------------------------------- //
        //                                               constructor                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name constructor
        ///@{
        /**
         * @brief Constructs the @ref mpicxx::multiple_spawner object with the contents of the two ranges
         *        [@p first_commands, @p last_commands) and [@p first_maxprocs, @p last_maxprocs).
         * @tparam InputItCommands must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator)
         * @tparam InputItMaxprocs must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator)
         * @param[in] first_commands iterator to the first executable name in the first range
         * @param[in] last_commands iterator one-past the last executable name in the first range
         * @param[in] first_maxprocs iterator to the first number of maxprocs in the second range
         * @param[in] last_maxprocs iterator one-past the last number of maxprocs in the second range
         *
         * @pre @p first_commands and @p last_commands **must** refer to the same container.
         * @pre @p first_commands and @p last_commands **must** form a valid, non-empty range,
         *      i.e. @p first_commands must be strictly less than @p last_commands.
         * @pre @p first_maxprocs and @p last_maxprocs **must** refer to the same container.
         * @pre @p first_maxprocs and @p last_maxprocs **must** form a valid, non-empty range,
         *      i.e. @p first_maxprocs must be strictly less than @p last_maxprocs.
         * @pre The sizes of the two iterator ranges [@p first_commands, @p last_commands) and [@p first_maxprocs, @p last_maxprocs)
         *      **must** be equal.
         * @pre **Any** executable name **must not** be empty.
         * @pre **Any** maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         *
         * @assert_precondition{ If @p first_commands and @p last_commands don't denote a valid, non-empty iterator range. \n
         *                       If @p first_maxprocs and @p last_maxprocs don't denote a valid, non-empty iterator range. \n
         *                       If the sizes of the iterator ranges mismatch. }
         * @assert_sanity{ If any executable name is empty. \n
         *                 If any number of maxprocs is invalid. \n
         *                 If the total number of maxprocs is invalid. }
         */
        template <std::input_iterator InputItCommands, std::input_iterator InputItMaxprocs>
        multiple_spawner(InputItCommands first_commands, InputItCommands last_commands,
                         InputItMaxprocs first_maxprocs, InputItMaxprocs last_maxprocs)
        {
            MPICXX_ASSERT_PRECONDITION(this->legal_non_empty_iterator_range(first_commands, last_commands),
                    "Attempt to pass an illegal iterator range ('first_commands' must be strictly less than 'last_commands')!");
            MPICXX_ASSERT_PRECONDITION(this->legal_non_empty_iterator_range(first_maxprocs, last_maxprocs),
                    "Attempt to pass an illegal iterator range ('first_maxprocs' must be strictly less than 'last_maxprocs')!");
            MPICXX_ASSERT_PRECONDITION(this->legal_number_of_values(first_commands, last_commands, first_maxprocs, last_maxprocs),
                    "Attempt to pass two iterator ranges of different sizes (size of first range (which is {}) != size of second range (which is {}))!",
                    std::distance(first_commands, last_commands), std::distance(first_maxprocs, last_maxprocs));

            // set commands according to passed values
            size_ = std::distance(first_commands, last_commands);
            commands_.reserve(size_);
            for (; first_commands != last_commands; ++first_commands) {
                commands_.emplace_back(*first_commands);

                MPICXX_ASSERT_SANITY(this->legal_command(commands_.back()),
                        "Attempt to set the {}-th executable name to the empty string!",
                        size_ - std::distance(first_commands, last_commands));
            }

            // set maxprocs according to passed values
            const size_type maxprocs_size = std::distance(first_maxprocs, last_maxprocs);
            maxprocs_.reserve(maxprocs_size);
            for (; first_maxprocs != last_maxprocs; ++first_maxprocs) {
                maxprocs_.emplace_back(*first_maxprocs);

                MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs_.back()),
                        "Attempt to set the {}-th maxprocs value (which is {}), which falls outside the valid range (0, {}]!",
                        maxprocs_size - std::distance(first_maxprocs, last_maxprocs), maxprocs_.back(),
                        mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));
            }

            MPICXX_ASSERT_SANITY(this->legal_maxprocs(this->total_maxprocs()),
                    "Attempt to set the total number of maxprocs (which is: {} = {}), which falls outside the valid range (0, {}]!",
                    fmt::join(maxprocs_, " + "), this->total_maxprocs(),
                    mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));

            // set info objects to default values
            info_.assign(size_, mpicxx::info::null);
            // set command line arguments to default values
            argvs_.assign(size_, std::vector<std::string>());
        }
        /**
         * @brief Constructs the @ref mpicxx::multiple_spawner object with the contents of the two
         *        [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) lists
         *        @p ilist_commands and @p ilist_maxprocs.
         * @param[in] ilist_commands the list of executable names
         * @param[in] ilist_maxprocs the list of number of maxprocs
         *
         * @pre @p ilist_commands **must not** be empty.
         * @pre @p ilist_maxprocs **must not** be empty.
         * @pre **Any** executable name **must not** be empty.
         * @pre **Any** maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         *
         * @assert_precondition{ If @p ilist_commands is empty. \n
         *                       If @p ilist_maxprocs is empty. }
         * @assert_sanity{ If any executable name is empty. \n
         *                 If any number of maxprocs is invalid.\n
         *                 If the total number of maxprocs is invalid. }
         */
        multiple_spawner(std::initializer_list<std::string> ilist_commands, std::initializer_list<int> ilist_maxprocs)
            : multiple_spawner(ilist_commands.begin(), ilist_commands.end(), ilist_maxprocs.begin(), ilist_maxprocs.end()) { }
        /**
         * @brief Constructs the @ref mpicxx::multiple_spawner object with the contents of the range [@p first, @p last).
         * @tparam InputIt must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator)
         * @param[in] first iterator to the first pair in the range
         * @param[in] last iterator one-past the last pair in the range
         *
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid, non-empty range, i.e. @p first must be strictly less than @p last.
         * @pre **Any** executable name **must not** be empty.
         * @pre **Any** maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         *
         * @assert_precondition{ If @p first and @p last don't denote a valid, non-empty iterator range. }
         * @assert_sanity{ If any executable name is empty. \n
         *                 If any number of maxprocs is invalid. \n
         *                 If the total number of maxprocs is invalid. }
         */
        template <std::input_iterator InputIt>
        multiple_spawner(InputIt first, InputIt last) {
            MPICXX_ASSERT_PRECONDITION(this->legal_non_empty_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be strictly less than 'last')!");

            // set command and maxprocs according to passed values
            size_ = std::distance(first, last);
            commands_.reserve(size_);
            maxprocs_.reserve(size_);

            for (; first != last; ++first) {
                const auto& pair = *first;

                MPICXX_ASSERT_SANITY(this->legal_command(pair.first),
                        "Attempt to set the {}-th executable name to the empty string!", size_ - std::distance(first, last));
                MPICXX_ASSERT_SANITY(this->legal_maxprocs(pair.second),
                        "Attempt to set the {}-th maxprocs value (which is {}), which falls outside the valid range (0, {}]!",
                        size_ - std::distance(first, last), pair.second,
                        mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));

                commands_.emplace_back(pair.first);
                maxprocs_.emplace_back(pair.second);
            }

            MPICXX_ASSERT_SANITY(this->legal_maxprocs(this->total_maxprocs()),
                    "Attempt to set the total number of maxprocs (which is: {} = {}), which falls outside the valid range (0, {}]!",
                    fmt::join(maxprocs_, " + "), this->total_maxprocs(),
                    mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));

            // set info objects to default values
            info_.assign(size_, mpicxx::info::null);
            // set command line arguments to default values
            argvs_.assign(size_, std::vector<std::string>());
        }
        /**
         * @brief Constructs the @ref mpicxx::multiple_spawner object with the contents of the
         *        [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) list @p ilist.
         * @param[in] ilist the list of executable names and maxprocs pairs
         *
         * @pre @p ilist **must not** be empty.
         * @pre **Any** executable name **must not** be empty.
         * @pre **Any** maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         *
         * @assert_precondition{ If @p ilist is empty. }
         * @assert_sanity{ If any executable name is empty. \n
         *                 If any number of maxprocs is invalid.\n
         *                 If the total number of maxprocs is invalid. }
         */
        multiple_spawner(std::initializer_list<std::pair<std::string, int>> ilist) : multiple_spawner(ilist.begin(), ilist.end()) { }
        /**
         * @brief Constructs the @ref mpicxx::multiple_spawner object with the contents of the parameter pack @p args.
         * @tparam T an arbitrary number (but at least 1) of pairs meeting the @ref mpicxx::detail::is_pair requirements
         * @param[in] args the list of executable names and maxprocs pairs
         *
         * @pre **Any** executable name **must not** be empty.
         * @pre **Any** maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         *
         * @assert_sanity{ If any executable name is empty. \n
         *                 If any number of maxprocs is invalid. \n
         *                 If the total number of maxprocs is invalid. }
         */
        template <detail::is_pair... T>
        explicit multiple_spawner(T&&... args) requires (sizeof...(T) > 0) {
            // set command and maxprocs according to passed values
            size_ = sizeof...(T);
            commands_.reserve(size_);
            maxprocs_.reserve(size_);

            ([&] (auto&& arg) {

                MPICXX_ASSERT_SANITY(this->legal_command(arg.first), "Attempt to set an executable name to the empty string!");
                MPICXX_ASSERT_SANITY(this->legal_maxprocs(arg.second),
                        "Attempt to set the a maxprocs value (which is {}), which falls outside the valid range (0, {}]!",
                        arg.second, mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));

                using pair_t = decltype(arg);
                commands_.emplace_back(std::forward<pair_t>(arg).first);
                maxprocs_.emplace_back(std::forward<pair_t>(arg).second);
            }(std::forward<T>(args)), ...);

            MPICXX_ASSERT_SANITY(this->legal_maxprocs(this->total_maxprocs()),
                    "Attempt to set the total number of maxprocs (which is: {} = {}), which falls outside the valid range (0, {}]!",
                    fmt::join(maxprocs_, " + "), this->total_maxprocs(),
                    mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));

            // set info objects to default values
            info_.assign(size_, mpicxx::info::null);
            // set command line arguments to default values
            argvs_.assign(size_, std::vector<std::string>());
        }
        // TODO 2020-05-11 22:58 breyerml: change to c++20 concepts syntax as soon as GCC bug has been fixed
        /**
         * @brief Constructs the @ref mpicxx::multiple_spawner object with the spawner object(s) of the parameter pack @p args.
         * @tparam T an arbitrary number (but at least 1) of spawners meeting the @ref mpicxx::detail::is_spawner requirements
         * @param[in] args the spawners which should get merged into this @ref mpicxx::multiple_spawner
         *
         * @pre **All** roots **must** be equal.
         * @pre **All** communicators **must** be equal.
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         *
         * @assert_precondition{ If not all roots are equivalent. \n
         *                       If not all communicators are equivalent. }
         * @assert_sanity{ If the total number of maxprocs is invalid. }
         */
        template <typename... T, std::enable_if_t<(is_spawner_v<T> && ...), int> = 0>
        explicit multiple_spawner(T&&... args) requires (sizeof...(T) > 0) {
            MPICXX_ASSERT_PRECONDITION(detail::all_same([](const auto& arg) { return arg.root(); }, args...),
                    "Attempt to use different root processes!");
            MPICXX_ASSERT_PRECONDITION(detail::all_same([](const auto& arg) { return arg.communicator(); }, args...),
                    "Attempt to use different communicators!");

            ([&] (auto&& arg) {
                using spawner_t = decltype(arg);
                if constexpr (std::is_same_v<std::remove_cvref_t<spawner_t>, single_spawner>) {
                    ++size_;
                    commands_.emplace_back(std::forward<spawner_t>(arg).command());
                    argvs_.emplace_back(std::forward<spawner_t>(arg).argv());
                    maxprocs_.emplace_back(std::forward<spawner_t>(arg).maxprocs());
                    info_.emplace_back(std::forward<spawner_t>(arg).spawn_info());
                } else if constexpr (std::is_same_v<std::remove_cvref_t<spawner_t>, multiple_spawner>) {
                    size_ += arg.size();
                    for (multiple_spawner::size_type i = 0; i < arg.size(); ++i) {
                        commands_.emplace_back(std::forward<spawner_t>(arg).command_at(i));
                        argvs_.emplace_back(std::forward<spawner_t>(arg).argv_at(i));
                        maxprocs_.emplace_back(std::forward<spawner_t>(arg).maxprocs_at(i));
                        info_.emplace_back(std::forward<spawner_t>(arg).spawn_info_at(i));
                    }
                }
                root_ = arg.root();
                comm_ = arg.communicator();
            }(std::forward<T>(args)), ...);

            MPICXX_ASSERT_SANITY(this->legal_maxprocs(this->total_maxprocs()),
                     "Attempt to set the total number of maxprocs (which is: {} = {}), which falls outside the valid range (0, {}]!",
                     fmt::join(maxprocs_, " + "), this->total_maxprocs(),
                     mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                          modify spawn information                                          //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name modify spawn information
        ///@{
        /**
         * @brief Replaces the old executable names with the new names from the range [@p first, @p last).
         * @tparam InputIt must meet [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) requirements
         * @param[in] first iterator to the first executable name in the range
         * @param[in] last iterator one-past the last executable name in the range
         * @return `*this`
         *
         * @pre @p first and @p last **must** refer to the same container.
         * @pre The size of the range [@p first, @p last) **must** match the size of the this @ref mpicxx::multiple_spawner and thus
         *      must be legal.
         * @pre All executable names in the range [@p first, @p last) **must not** be empty.
         *
         * @assert_precondition{ If @p first and @p last don't denote a valid iterator range. }
         * @assert_sanity{ If the sizes mismatch. \n
         *                 If any new executable name is empty. }
         */
        template <std::input_iterator InputIt>
        multiple_spawner& set_command(InputIt first, InputIt last) requires (!detail::is_c_string<InputIt>) {
            MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(first, last),
                    "Illegal number of values: std::distance(first, last) (which is {}) != this->size() (which is {})",
                    std::distance(first, last), this->size());

            commands_.assign(first, last);

            MPICXX_ASSERT_SANITY(this->legal_command(commands_).first,
                    "Attempt to set the {}-th executable name to the empty string!", this->legal_command(commands_).second);

            return *this;
        }
        /**
         * @brief Replaces the old executable names with the new names from the
         *        [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) @p ilist.
         * @param[in] ilist the new executable names
         * @return `*this`
         *
         * @pre The size of @p ilist **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre All executable names in @p ilist **must not** be empty.
         *
         * @assert_sanity{ If the sizes mismatch. \n
         *                 If any new executable name is empty. }
         */
        multiple_spawner& set_command(std::initializer_list<std::string> ilist) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(ilist),
                    "Illegal number of values: ilist.size() (which is {}) != this->size() (which is {})",
                    ilist.size(), this->size());

            commands_.assign(ilist);

            MPICXX_ASSERT_SANITY(this->legal_command(commands_).first,
                    "Attempt to set the {}-th executable name to the empty string!", this->legal_command(commands_).second);

            return *this;
        }
        /**
         * @brief Replaces the old executable names with the new names from the parameter pack @p args.
         * @tparam T an arbitrary number (but at least 1) of string like objects meeting @p mpicxx::detail::is_string requirements
         * @param[in] args the new executable names
         * @return `*this`
         *
         * @pre The size of the parameter pack @p args **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre All executable names in the parameter pack @p args **must not** be empty.
         *
         * @assert_sanity{ If the sizes mismatch. \n
         *                 If any new executable name is empty. }
         */
        template <detail::is_string... T>
        multiple_spawner& set_command(T&&... args) requires (sizeof...(T) > 0) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(args...),
                    "Illegal number of values: sizeof...(T) (which is {}) != this->size() (which is {})", sizeof...(T), this->size());

            commands_.clear();
            (commands_.emplace_back(std::forward<T>(args)), ...);

            MPICXX_ASSERT_SANITY(this->legal_command(commands_).first,
                    "Attempt to set the {}-th executable name to the empty string!", this->legal_command(commands_).second);

            return *this;
        }
        /**
         * @brief Change the @p i-th executable name to @p name.
         * @tparam T must meet the @p mpicxx::detail::is_string requirements
         * @param[in] i the index of the executable name
         * @param[in] name the new name of the @p i-th executable
         * @return `*this`
         *
         * @pre @p i **must not** be greater than `this->size()`.
         * @pre @p name **must not** be empty.
         *
         * @assert_sanity{ If @p name is empty. }
         *
         * @throws std::out_of_range if the index @p i falls outside the valid range
         */
        template <detail::is_string T>
        multiple_spawner& set_command_at(const std::size_t i, T&& name) {
            if (i >= this->size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::set_command_at(const std::size_t, T&&) range check: i (which is {}) >= this->size() (which is {})",
                        i, this->size()));
            }

            commands_[i] = std::forward<T>(name);

            MPICXX_ASSERT_SANITY(this->legal_command(commands_[i]), "Attempt to set the {}-th executable name to the empty string!", i);

            return *this;
        }

        /**
         * @brief Adds all command line arguments in the range [@p first, @p last) to the respective executable.
         * @details `std::begin(*first)` and `std::end(*first)` must be valid statements (which holds e.g. for a two-dimensional
         *          [`std::vector`](https://en.cppreference.com/w/cpp/container/vector)).
         *
         *          Example: @snippet examples/startup/multiple_spawner.cpp add_argv version with iterator range
         * @tparam InputIt must meet the [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) requirements
         * @param[in] first iterator to the first command line arguments list in the range
         * @param[in] last iterator one-past the last command line arguments list in the range
         * @return `*this`
         *
         * @pre @p first and @p last **must** refer to the same container.
         * @pre The size of the range [@p first, @p last) **must** match the size of this @ref mpicxx::multiple_spawner
         *      and thus **must** be legal.
         * @pre All command line arguments **must not** be empty.
         *
         * @assert_precondition{ If @p first and @p last don't denote a valid iterator range. }
         * @assert_sanity{ If the sizes mismatch. \n
         *                 If any command line argument is empty. }
         */
        template <std::input_iterator InputIt>
        multiple_spawner& add_argv(InputIt first, InputIt last) {
            MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(first, last),
                    "Illegal number of values: std::distance(first, last) (which is {}) != this->size() (which is {})",
                    std::distance(first, last), this->size());

            for (std::size_t i = 0; first != last; ++first) {
                const auto& container = *first;
                this->add_argv_at(i++, std::begin(container), std::end(container));
            }
            return *this;
        }
        /**
         * @brief Adds all command line arguments of the
         *        [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) @p ilist to the respective
         *        executable.
         * @details Example: @snippet examples/startup/multiple_spawner.cpp add_argv version with initializer_list
         * @tparam T must be convertible to [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string)
         *           via @ref mpicxx::detail::convert_to_string
         * @param[in] ilist the lists of the (additional) command line arguments
         * @return `*this`
         *
         * @pre The size of @p ilist **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre All command line arguments **must not** be empty.
         *
         * @assert_sanity{ If the sizes mismatch. \n
         *                 If any command line argument is empty. }
         */
        template <typename T = std::string>
        multiple_spawner& add_argv(std::initializer_list<std::initializer_list<T>> ilist) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(ilist),
                    "Illegal number of values: ilist.size() (which is {}) != this->size() (which is {})",
                    ilist.size(), this->size());

            for (std::size_t i = 0; const auto l : ilist) {
                this->add_argv_at(i++, l);
            }
            return *this;
        }
        /**
         * @brief Adds all command line arguments of the parameter pack @p args to the respective executable.
         * @details Example: @snippet examples/startup/multiple_spawner.cpp add_argv version with parameter pack
         * @tparam T must be a container type or C-style array type and must not be empty
         * @param[in] args the lists of the (additional) command line arguments
         * @return `*this`
         *
         * @pre The size of the parameter pack @p args **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre All command line arguments **must not** be empty.
         *
         * @assert_sanity{ If the sizes mismatch. \n
         *                 If any command line argument is empty. }
         */
        template <typename... T>
        multiple_spawner& add_argv(T&&... args) requires (sizeof...(T) > 0) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(args...),
                    "Illegal number of values: sizeof...(T) (which is {}) != this->size() (which is {})",
                    sizeof...(T), this->size());

            std::size_t i = 0;
            ([&](auto&& arg) {
                this->add_argv_at(i++, std::begin(arg), std::end(arg));
            }(std::forward<T>(args)), ...);
            return *this;
        }

        /**
         * @brief Adds all command line arguments in the range [@p first, @p last) to the @p i-th executable.
         * @details Example: @snippet examples/startup/multiple_spawner.cpp add_argv_at version with iterator range
         * @tparam InputIt must meet the [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) requirements.
         * @param[in] i the index of the executable
         * @param[in] first iterator to the first command line argument in the range
         * @param[in] last iterator one-past the last command line argument in the range
         * @return `*this`
         *
         * @pre @p i **must not** be greater than `this->size()`.
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid range, i.e. @p first must be less or equal than @p last.
         * @pre All command line arguments **must not** be empty.
         *
         * @assert_precondition{ If @p first and @p last don't denote a valid iterator range. }
         * @assert_sanity{ If any command line argument is empty. }
         *
         * @throws std::out_of_range if the index @p i falls outside the valid range
         */
        template <std::input_iterator InputIt>
        multiple_spawner& add_argv_at(const std::size_t i, InputIt first, InputIt last) requires (!detail::is_c_string<InputIt>) {
            MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");

            for (; first != last; ++first) {
                this->add_argv_at(i, *first);
            }
            return *this;
        }
        /**
         * @brief Adds all command line arguments in the
         *        [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) @p ilist to the @p i-th executable.
         * @details Example: @snippet examples/startup/multiple_spawner.cpp add_argv_at version with initializer_list
         * @tparam T must be convertible to [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string)
         *           via @ref mpicxx::detail::convert_to_string
         * @param[in] i the index of the executable
         * @param[in] ilist the (additional) command line arguments
         * @return `*this`
         *
         * @pre @p i **must not** be greater than `this->size()`.
         * @pre All command line arguments **must not** be empty.
         *
         * @assert_sanity{ If any command line argument is empty. }
         *
         * @throws std::out_of_range if the index @p i falls outside the valid range
         */
        template <typename T = std::string>
        multiple_spawner& add_argv_at(const std::size_t i, std::initializer_list<T> ilist) {
            for (const auto& val : ilist) {
                this->add_argv_at(i, val);
            }
            return *this;
        }
        /**
         * @brief Adds all command line arguments in the parameter pack @p args to the @p i-th executable.
         * @details Example: @snippet examples/startup/multiple_spawner.cpp add_argv_at version with parameter pack
         * @tparam T must be convertible to [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string)
         *           via @ref mpicxx::detail::convert_to_string and must not be empty
         * @param[in] i the index of the executable
         * @param[in] args the (additional) command line arguments
         * @return `*this`
         *
         * @pre @p i **must not** be greater than `this->size()`.
         * @pre All command line arguments in @p args **must not** be empty.
         *
         * @assert_sanity{ If any command line argument is empty. }
         *
         * @throws std::out_of_range if the index @p i falls outside the valid range
         */
        template <typename... T>
        multiple_spawner& add_argv_at(const std::size_t i, T&&... args) requires (sizeof...(T) > 0) {
            if (i >= this->size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::add_argv_at(const std::size_t, T&&) range check: i (which is {}) >= this->size() (which is {})",
                        i, this->size()));
            }

            ([&](auto&& arg) {
                // convert argument to a std::string
                std::string argv = detail::convert_to_string(std::forward<T>(arg));

                MPICXX_ASSERT_SANITY(this->legal_argv(argv), "Attempt to set an empty command line argument!");

                // add command line argument at position i
                argvs_[i].emplace_back(std::move(argv));
            }(std::forward<T>(args)), ...);
            return *this;
        }

        /**
         * @brief Removes all command line arguments.
         * @return `*this`
         */
        multiple_spawner& remove_argv() {
            std::vector<std::vector<std::string>>(this->size()).swap(argvs_);
            return *this;
        }
        /**
         * @brief Removes all command line arguments of the @p i-th executable.
         * @param[in] i the index of the executable
         * @return `*this`
         *
         * @pre @p i **must not** be greater than `this->size()`.
         *
         * @throws std::out_of_range if the index @p i falls outside the valid range
         */
        multiple_spawner& remove_argv_at(const std::size_t i) {
            if (i >= this->size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::remove_argv_at(const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                        i, this->size()));
            }

            std::vector<std::string>().swap(argvs_[i]);
            return *this;
        }
        /**
         * @brief Removes the @p j-th command line argument of the @p i-th executable.
         * @param[in] i the index of the executable
         * @param[in] j the index of the command line argument
         * @return `*this`
         *
         * @pre @p i **must not** be greater than `this->size()`.
         * @pre @p j **must not** be greater than `this->argv_size_at(i)`.
         *
         * @throws std::out_of_range if the indices @p i or @p j fall outside their valid range
         */
        multiple_spawner& remove_argv_at(const std::size_t i, const std::size_t j) {
            if (i >= this->size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::remove_argv_at(const std::size_t, const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                        i, this->size()));
            }
            if (j >= argvs_[i].size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::remove_argv_at(const std::size_t, const std::size_t) range check: j (which is {}) >= argvs_[{}].size() (which is {})",
                        j, i, argvs_[i].size()));
            }

            argvs_[i].erase(argvs_[i].begin() + j);
            return *this;
        }


        /**
         * @brief Replaces the old number of processes with the new numbers from the range [@p first, @p last).
         * @tparam InputIt must meet [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) requirements
         * @param[in] first iterator to the first number of processes in the range
         * @param[in] last iterator one-past the last number of processes in the range
         * @return `*this`
         *
         * @pre @p first and @p last **must** refer to the same container.
         * @pre The size of the range [@p first, @p last) **must** match the size of this @ref mpicxx::multiple_spawner and thus
         *      must be legal.
         * @pre **Any** maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         *
         * @assert_precondition{ If @p first and @p last don't denote a valid iterator range. }
         * @assert_sanity{ If the sizes mismatch. \n
         *                 If any number of maxprocs is invalid. \n
         *                 If the total number of maxprocs is invalid. }
         */
        template <std::input_iterator InputIt>
        multiple_spawner& set_maxprocs(InputIt first, InputIt last) {
            MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(first, last),
                    "Illegal number of values: std::distance(first, last) (which is {}) != this->size() (which is {})",
                    std::distance(first, last), this->size());

            maxprocs_.assign(first, last);

            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs_).first,
                    "Attempt to set the {}-th maxprocs value (which is {}), which falls outside the valid range (0, {}]!",
                    this->legal_maxprocs(maxprocs_).second, maxprocs_[this->legal_maxprocs(maxprocs_).second],
                    mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));
            MPICXX_ASSERT_SANITY(this->legal_maxprocs(this->total_maxprocs()),
                    "Attempt to set the total number of maxprocs (which is: {} = {}), which falls outside the valid range (0, {}]!",
                    fmt::join(maxprocs_, " + "), this->total_maxprocs(),
                    mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));

            return *this;
        }
        /**
         * @brief Replaces the old number of processes with the new numbers from the
         *        [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) @p ilist.
         * @param[in] ilist the new number of processes
         * @return `*this`
         *
         * @pre The size of @p ilist **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre **Any** maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         *
         * @assert_sanity{ If the sizes mismatch. \n
         *                 If any number of maxprocs is invalid. \n
         *                 If the total number of maxprocs is invalid. }
         */
        multiple_spawner& set_maxprocs(std::initializer_list<int> ilist) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(ilist),
                     "Illegal number of values: ilist.size() (which is {}) != this->size() (which is {})",
                     ilist.size(), this->size());

            maxprocs_.assign(ilist);

            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs_).first,
                     "Attempt to set the {}-th maxprocs value (which is {}), which falls outside the valid range (0, {}]!",
                     this->legal_maxprocs(maxprocs_).second, maxprocs_[this->legal_maxprocs(maxprocs_).second],
                     mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));
            MPICXX_ASSERT_SANITY(this->legal_maxprocs(this->total_maxprocs()),
                     "Attempt to set the total number of maxprocs (which is: {} = {}), which falls outside the valid range (0, {}]!",
                     fmt::join(maxprocs_, " + "), this->total_maxprocs(),
                     mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));

            return *this;
        }
        /**
         * @brief Replaces the old number of processes with the new numbers from the parameter pack @p args.
         * @tparam T an arbitrary number (but at least 1) of integral objects meeting the
         *           [`std::integral`](https://en.cppreference.com/w/cpp/concepts/integral) requirements
         * @param[in] args the parameter pack containing the new number of processes
         * @return `*this`
         *
         * @pre The size of the parameter pack @p args **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre **Any** maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         *
         * @assert_sanity{ If the sizes mismatch. \n
         *                 If any number of maxprocs is invalid. \n
         *                 If the total number of maxprocs is invalid. }
         */
        template <std::integral... T>
        multiple_spawner& set_maxprocs(T... args) requires (sizeof...(T) > 0) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(args...),
                     "Illegal number of values: sizeof...(T) (which is {}) != this->size() (which is {})", sizeof...(T), this->size());

            maxprocs_.clear();
            (maxprocs_.emplace_back(std::forward<T>(args)), ...);

            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs_).first,
                     "Attempt to set the {}-th maxprocs value (which is {}), which falls outside the valid range (0, {}]!",
                     this->legal_maxprocs(maxprocs_).second, maxprocs_[this->legal_maxprocs(maxprocs_).second],
                     mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));
            MPICXX_ASSERT_SANITY(this->legal_maxprocs(this->total_maxprocs()),
                     "Attempt to set the total number of maxprocs (which is: {} = {}), which falls outside the valid range (0, {}]!",
                     fmt::join(maxprocs_, " + "), this->total_maxprocs(),
                     mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));

            return *this;
        }
        /**
         * @brief Change the @p i-th number of processes to @p maxprocs.
         * @param[in] i the index of the executable
         * @param[in] maxprocs the new maximum number of processes
         * @return `*this`
         *
         * @pre @p i **must not** be greater than `this->size()`.
         * @pre @p maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         *
         * @assert_sanity{ If @p maxprocs is invalid. \n
         *                 If the total number of maxprocs is invalid. }
         *
         * @throws std::out_of_range if the index @p i falls outside the valid range
         */
        multiple_spawner& set_maxprocs_at(const std::size_t i, const int maxprocs) {
            if (i >= this->size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::set_maxprocs_at(const std::size_t, const int) range check: i (which is {}) >= this->size() (which is {})",
                        i, this->size()));
            }

            maxprocs_[i] = maxprocs;

            MPICXX_ASSERT_SANITY(this->legal_maxprocs(maxprocs_).first,
                     "Attempt to set the {}-th maxprocs value (which is {}), which falls outside the valid range (0, {}]!",
                     this->legal_maxprocs(maxprocs_).second, maxprocs_[this->legal_maxprocs(maxprocs_).second],
                     mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));
            MPICXX_ASSERT_SANITY(this->legal_maxprocs(this->total_maxprocs()),
                     "Attempt to set the total number of maxprocs (which is: {} = {}), which falls outside the valid range (0, {}]!",
                     fmt::join(maxprocs_, " + "), this->total_maxprocs(),
                     mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));

            return *this;
        }


        /**
         * @brief Replaces the old spawn info with the new info from the range [@p first, @p last).
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
         * @tparam InputIt must meet [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) requirements
         * @param[in] first iterator to the first spawn info in the range
         * @param[in] last iterator one-past the last spawn info in the range
         * @return `*this`
         *
         * @pre @p first and @p last **must** refer to the same container.
         * @pre The size of the range [@p first, @p last) **must** match the size of this @ref mpicxx::multiple_spawner and thus
         *      must be legal.
         *
         * @assert_precondition{ If @p first and @p last don't denote a valid iterator range. }
         * @assert_sanity{ If the sizes mismatch. }
         */
        template <std::input_iterator InputIt>
        multiple_spawner& set_spawn_info(InputIt first, InputIt last) {
            MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(first, last),
                    "Illegal number of values: std::distance(first, last) (which is {}) != this->size() (which is {})",
                    std::distance(first, last), this->size());

            info_.assign(first, last);
            return *this;
        }
        /**
         * @brief Replaces the old spawn info with the new info from the
         *        [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) @p ilist.
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
         * @param[in] ilist the new spawn info
         * @return `*this`
         *
         * @pre The size of @p ilist **must** match the size of this @ref mpicxx::multiple_spawner.
         *
         * @assert_sanity{ If the sizes mismatch. }
         */
        multiple_spawner& set_spawn_info(std::initializer_list<info> ilist) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(ilist),
                    "Illegal number of values: ilist.size() (which is {}) != this->size() (which is {})",
                    ilist.size(), this->size());

            info_.assign(ilist);
            return *this;
        }
        /**
         * @brief Replaces the old spawn info with the new info from the parameter pack @p args.
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
         * @tparam T an arbitrary number (but at least 1) of @ref mpicxx::info objects meeting the @ref mpicxx::detail::is_info requirements
         * @param[in] args the new spawn info
         * @return `*this`
         *
         * @pre The size of the parameter pack @p args **must** match the size of this @ref mpicxx::multiple_spawner.
         *
         * @assert_sanity{ If the sizes mismatch. }
         */
        template <detail::is_info... T>
        multiple_spawner& set_spawn_info(T&&... args) requires (sizeof...(T) > 0) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(args...),
                    "Illegal number of values: sizeof...(T) (which is {}) != this->size() (which is {})", sizeof...(T), this->size());

            info_.clear();
            (info_.emplace_back(std::forward<T>(args)), ...);
            return *this;
        }
        /**
         * @brief Change the @p i-th spawn info to @p spawn_info.
         * @param[in] i the index of the executable
         * @param[in] spawn_info the new spawn info
         * @return `*this`
         *
         * @pre @p i **must not** be greater than `this->size()`.
         *
         * @throws std::out_of_range if the index @p i falls outside the valid range
         */
        multiple_spawner& set_spawn_info_at(const std::size_t i, info spawn_info) {
            if (i >= this->size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::set_spawn_info_at(const std::size_t, info) range check: i (which is {}) >= this->size() (which is {})",
                        i, this->size()));
            }

            info_[i] = std::move(spawn_info);
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
        multiple_spawner& set_root(const int root) noexcept {
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
        multiple_spawner& set_communicator(MPI_Comm comm) noexcept {
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
         * @brief Returns all executable names.
         * @return the executable names
         * @nodiscard
         */
        [[nodiscard]]
        const std::vector<std::string>& command() const noexcept { return commands_; }
        /**
         * @brief Returns the name of the @p i-th executable.
         * @param[in] i the index of the executable
         * @return the @p i-th executable name
         * @nodiscard
         *
         * @pre @p i **must not** be greater than `this->size()`.
         *
         * @throws std::out_of_range if the index @p i falls outside the valid range
         */
        [[nodiscard]]
        const std::string& command_at(const std::size_t i) const {
            if (i >= this->size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::command_at(const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                        i, this->size()));
            }

            return commands_[i];
        }

        /**
         * @brief Returns all added command line arguments.
         * @return the command line arguments of all executables
         * @nodiscard
         */
        [[nodiscard]]
        const std::vector<std::vector<std::string>>& argv() const noexcept { return argvs_; }
        /**
         * @brief Returns all added command line arguments of the @p i-th executable.
         * @param[in] i the index of the executable
         * @return the command line arguments of the @p i-th executable
         * @nodiscard
         *
         * @pre @p i **must not** be greater than `this->size()`.
         *
         * @throws std::out_of_range if the index @p i falls outside the valid range
         */
        [[nodiscard]]
        const std::vector<std::string>& argv_at(const std::size_t i) const {
            if (i >= this->size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::argv_at(const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                        i, this->size()));
            }

            return argvs_[i];
        }
        /**
         * @brief Returns the @p j-th command line argument of the @p i-th executable.
         * @param[in] i the index of the executable
         * @param[in] j the index of the command line argument
         * @return the @p j-th command line argument of the @p i-th executable
         * @nodiscard
         *
         * @pre @p i **must not** be greater than `this->size()`.
         * @pre @p j **must not** be greater than `this->argv_size_at(i)`.
         *
         * @throws std::out_of_range if the indices @p i or @p j fall outside their valid ranges
         */
        [[nodiscard]]
        const std::string& argv_at(const std::size_t i, const std::size_t j) const {
            if (i >= this->size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::argv_at(const std::size_t, const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                        i, this->size()));
            }
            if (j >= argvs_[i].size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::argv_at(const std::size_t, const std::size_t) range check: j (which is {}) >= argvs_[{}].size() (which is {})",
                        j, i, argvs_[i].size()));
            }

            return argvs_[i][j];
        }
        /**
         * @brief Returns the number of added command line arguments per executable.
         * @return the number of command line arguments per executable
         * @nodiscard
         *
         * @note Creates a new [`std::vector`](https://en.cppreference.com/w/cpp/container/vector) on each invocation.
         */
        [[nodiscard]]
        std::vector<argv_size_type> argv_size() const {
            std::vector<argv_size_type> sizes(this->size());
            std::transform(argvs_.cbegin(), argvs_.cend(), sizes.begin(), [](const auto& v) { return v.size(); });
            return sizes;
        }
        /**
         * @brief Returns the number of added command line arguments of the @p i-th executable.
         * @param[in] i the index of the executable
         * @return the number of command line arguments of the @p i-th executable
         * @nodiscard
         *
         * @pre @p i **must not** be greater than `this->size()`.
         *
         * @throws std::out_of_range if the index @p i falls outside the valid range
         */
        [[nodiscard]]
        argv_size_type argv_size_at(const std::size_t i) const {
            if (i >= this->size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::argv_size_at(const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                        i, this->size()));
            }

            return argvs_[i].size();
        }

        /**
         * @brief Returns all numbers of processes.
         * @return the number of processes
         * @nodiscard
         */
        [[nodiscard]]
        const std::vector<int>& maxprocs() const noexcept { return maxprocs_; }
        /**
         * @brief Returns the @p i-th number of processes.
         * @param[in] i the index of the executable
         * @return the @p i-th number of processes
         * @nodiscard
         *
         * @pre @p i **must not** be greater than `this->size()`.
         *
         * @throws std::out_of_range if the index @p i falls outside the valid range
         */
        [[nodiscard]]
        int maxprocs_at(const std::size_t i) const {
            if (i >= this->size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::maxprocs_at(const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                        i, this->size()));
            }

            return maxprocs_[i];
        }

        /**
         * @brief Returns all spawn info.
         * @return the info objects used to spawn the executables
         * @nodiscard
         */
        [[nodiscard]]
        const std::vector<info>& spawn_info() const noexcept { return info_; }
        /**
         * @brief Returns the @p i-th spawn info used to spawn the executables.
         * @param[in] i the index of the executable
         * @return the @p i-th spawn info
         * @nodiscard
         *
         * @pre @p i **must not** be greater than `this->size()`.
         *
         * @throws std::out_of_range if the index @p i falls outside the valid range
         */
        [[nodiscard]]
        const info& spawn_info_at(const std::size_t i) const {
            if (i >= this->size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::spawn_info_at(const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                        i, this->size()));
            }

            return info_[i];
        }

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

        /**
         * @brief Returns the size of this @ref mpicxx::multiple_spawner object, i.e. the number of spawned executables
         *        (**not** the total number of processes to spawn).
         * @return the size if this @ref mpicxx::multiple_spawner
         * @nodiscard
         */
        [[nodiscard]]
        size_type size() const noexcept {
            MPICXX_ASSERT_SANITY(detail::all_same([](const auto& vec) { return vec.size(); }, commands_, argvs_, maxprocs_, info_),
                    "Attempt to retrieve the size while the sizes of the members (commands = {}, argvs = {}, maxprocs = {}, info = {}) differ!",
                    commands_.size(), argvs_.size(), maxprocs_.size(), info_.size());

            return size_;
        }
        /**
         * @brief Returns the total number of process that will get spawned.
         * @return the total number of processes
         * @nodiscard
         */
        [[nodiscard]]
        int total_maxprocs() const noexcept {
            // custom implementation because std::reduce/std::accumulate aren't noexcept
            int total = 0;
            for (const int i : maxprocs_) {
                total += i;
            }
            return total;
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            spawn new process(es)                                           //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name spawn new process(es)
        ///@{
        /**
         * @brief Spawns a number of MPI processes associated with multiple executables according to the previously set options.
         * @details The returned @ref mpicxx::spawn_result object **only** contains the intercommunicator.
         *
         *    Example: @snippet examples/startup/multiple_spawner.cpp spawn without error codes
         * @return the result of the spawn invocation
         *
         * @pre The number of executables **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre All executable name **must not** be empty.
         * @pre The number of command line argument lists **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre All command line arguments **must not** be empty.
         * @pre The number of maxprocs **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre All maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The number of spawn info **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre root **must not** be less than `0` and greater or equal than the size of the communicator (set via
         *      @ref set_communicator(MPI_Comm) or default
         *      [*MPI_COMM_WORLD*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm)).
         * @pre comm **must not** be [*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm).
         *
         * @assert_precondition{ If **any** size mismatches. \n
         *                       If any executable name is empty. \n
         *                       If any command line argument is empty. \n
         *                       If any number of maxprocs is invalid. \n
         *                       If the total number of maxprocs is invalid. \n
         *                       If root isn't a legal root. \n
         *                       If comm is the null communicator
         *                       ([*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm)). }
         *
         * @calls{
         * int MPI_Comm_spawn_multiple(int count, char *array_of_commands[], char **array_of_argv[], const int array_of_maxprocs[], const MPI_Info array_of_info[], int root, MPI_Comm comm, MPI_Comm *intercomm, int array_of_errcodes[]);    // exactly once
         * }
         */
        spawn_result spawn() {
            return this->spawn_impl<spawn_result>();
        }
        /**
         * @brief Spawns a number of MPI processes associated with multiple executables according to the previously set options.
         * @details The returned @ref mpicxx::spawn_result_with_errcodes object contains the intercommunicator **and** information about the
         *          possibly occurring error codes.
         *
         *    Example: @snippet examples/startup/multiple_spawner.cpp spawn with error codes
         * @return the result of the spawn invocation
         *
         * @pre The number of executables **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre All executable name **must not** be empty.
         * @pre The number of command line argument lists **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre All command line arguments **must not** be empty.
         * @pre The number of maxprocs **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre All maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The number of spawn info **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre root **must not** be less than `0` and greater or equal than the size of the communicator (set via
         *      @ref set_communicator(MPI_Comm) or default
         *      [*MPI_COMM_WORLD*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm)).
         * @pre comm **must not** be [*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm).
         *
         * @assert_precondition{ If **any** size mismatches. \n
         *                       If any executable name is empty. \n
         *                       If any command line argument is empty. \n
         *                       If any number of maxprocs is invalid. \n
         *                       If the total number of maxprocs is invalid. \n
         *                       If root isn't a legal root. \n
         *                       If comm is the null communicator
         *                       ([*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm)). }
         *
         * @calls{
         * int MPI_Comm_spawn_multiple(int count, char *array_of_commands[], char **array_of_argv[], const int array_of_maxprocs[], const MPI_Info array_of_info[], int root, MPI_Comm comm, MPI_Comm *intercomm, int array_of_errcodes[]);    // exactly once
         * }
         */
        spawn_result_with_errcodes spawn_with_errcodes() {
            return this->spawn_impl<spawn_result_with_errcodes>();
        }
        ///@}


    private:
        /*
         * @brief Spawns a number of MPI processes associated with multiple executables according to the previously set options.
         * @tparam return_type either @ref mpicxx::spawn_result or @ref mpicxx::spawn_result_with_errcodes
         * @return the result of the spawn invocation
         *
         * @pre The number of executables **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre All executable name **must not** be empty.
         * @pre The number of command line argument lists **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre All command line arguments **must not** be empty.
         * @pre The number of maxprocs **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre All maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         *      (@ref mpicxx::universe_size()).
         * @pre The number of spawn info **must** match the size of this @ref mpicxx::multiple_spawner.
         * @pre root **must not** be less than `0` and greater or equal than the size of the communicator (set via
         *      @ref set_communicator(MPI_Comm) or default
         *      [*MPI_COMM_WORLD*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm)).
         * @pre comm **must not** be [*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm).
         *
         * @assert_precondition{ If **any** size mismatches. \n
         *                       If any executable name is empty. \n
         *                       If any command line argument is empty. \n
         *                       If any number of maxprocs is invalid. \n
         *                       If the total number of maxprocs is invalid. \n
         *                       If root isn't a legal root. \n
         *                       If comm is the null communicator
         *                       ([*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm)). }
         *
         * @calls{
         * int MPI_Comm_spawn_multiple(int count, char *array_of_commands[], char **array_of_argv[], const int array_of_maxprocs[], const MPI_Info array_of_info[], int root, MPI_Comm comm, MPI_Comm *intercomm, int array_of_errcodes[]);    // exactly once
         * }
         */
        template <typename return_type>
        return_type spawn_impl() {
            MPICXX_ASSERT_PRECONDITION(this->legal_number_of_values(commands_),
                    "Illegal number of values: commands_.size() (which is {}) != this->size() (which is {})",
                    commands_.size(), this->size());
            MPICXX_ASSERT_PRECONDITION(this->legal_command(commands_).first,
                    "Attempt to use the {}-th executable name which is only an empty string!", this->legal_command(commands_).second);
            MPICXX_ASSERT_PRECONDITION(this->legal_number_of_values(argvs_),
                    "illegal number of values: argvs_.size() (which is {}) != this->size() (which is {})",
                    argvs_.size(), this->size());
            MPICXX_ASSERT_PRECONDITION(this->legal_argv(argvs_), "Attempt to use an empty command line argument!",);
            MPICXX_ASSERT_PRECONDITION(this->legal_number_of_values(maxprocs_),
                    "Illegal number of values: maxprocs_.size() (which is {}) != this->size() (which is {})",
                    maxprocs_.size(), this->size());
            MPICXX_ASSERT_PRECONDITION(this->legal_maxprocs(maxprocs_).first,
                    "Attempt to use the {}-th maxprocs value (which is {}), which falls outside the valid range (0, {}]!",
                    this->legal_maxprocs(maxprocs_).second, maxprocs_[this->legal_maxprocs(maxprocs_).second],
                    mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));
            MPICXX_ASSERT_PRECONDITION(this->legal_maxprocs(this->total_maxprocs()),
                    "Attempt to use the total number of maxprocs (which is: {} = {}), which falls outside the valid range (0, {}]!",
                    fmt::join(maxprocs_, " + "), this->total_maxprocs(),
                    mpicxx::universe_size().value_or(std::numeric_limits<int>::max()));
            MPICXX_ASSERT_PRECONDITION(this->legal_number_of_values(info_),
                    "Illegal number of values: info_.size() (which is {}) != this->size() (which is {})",
                    info_.size(), this->size());
            MPICXX_ASSERT_PRECONDITION(this->legal_root(root_, comm_),
                    "The previously set root '{}' isn't a valid root in the current communicator!", root_);
            MPICXX_ASSERT_PRECONDITION(this->legal_communicator(comm_), "Can't use the  null communicator!");

            return_type res(this->total_maxprocs());

            // determine whether the placeholder MPI_ERRCODES_IGNORE shall be used or a "real" std::vector
            auto errcode = [&res]() {
                if constexpr (std::is_same_v<return_type, spawn_result_with_errcodes>) {
                    return res.errcodes_.data();
                } else {
                    return MPI_ERRCODES_IGNORE;
                }
            }();

            // convert vector of std::strings to vector of char*
            std::vector<char*> commands_ptr;
            commands_ptr.reserve(commands_.size());
            for (std::size_t i = 0; i < commands_.size(); ++i) {
                commands_ptr.emplace_back(commands_[i].data());
            }
            // convert vector of mpicxx::info to vector of MPI_INFO
            std::vector<MPI_Info> info_ptr;
            info_ptr.reserve(info_.size());
            for (std::size_t i = 0; i < info_.size(); ++i) {
                info_ptr.emplace_back(info_[i].get());
            }

            if (std::all_of(argvs_.cbegin(), argvs_.cend(), [](const auto& vec) { return vec.empty(); })) {
                // no additional arguments provided -> use MPI_ARGVS_NULL
                MPI_Comm_spawn_multiple(static_cast<int>(this->size()), commands_ptr.data(), MPI_ARGVS_NULL, maxprocs_.data(),
                                        info_ptr.data(), root_, comm_, &res.intercomm_, errcode);
            } else {
                // convert command line arguments to char***
                char*** argv_ptr = new char**[argvs_.size()];
                for (std::size_t i = 0; i < argvs_.size(); ++i) {
                    argv_ptr[i] = new char*[argvs_[i].size() + 1];
                    for (std::size_t j = 0; j < argvs_[i].size(); ++j) {
                        argv_ptr[i][j] = argvs_[i][j].data();
                    }
                    argv_ptr[i][argvs_[i].size()] = nullptr;
                }

                MPI_Comm_spawn_multiple(static_cast<int>(this->size()), commands_ptr.data(), argv_ptr, maxprocs_.data(),
                                        info_ptr.data(), root_, comm_, &res.intercomm_, errcode);

                // delete char***
                for (std::size_t i = 0; i < argvs_.size(); ++i) {
                    delete[] argv_ptr[i];
                }
                delete[] argv_ptr;
            }

            return res;
        }

#if MPICXX_ASSERTION_LEVEL > 0
        /*
         * @brief Checks whether the sizes of the iterator ranges [@p first1, @p last1) and [@p first2, @p last2) are equal.
         * @tparam InputIt1 must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator)
         * @tparam InputIt2 must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator)
         * @param[in] first1 iterator to the first element of the first range
         * @param[in] last1 iterator one-past the last element of the first range
         * @param[in] first2 iterator to the first element of the second range
         * @param[in] last2 iterator one-past the last element of the second range
         * @return `true` if both sizes are equal, `false` otherwise
         */
        template <std::input_iterator InputIt1, std::input_iterator InputIt2>
        bool legal_number_of_values(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2) {
            return std::distance(first1, last1) == std::distance(first2, last2);
        }
        /*
         * @brief Checks whether the size of the iterator range [@p first, @p last) equals the size of this @ref mpicxx::multiple_spawner.
         * @tparam InputIt must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator)
         * @param[in] first iterator to the first element of the range
         * @param[in] last iterator one-past the last element of the range
         * @return `true` if both sizes are equal, `false` otherwise
         */
        template <std::input_iterator InputIt>
        bool legal_number_of_values(InputIt first, InputIt last) {
            using difference_type = typename std::iterator_traits<InputIt>::difference_type;
            return std::distance(first, last) == static_cast<difference_type>(this->size());
        }
        /*
         * @brief Checks whether the size of the [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list)
         *        @p ilist equals the size of this @ref mpicxx::multiple_spawner.
         * @tparam T an arbitrary type
         * @param[in] ilist the [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list)
         * @return `true` if both sizes are equal, `false` otherwise
         */
        template <typename T>
        bool legal_number_of_values(const std::initializer_list<T> ilist) const {
            return ilist.size() == this->size();
        }
        /*
         * @brief Checks whether the size of the parameter pack @p args equals the size of this @ref mpicxx::multiple_spawner.
         * @tparam T an arbitrary type
         * @param[in] args the parameter pack
         * @return `true` if both sizes are equal, `false` otherwise
         */
        template <typename... T>
        bool legal_number_of_values(T&&... args) const {
            return sizeof...(args) == this->size();
        }
        /*
         * @brief Checks whether the size of the [`std::vector`](https://en.cppreference.com/w/cpp/container/vector) @p vec equals
         *        the size of this @ref mpicxx::multiple_spawner.
         * @tparam T an arbitrary type
         * @param[in] vec the [`std::vector`](https://en.cppreference.com/w/cpp/container/vector)
         * @return `true` if both sizes are equal, `false` otherwise
         */
        template <typename T>
        bool legal_number_of_values(std::vector<T>& vec) const {
            return vec.size() == this->size();
        }
        /*
         * @brief Check whether @p first and @p last denote a valid range, i.e. @p first is less or equal than @p last.
         * @details Checks whether the distance between @p first and @p last is not negative.
         * @tparam InputIt must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator)
         * @param[in] first iterator to the first element of the range
         * @param[in] last iterator to one-past the last element of the range
         * @return `true` if @p first and @p last denote a valid range, `false` otherwise
         */
        template <std::input_iterator InputIt>
        bool legal_iterator_range(InputIt first, InputIt last) const {
            return std::distance(first, last) >= 0;
        }
        /*
         * @brief Check whether @p first and @p last denote a valid, non-empty range, i.e. @p first is less than @p last.
         * @details Checks whether the distance between @p first and @p last is greater than `0`.
         * @tparam InputIt must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator)
         * @param[in] first iterator to the first element of the range
         * @param[in] last iterator to one-past the last element of the range
         * @return `true` if @p first and @p last denote a valid, non-empty range, `false` otherwise
         */
        template <std::input_iterator InputIt>
        bool legal_non_empty_iterator_range(InputIt first, InputIt last) {
            return std::distance(first, last) > 0;
        }
        /*
         * @brief Check whether @p command is legal, i.e. it is **not** empty.
         * @param[in] command the executable name
         * @return `true` if @p command is a valid executable name, `false` otherwise
         */
        bool legal_command(const std::string& command) const noexcept {
            return !command.empty();
        }
        /*
         * @brief Checks whether all executable names in @p commands are valid.
         * @param[in] commands the list of executable names
         * @return `true` if all executable names in @p commands are valid, `false` otherwise
         */
        std::pair<bool, std::size_t> legal_command(const std::vector<std::string>& commands) const noexcept {
            for (std::size_t i = 0; i < commands.size(); ++i) {
                if (!this->legal_command(commands[i])) {
                    return std::make_pair(false, i);
                }
            }
            return std::make_pair(true, commands.size());
        }
        /*
         * @brief Check whether @p arg is legal, i.e. it isn't empty.
         * @param[in] arg the command line argument
         * @return `true` if @p arg is valid, `false` otherwise
         */
        bool legal_argv(const std::string& arg) const noexcept {
            return !arg.empty();
        }
        /*
         * @brief Check whether all @p argvs are legal, i.e. the aren't empty.
         * @param[in] argvs the command line arguments
         * @return `true` if all @p argvs are valid, `false` otherwise
         */
        bool legal_argv(const std::vector<std::vector<std::string>>& argvs) const noexcept {
            for (std::size_t i = 0; i < argvs.size(); ++i) {
                for (std::size_t j = 0; j < argvs[i].size(); ++j) {
                    if (!this->legal_argv(argvs[i][j])) {
                        return false;
                    }
                }
            }
            return true;
        }
        /*
         * @brief Checks whether @p maxprocs is valid.
         * @details Checks whether @p maxprocs is greater than `0`. In addition, if the universe size could be queried, it's checked
         *          whether @p maxprocs is less or equal than the @ref mpicxx::universe_size().
         * @param[in] maxprocs the number of processes
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
         * @brief Checks whether all number of process in @p maxprocs are legal.
         * @param[in] maxprocs the list of number of maxprocs
         * @return `true` if all number of processes in @p maxprocs are legal, `false` otherwise
         */
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
         * @brief Checks whether @p comm is valid, i.e. it does **not** refer to
         *        [*MPI_COMM_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node149.htm).
         * @param[in] comm a intercommunicator
         * @return `true` if @p comm is valid, `false` otherwise
         */
        bool legal_communicator(const MPI_Comm comm) const noexcept {
            return comm != MPI_COMM_NULL;
        }
#endif

        size_type size_ = 0;
        std::vector<std::string> commands_;
        std::vector<std::vector<std::string>> argvs_;
        std::vector<int> maxprocs_;
        std::vector<info> info_;
        int root_ = 0;
        MPI_Comm comm_ = MPI_COMM_WORLD;
    };

}

#endif // MPICXX_MULTIPLE_SPAWNER_HPP