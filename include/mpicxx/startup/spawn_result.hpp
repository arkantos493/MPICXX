/**
 * @file include/mpicxx/startup/spawn_result.hpp
 * @author Marcel Breyer
 * @date 2020-04-14
 *
 * @brief Implements the class which gets returned from @ref mpicxx::single_spawner::spawn() resp. @ref mpicxx::multiple_spawner::spawn().
 */

#ifndef MPICXX_SPAWNER_RESULT_HPP
#define MPICXX_SPAWNER_RESULT_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <mpi.h>


namespace mpicxx {

    // TODO 2020-04-14 21:55 breyerml: change from MPI_Comm to mpicxx equivalent
    // TODO 2020-04-14 21:55 breyerml: change from int to mpicxx errcode equivalent
    // TODO 2020-04-14 21:55 breyerml: change from fmt::format to std::format
    // TODO 2020-04-14 21:55 breyerml: copy/move constructor/assignment

    // forward declare all spawner classes
    class single_spawner;
    class multiple_spawner;


    /**
     * @nosubgrouping
     * @brief This class implements all functions that can be called on the result of @ref mpicxx::single_spawner::spawn() resp.
     * @ref mpicxx::multiple_spawner::spawn().
     */
    class spawn_result {
        /// befriend @ref mpicxx::single_spawner
        friend class mpicxx::single_spawner;
        /// befriend @ref mpicxx::multiple_spawner
        friend class mpicxx::multiple_spawner;


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                constructor                                                 //
        // ---------------------------------------------------------------------------------------------------------- //
        /*
         * @brief Construct a new spawner_base object.
         * @param[in] maxprocs the total number of spawned processes.
         *
         * @assert_sanity{ If @p maxprocs is invalid. }
         */
        spawn_result(const int maxprocs) : errcodes_(maxprocs, -1) { }


    public:
        // ---------------------------------------------------------------------------------------------------------- //
        //                                          lookup spawn information                                          //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Returns the number of spawned processes.
         * @details Two possible behaviours:
         * 1. **hard** spawn: Either `maxprocs` processes are spawned (returning `maxprocs`) or the call to spawn results in an error
         * (returning `0`).
         * 2. **soft** spawn: The info object may specify an arbitrary set \f$\{m_i : 0 \leq m_i \leq maxprocs \}\f$ of allowed values for
         * the number of spawned processes. If one of these allowed numbers of processes \f$ m_i \f$ can be spawned, the call to spawn
         * succeeds (returning \f$ m_i \f$). If it isn't possible to spawn one of the allowed number of processes, the call to spawn results
         * in an error (returning `0`).
         * @return the number of spawned processes (`[[nodiscard]]`)
         *
         * @calls{
         * int MPI_Comm_remote_size(MPI_Comm comm, int *size);      // at most once
         * }
         */
        [[nodiscard]] int number_of_spawned_processes() const {
            if (intercomm_ != MPI_COMM_NULL) {
                int size;
                MPI_Comm_remote_size(intercomm_, &size);
                return size;
            } else {
                return 0;
            }
        }
        /**
         * @brief Check whether it was possible to spawn the requested number of processes.
         * @return `true` if the requested number of processes could be spawned, `false` otherwise (`[[nodiscard]]`)
         *
         * @calls{
         * int MPI_Comm_remote_size(MPI_Comm comm, int *size);      // at most once
         * }
         */
        [[nodiscard]] bool maxprocs_processes_spawned() const {
            return errcodes_.size() == static_cast<std::size_t>(this->number_of_spawned_processes());
        }
        /**
         * @brief Returns the intercommunicator between the original group and the newly spawned group.
         * @return the intercommunicator (`[[nodiscard]]`)
         */
        [[nodiscard]] MPI_Comm intercommunicator() const noexcept {
            return intercomm_;
        }
        /**
         * @brief Returns the errcodes (one code per process) returned by the spawn call.
         * @return the errcodes (`[[nodiscard]]`)
         */
        [[nodiscard]] const std::vector<int>& errcodes() const noexcept {
            return errcodes_;
        }
        /**
         * @brief Prints the number of failed spawns and the respective errcode messages (including how often the errcode occurred).
         * @param[inout] out the output stream on which the errcodes messages should be written
         *
         * @calls{
         * int MPI_Error_string(int errorcode, char *string, int *resultlen);       // at most 'maxprocs' times
         * }
         */
        void print_errors_to(std::ostream& out = std::cout) const {
            // count and display the number of errors
            const auto failed_spawns = std::count_if(errcodes_.cbegin(), errcodes_.cend(),
                    [](const int err) { return err != MPI_SUCCESS; });
            out << fmt::format("{} {} occurred!:\n", failed_spawns, failed_spawns == 1 ? "error" : "errors");

            // count how often each error occurred
            std::map<int, int> counts;
            for (const int err : errcodes_) {
                if (err != MPI_SUCCESS) {
                    ++counts[err];
                }
            }

            // retrieve the error string and print it
            for (const auto [err, count] : counts) {
                if (err == -1) {
                    out << fmt::format("{:>5}x Failed to retrieve error string\n", count);
                } else {
                    char error_string[MPI_MAX_ERROR_STRING];
                    int resultlen;
                    MPI_Error_string(err, error_string, &resultlen);
                    out << fmt::format("{:>5}x {}\n", count, std::string(error_string, resultlen));
                }
            }
        }


    private:
        MPI_Comm intercomm_;
        std::vector<int> errcodes_;
    };


    /**
     * @brief Returns the parent intracommunicator of the current process if the process was started with *MPI_COMM_SPAWN* or
     * *MPI_COMM_SPAWN_MULTIPLE*.
     * @return a [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional) containing the parent intracommunicator or
     * `std::nullopt` (`[[nodiscard]]`)
     */
    [[nodiscard]] inline std::optional<MPI_Comm> parent_process() {
        MPI_Comm intercomm;
        MPI_Comm_get_parent(&intercomm);
        if (intercomm != MPI_COMM_NULL) {
            return std::make_optional(intercomm);
        } else {
            return std::nullopt;
        }
    }

}

#endif // MPICXX_SPAWNER_RESULT_HPP
