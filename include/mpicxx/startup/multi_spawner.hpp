/**
 * @file include/mpicxx/startup/multi_spawner.hpp
 * @author Marcel Breyer
 * @date 2020-04-09
 *
 * @brief Implements wrapper around the *MPI_COMM_SPAWN_MULTIPLE* function.
 */

#ifndef MPICXX_MULTI_SPAWNER_HPP
#define MPICXX_MULTI_SPAWNER_HPP

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
#include <mpicxx/startup/spawner_base.hpp>


namespace mpicxx {

    // TODO 2020-03-22 19:04 marcel: change from MPI_Comm to mpicxx equivalent
    // TODO 2020-03-23 12:56 marcel: errcode equivalent
    // TODO 2020-03-23 12:56 marcel: change from fmt::format to std::format
    // TODO 2020-03-23 17:37 marcel: copy/move constructor/assignment

    /**
     * @brief Spawner class which enables to spawn (multiple) **different** MPI processes at runtime.
     */
    class multiple_spawner {
        /// the type of a single argv argument (including a key and a value)
        using argv_type = std::pair<std::string, std::string>;
    public:
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

            base_ = spawner_base(std::reduce(maxprocs_.cbegin(), maxprocs_.cend()));
      }

        [[nodiscard]] const std::vector<std::string>& command() const noexcept { return commands_; }
//        [[nodiscard]] const std::string& command(const std::size_t i) const noexcept { return commands_[i]; } // TODO 2020-03-24 17:09 marcel: exceptions?

        [[nodiscard]] const std::vector<int>& maxprocs() const noexcept { return maxprocs_; }
//        [[nodiscard]] int maxprocs(const std::size_t i) const noexcept { return maxprocs_[i]; }


        template <typename... Infos>
        requires (std::is_same_v<std::decay_t<Infos>, info> && ...)
        void set_spawn_info(Infos... infos) noexcept(std::is_nothrow_copy_assignable_v<info>) {
            // TODO 2020-03-24 17:29 marcel: asserts?, parameter type?, calls?
            const auto add_to = [&]<typename T>(T&& arg) {
                infos_.emplace_back(std::forward<T>(arg));
            };
            (add_to(std::forward<Infos>(infos)), ...);
        }
        [[nodiscard]] const std::vector<info>& spawn_info() const noexcept { return infos_; }
//        [[nodiscard]] const info& spawn_info(const std::size_t i) const noexcept { return infos_[i]; }


        /**
         * @brief DOC
         */
        void spawn() {

        }


    private:
        spawner_base base_;

        std::vector<std::string> commands_;
        std::vector<int> maxprocs_;
        std::vector<std::vector<argv_type>> argvs_;
        std::vector<info> infos_;
    };

}


#endif // MPICXX_MULTI_SPAWNER_HPP
