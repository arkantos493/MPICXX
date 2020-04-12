/**
 * @file include/mpicxx/startup/single_spawner.hpp
 * @author Marcel Breyer
 * @date 2020-04-12
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

    /**
     * @brief Spawner class which enables to spawn (multiple) MPI processes at runtime.
     */
    class single_spawner {
        /// The type of a single argv argument (represented by  a key and value).
        using argv_type = std::pair<std::string, std::string>;
    public:
        // ---------------------------------------------------------------------------------------------------------- //
        //                                               constructor                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name constructor
        ///@{
        /**
         * @brief Construct a new single_spawner object.
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
            : base_(maxprocs), command_(std::forward<decltype(command)>(command)), maxprocs_(maxprocs)
        {
            MPICXX_ASSERT_SANITY(!command_.empty(), "No executable name given!");
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                       getter/setter spawn information                                      //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name PLACEHOLDER
        ///@{
        /**
         * @brief Returns the name of the executable which should get spawned.
         * @return the executable name (`[[nodiscard]]`)
         */
        [[nodiscard]] const std::string& command() const noexcept { return command_; }
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
         * @brief Returns the info object representing additional information for the runtime system where and how to spawn the processes.
         * @return the info object (`[[nodiscard]]`)
         */
        [[nodiscard]] const info& spawn_info() const noexcept { return *info_; }
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
        single_spawner& add_argv(std::string key, T&& value) {
            // add leading '-' if necessary
            if (!key.starts_with('-')) {
                key.insert(0, 1, '-');
            }
            // add [key, value]-argv-pair to argvs
            argv_.emplace_back(std::move(key), detail::convert_to_string(std::forward<T>(value)));
            return *this;
        }
        /**
         * @brief Adds all elements from the range [@p first, @p last)  to the `argvs` list.
         * @tparam InputIt must meet the requirements of [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator).
         * @param[in] first iterator to the first `argv` in the range
         * @param[in] last iterator one-past the last `argv` in the range
         *
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid range, i.e. @p first must be less or equal than @p last.
         *
         * @assert_precondition{ If @p first and @p last don't denote a valid range. }
         */
        template <std::input_iterator InputIt>
        single_spawner& add_argv(InputIt first, InputIt last) requires (!std::is_constructible_v<std::string, InputIt>) {
            MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");

            for (; first != last; ++first) {
                auto&& pair = *first;
                this->add_argv(std::forward<decltype(pair.first)>(pair.first), std::forward<decltype(pair.second)>(pair.second));
            }
            return *this;
        }
        /**
         * @brief Adds all elements from the initializer list @p ilist to the `argvs` list.
         * @tparam T the type of the value
         * @param[in] ilist initializer list to insert the `argvs` from
         */
        template <typename T>
        single_spawner& add_argv(std::initializer_list<std::pair<std::string, T>> ilist) {
            return this->add_argv(ilist.begin(), ilist.end());
        }
        /**
         * @brief Returns the arguments which will be passed to `command`.
         * @return the arguments passed to `command` (`[[nodiscard]]`)
         */
        [[nodiscard]] const std::vector<argv_type>& argv() const noexcept { return argv_; }
        /**
         * @brief Returns the i-th argument which will be passed to `command`.
         * @param[in] i the argv to return
         * @return the i-th argument (`[[nodiscard]]`)
         *
         * @throws std::out_of_range if the index @p i is an out-of-bounce access
         */
        [[nodiscard]] const argv_type& argv(const std::size_t i) const {
            if (i >= argv_.size()) {
                throw std::out_of_range(fmt::format("Out-of-bounce access!: {} < {}", i, argv_.size()));
            }

            return argv_[i];
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            spawn new process(es)                                           //
        // ---------------------------------------------------------------------------------------------------------- //
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
            MPICXX_ASSERT_PRECONDITION(info_ != nullptr, "The spawn info object went out-of-scope!");

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


        // ---------------------------------------------------------------------------------------------------------- //
        //                                      functions provided by spawner_base                                    //
        // ---------------------------------------------------------------------------------------------------------- //
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
         * @copydoc detail::spawner_base::universe_size()
         */
        [[nodiscard]] static int universe_size() {
            return detail::spawner_base::universe_size();
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


    private:
#if ASSERTION_LEVEL > 0
        /*
         * @brief Check whether @p first and @p last denote a valid range, i.e. @p first is less or equal than @p last.
         * @details Checks whether the distance bewteen @p first and @p last is not negative.
         */
        template <std::input_iterator InputIt>
        bool legal_iterator_range(InputIt first, InputIt last) {
            return std::distance(first, last) >= 0;
        }
#endif
        detail::spawner_base base_;

        std::string command_;
        int maxprocs_;
        std::vector<argv_type> argv_;
        const info* info_ = &mpicxx::info::null;
    };

    /// default spawner is a @ref single_spawner
    using spawner = single_spawner;

}


#endif // MPICXX_SINGLE_SPAWNER_HPP
