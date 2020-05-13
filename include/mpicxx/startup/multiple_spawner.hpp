/**
 * @file include/mpicxx/startup/multiple_spawner.hpp
 * @author Marcel Breyer
 * @date 2020-05-13
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
#include <mpicxx/detail/utility.hpp>
#include <mpicxx/info/info.hpp>
#include <mpicxx/startup/single_spawner.hpp>
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
        // TODO 2020-05-13 00:21 breyerml: remove as soon as GCC bug has been fixed
        template <typename SpawnerType>
        static constexpr bool is_spawner_v = std::is_same_v<std::remove_cvref_t<SpawnerType>, single_spawner>
                || std::is_same_v<std::remove_cvref_t<SpawnerType>, multiple_spawner>;

    public:
        /// The type of a single argv argument (represented by  a key and value).
        using argv_value_type = std::pair<std::string, std::string>;
        /// Unsigned integer type for argv.
        using argv_size_type = std::size_t;
        /// Unsigned integer type.
        using size_type = std::size_t;

        // ---------------------------------------------------------------------------------------------------------- //
        //                                               constructor                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name constructor
        ///@{
        /**
         * @brief Constructs the multiple_spawner object with the contents of the range [@p first, @p last).
         * @tparam InputIt must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator).
         * @param[in] first iterator to the first pair in the range
         * @param[in] last iterator one-past the last pair in the range
         *
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid, non-empty range, i.e. @p first must be strictly less than @p last.
         * @pre **Any** command **must not** be empty.
         * @pre **Any** maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         *
         * @assert_precondition{ If @p first and @p last don't denote a valid, non-empty iterator range. }
         * @assert_sanity{
         * If any command name is empty.\n
         * If any number of maxprocs is invalid.\n
         * If the total number of maxprocs is invalid.
         * }
         */
        template <std::input_iterator InputIt>
        multiple_spawner(InputIt first, InputIt last) {
            const auto size = std::distance(first, last);

            MPICXX_ASSERT_PRECONDITION(this->legal_non_empty_iterator_range(first, last), "No values given! 0 < {}", size);

            // set command and maxprocs according to passed values
            commands_.reserve(size);
            maxprocs_.reserve(size);

            for (; first != last; ++first) {
                const auto& pair = *first;

                MPICXX_ASSERT_SANITY(this->legal_command(pair.first), "No executable name given at {}!", size - std::distance(first, last));
                MPICXX_ASSERT_SANITY(this->legal_maxprocs(pair.second),
                        "Can't spawn the given number of processes at {}!: 0 < {} <= {}",
                        size - std::distance(first, last), pair.second,
                        multiple_spawner::universe_size().value_or(std::numeric_limits<int>::max()));

                commands_.emplace_back(pair.first);
                maxprocs_.emplace_back(pair.second);
            }

            MPICXX_ASSERT_SANITY(this->legal_maxprocs(std::reduce(maxprocs_.begin(), maxprocs_.end())),
                    "Can't spawn total number of processes!: 0 < {} <= {}",
                    std::reduce(maxprocs_.begin(), maxprocs_.end()),
                    single_spawner::universe_size().value_or(std::numeric_limits<int>::max()));

            // set info objects to default values
            infos_.assign(size, mpicxx::info::null);
            // set argvs objects to default values
            argvs_.assign(size, std::vector<argv_value_type>());
        }
        /**
         * @brief Constructs the multiple_spawner object with the contents of the initializer list @p ilist.
         * @param[in] ilist initializer list to initialize the multiple_spawner object with
         *
         * @pre @p first and @p last **must** form a non-empty range, i.e. @p first must be strictly less than @p last.
         * @pre **Any** command **must not** be empty.
         * @pre **Any** maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         *
         * @assert_precondition{ If @p first and @p last don't denote a non-empty iterator range. }
         * @assert_sanity{
         * If any command name is empty.\n
         * If any number of maxprocs is invalid.\n
         * If the total number of maxprocs is invalid.
         * }
         */
        multiple_spawner(std::initializer_list<std::pair<std::string, int>> ilist) : multiple_spawner(ilist.begin(), ilist.end()) { }
        /**
         * @brief Constructs the multiple_spawner object with the contents of the parameter pack @p args.
         * @tparam T an arbitrary number of pairs meeting the @ref detail::is_pair requirements.
         * @param[in] args the parameter pack to initializer the multiple_spawner object with
         *
         * @pre **Any** command **must not** be empty.
         * @pre **Any** maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         *
         * @assert_sanity{
         * If any command name is empty.\n
         * If any number of maxprocs is invalid.\n
         * If the total number of maxprocs is invalid.
         * }
         */
        template <detail::is_pair... T>
        multiple_spawner(T&&... args) requires (sizeof...(T) > 0) {
            // set command and maxprocs according to passed values
            constexpr auto size = sizeof...(T);
            commands_.reserve(size);
            maxprocs_.reserve(size);

            ([&] (auto&& arg) {

                MPICXX_ASSERT_SANITY(this->legal_command(arg.first), "No executable name given!");
                MPICXX_ASSERT_SANITY(this->legal_maxprocs(arg.second),
                        "Can't spawn the given number of processes!: 0 < {} <= {}",
                        arg.second, multiple_spawner::universe_size().value_or(std::numeric_limits<int>::max()));

                using pair_t = decltype(arg);
                commands_.emplace_back(std::forward<pair_t>(arg).first);
                maxprocs_.emplace_back(std::forward<pair_t>(arg).second);
            }(std::forward<T>(args)), ...);

            MPICXX_ASSERT_SANITY(this->legal_maxprocs(std::reduce(maxprocs_.begin(), maxprocs_.end())),
                    "Can't spawn total number of processes!: 0 < {} <= {}",
                    std::reduce(maxprocs_.begin(), maxprocs_.end()),
                    single_spawner::universe_size().value_or(std::numeric_limits<int>::max()));

            // set info objects to default values
            infos_.assign(size, mpicxx::info::null);
            // set argvs objects to default values
            argvs_.assign(size, std::vector<argv_value_type>());
        }
        // TODO 2020-05-11 22:58 breyerml: change to c++20 concepts syntax as soon as GCC bug has been fixed
        /**
         * @brief Constructs the multiple_spawner object with the spawner object(s) of the parameter pack @p args.
         * @tparam T an arbitrary number of spawners meeting the @ref detail::is_spawner requirements.
         * @param[in] args the parameter pack to initializer the multiple_spawner object with
         *
         * @pre **All** roots **must** be equal.
         * @pre **All** communicators **must** be equal.
         * @pre The total number of maxprocs **must not** be less or equal than `0` or greater than the maximum possible number of processes
         * (@ref universe_size()).
         *
         * @assert_precondition{
         * If not all roots are equivalent. \n
         * If not all communicators are equivalent.
         * }
         * @assert_sanity{ If the total number of maxprocs is invalid. }
         */
        template <typename... T, std::enable_if_t<(is_spawner_v<T> && ...), int> = 0>
        multiple_spawner(T&&... args) requires (sizeof...(T) > 0) {
            MPICXX_ASSERT_PRECONDITION(detail::all_same([](const auto& arg) { return arg.root(); }, args...),
                    "Different root processes!");
            MPICXX_ASSERT_PRECONDITION(detail::all_same([](const auto& arg) { return arg.communicator(); }, args...),
                    "Different communicators!");

            ([&] (auto&& arg) {
                using spawner_t = decltype(arg);
                if constexpr (std::is_same_v<std::remove_cvref_t<spawner_t>, single_spawner>) {
                    commands_.emplace_back(std::forward<spawner_t>(arg).command());
                    argvs_.emplace_back(std::forward<spawner_t>(arg).argv());
                    maxprocs_.emplace_back(std::forward<spawner_t>(arg).maxprocs());
                    infos_.emplace_back(std::forward<spawner_t>(arg).spawn_info());
                } else if constexpr (std::is_same_v<std::remove_cvref_t<spawner_t>, multiple_spawner>) {
                    for (multiple_spawner::size_type i = 0; i < arg.size(); ++i) {
                        commands_.emplace_back(std::forward<spawner_t>(arg).command(i));
                        argvs_.emplace_back(std::forward<spawner_t>(arg).argv(i));
                        maxprocs_.emplace_back(std::forward<spawner_t>(arg).maxprocs(i));
                        infos_.emplace_back(std::forward<spawner_t>(arg).spawn_info(i));
                    }
                }
                root_ = arg.root();
                comm_ = arg.communicator();
            }(std::forward<T>(args)), ...);

            MPICXX_ASSERT_SANITY(this->legal_maxprocs(std::reduce(maxprocs_.begin(), maxprocs_.end())),
                    "Can't spawn total number of processes!: 0 < {} <= {}",
                    std::reduce(maxprocs_.begin(), maxprocs_.end()),
                    single_spawner::universe_size().value_or(std::numeric_limits<int>::max()));
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                       getter/setter spawn information                                      //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name modify spawn information
        ///@{
        /**
         * @brief Replaces the old executable names with the new names from the range [@p first, @p last).
         * @param[in] first iterator to the first executable name in the range
         * @param[in] last iterator one-past the last executable name in the range
         * @return `*this`
         *
         * @pre The size of the range [@p first, @p last) **must** match the size of the this @ref multiple_spawner.
         * @pre All executable names in the range [@p first, @p last) **must not** be empty.
         *
         * @assert_sanity{
         * If the sizes mismatch. \n
         * If any new executable name is empty.
         * }
         */
        template <std::input_iterator InputIt>
        multiple_spawner& set_command(InputIt first, InputIt last) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(first, last),
                    "Illegal number of values! {} == {}", std::distance(first, last), this->size());

            commands_.assign(first, last);

            MPICXX_ASSERT_SANITY(this->legal_commands(commands_).first,
                    "No executable name given at {}!", this->legal_commands(commands_).second);

            return *this;
        }
        /**
         * @brief Replaces the old executable names with the new names from the initializer list @p ilist.
         * @param[in] ilist the new executable names
         * @return `*this`
         *
         * @pre The size of @p ilist **must** match the size of this @ref multiple_spawner.
         * @pre All executable names in @p ilist **must not** be empty.
         *
         * @assert_sanity{
         * If the sizes mismatch. \n
         * If any new executable name is empty.
         * }
         */
        multiple_spawner& set_command(std::initializer_list<std::string> ilist) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(ilist),
                    "Illegal number of values! {} == {}", ilist.size(), this->size());

            commands_.assign(ilist);

            MPICXX_ASSERT_SANITY(this->legal_commands(commands_).first,
                    "No executable name given at {}!", this->legal_commands(commands_).second);

            return *this;
        }
        /**
         * @brief Replaces the old executable names with the new names from the parameter pack @p args.
         * @tparam T an arbitrary number of string like objects meeting @p detail::is_string requirements.
         * @param args the parameter pack containing the new executable names
         * @return `*this`
         *
         * @pre The size of the parameter pack @p args **must** match the size of this @ref multiple_spawner.
         * @pre All executable names in the parameter pack @p args **must not** be empty.
         *
         * @assert_sanity{
         * If the sizes mismatch. \n
         * If any new executable name is empty.
         * }
         */
        template <detail::is_string... T>
        multiple_spawner& set_command(T&&... args) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(args...),
                    "Illegal number of values! {} == {}", sizeof...(T), this->size());

            commands_.clear();
            (commands_.emplace_back(std::forward<T>(args)), ...);

            MPICXX_ASSERT_SANITY(this->legal_commands(commands_).first,
                    "No executable name given at {}!", this->legal_commands(commands_).second);

            return *this;
        }
        /**
         * @brief Change the i-th executable name to @p name.
         * @tparam T must meet the @p detail::is_string requirements.
         * @param[in] i the index of the executable name to be changed
         * @param[in] name new name of the i-th executable
         * @return `*this`
         *
         * @pre @p name **must not** be empty.
         *
         * @assert_sanity{ If @p name is empty. }
         *
         * @throws std::out_of_range if the index @p i falls outside the valid range
         */
        template <detail::is_string T>
        multiple_spawner& set_command(const std::size_t i, T&& name) {
            if (i >= this->size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::set_command(const std::size_t, T&&) range check: i (which is {}) >= this->size() (which is {})",
                        i, this->size()));
            }

            commands_[i] = std::forward<T>(command);

            MPICXX_ASSERT_SANITY(this->legal_command(commands_[i]), "No executable name given at {}!", i);

            return *this;
        }
        /**
         * @brief Returns all executable names.
         * @return the executable names (`[[nodiscard]]`)
         */
        [[nodiscard]] const std::vector<std::string>& command() const noexcept { return commands_; }
        /**
         * @brief Returns the name of the i-th executable which should get spawned.
         * @param[in] i the index of the executable name to be retrieved
         * @return the i-th executable name (`[[nodiscard]]`)
         *
         * @throws std::out_of_range if the index @p i is an out-of-bounce access
         */
        [[nodiscard]] const std::string& command(const std::size_t i) const {
            if (i >= this->size()) {
                throw std::out_of_range(fmt::format(
                        "multiple_spawner::command(const std::size_t) range check: i (which is {}) >= this->size() (which is {})",
                        i, this->size()));
            }

            return commands_[i];
        }



        // TODO 2020-04-15 22:14 breyerml: argvs
        template <typename ValueType>
        multiple_spawner& add_argv(std::initializer_list<std::initializer_list<std::pair<std::string, ValueType>>> ilist) {
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(ilist),
                    "Illegal number of values! {} == {}", ilist.size(), this->size());

            for (std::size_t i = 0; auto list : ilist) {
                this->add_argv(i, list);
                ++i;
            }

            return *this;
        }

        template <std::input_iterator InputIt>
        multiple_spawner& add_argv(InputIt first, InputIt last) {
            MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");

            for (; first != last; ++first) {
                const auto& cont = *first;
                this->add_argv(cont.begin(), cont.end());
            }

            return *this;
        }


        template <typename ValueType>
        multiple_spawner& add_argv(const std::size_t i, std::string key, ValueType&& value) {
            if (i >= argvs_.size()) {
                throw std::out_of_range(fmt::format("Out-of-bounce access!: {} < {}", i, this->size()));
            }

            // add leading '-' if necessary
            if (!key.starts_with('-')) {
                key.insert(0, 1, '-');
            }

            MPICXX_ASSERT_SANITY(this->legal_argv_key(key), "Only '-' isn't a valid argument key!");

            // add [key, value]-argv-pair to argv_ at pos i
            argvs_[i].emplace_back(std::move(key), detail::convert_to_string(std::forward<ValueType>(value)));
            return *this;
        }

        template <std::input_iterator InputIt>
        requires (!std::is_constructible_v<std::string, InputIt>)
        multiple_spawner& add_argv(const std::size_t i, InputIt first, InputIt last) {
            MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");

            for (; first != last; ++first) {
                const auto& pair = *first;
                this->add_argv(i, pair.first, pair.second);
            }
            return *this;
        }

        template <typename ValueType>
        multiple_spawner& add_argv(const std::size_t i, std::initializer_list<std::pair<std::string, ValueType>> ilist) {
            return this->add_argv(i, ilist.begin(), ilist.end());
        }

        [[nodiscard]] const std::vector<std::vector<argv_value_type>>& argv() const noexcept {
            return argvs_;
        }
        [[nodiscard]] const std::vector<argv_value_type>& argv(const std::size_t i) const {
            if (i >= argvs_.size()) {
                throw std::out_of_range(fmt::format("Out-of-bounce access!: {} < {}", i, this->size()));
            }

            return argvs_[i];
        }
        [[nodiscard]] const argv_value_type& argv(const std::size_t i, const std::size_t j) const {
            if (i >= argvs_.size()) {
                throw std::out_of_range(fmt::format("Out-of-bounce access (i)!: {} < {}", i, this->size()));
            } else if (j >= argvs_[i].size()) {
                throw std::out_of_range(fmt::format("Out-of-bounce access (j)!: {} < {}", j, argvs_[i].size()));
            }

            return argvs_[i][j];
        }
        [[nodiscard]] std::vector<argv_size_type> argv_size() const {
            std::vector<argv_size_type> sizes(this->size());
            std::transform(argvs_.cbegin(), argvs_.cend(), sizes.begin(),
                    [](const std::vector<argv_value_type>& argvs) { return argvs.size(); });
            return sizes;
        }


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
                    "Illegal number of values! {} == {}", std::distance(first, last), this->size());

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
                    "Illegal number of values! {} == {}", ilist.size(), this->size());

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
                throw std::out_of_range(fmt::format("Out-of-bounce access!: {} < {}", i, this->size()));
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
                throw std::out_of_range(fmt::format("Out-of-bounce access!: {} < {}", i, this->size()));
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

            infos_.assign(first, last);

            return *this;
        }
        multiple_spawner& set_spawn_info(std::initializer_list<info> ilist) noexcept { // TODO 2020-04-16 21:51 breyerml: noexcept
            MPICXX_ASSERT_SANITY(this->legal_number_of_values(ilist),
                    "Illegal number of values! {} == {}", ilist.size(), commands_.size());

            infos_.assign(ilist);
            
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
         * @assert_sanity{ If @p root isn't a legal root. }
         */
        multiple_spawner& set_root(const int root) noexcept {
            MPICXX_ASSERT_SANITY(this->legal_root(root, comm_),
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
         * @assert_sanity{
         * If @p comm is the null communicator (*MPI_COMM_NULL*). \n
         * If the currently specified root isn't valid in @p comm.
         * }
         */
        multiple_spawner& set_communicator(MPI_Comm comm) noexcept {
            MPICXX_ASSERT_SANITY(this->legal_communicator(comm), "Can't use null communicator!");
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


        // additional
        [[nodiscard]] size_type size() const noexcept {
            MPICXX_ASSERT_SANITY(detail::all_same([](const auto& vec1, const auto& vec2) { return vec1.size() == vec2.size(); },
                    commands_, argvs_, maxprocs_, infos_), "Sizes differ!");

            return commands_.size();
        }
        [[nodiscard]] int total_maxprocs() const noexcept {
            int total = 0;
            for (const int i : maxprocs_) {
                total += i;
            }
            return total;
        }


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

//#if ASSERTION_LEVEL > 0
        template <std::input_iterator InputIt>
        bool legal_non_empty_iterator_range(InputIt first, InputIt last) {
            return std::distance(first, last) > 0;
        }

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
        std::pair<bool, std::size_t> legal_commands(const std::vector<std::string>& commands) const noexcept {
            for (std::size_t i = 0; i < commands.size(); ++i) {
                if (!this->legal_command(commands[i])) {
                    return std::make_pair(false, i);
                }
            }
            return std::make_pair(true, commands.size());
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
//#endif

        std::vector<std::string> commands_;
        std::vector<std::vector<argv_value_type>> argvs_;
        std::vector<int> maxprocs_;
        std::vector<info> infos_;
        int root_ = 0;
        MPI_Comm comm_ = MPI_COMM_WORLD;
    };

}


#endif // MPICXX_MULTIPLE_SPAWNER_HPP
