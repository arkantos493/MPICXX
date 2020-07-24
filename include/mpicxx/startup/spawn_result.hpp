/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-24
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Implements the class which gets returned from the @ref mpicxx::single_spawner::spawn(),
 *        @ref mpicxx::single_spawner::spawn_with_errcodes(), @ref mpicxx::multiple_spawner::spawn() and
 *        @ref mpicxx::multiple_spawner::spawn_with_errcodes() functions.
 */

#ifndef MPICXX_SPAWNER_RESULT_HPP
#define MPICXX_SPAWNER_RESULT_HPP

#include <fmt/format.h>
#include <mpi.h>

#include <algorithm>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mpicxx {

    // TODO 2020-04-14 21:55 breyerml: change from MPI_Comm to mpicxx equivalent
    // TODO 2020-04-14 21:55 breyerml: change from int to mpicxx errcode equivalent

    // forward declare all spawner classes
    class single_spawner;
    class multiple_spawner;


    /**
     * @nosubgrouping
     * @brief This class implements all functions that can be called on the result of @ref mpicxx::single_spawner::spawn_with_errcodes()
     *        respectively \n@ref mpicxx::multiple_spawner::spawn_with_errcodes().
     * @details Same as @ref mpicxx::spawn_result but also contains information about potentially error codes.
     */
    class spawn_result_with_errcodes {
        // befriend mpicxx::single_spawner
        friend class mpicxx::single_spawner;
        // befriend mpicxx::multiple_spawner
        friend class mpicxx::multiple_spawner;


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                constructor                                                 //
        // ---------------------------------------------------------------------------------------------------------- //
        /*
         * @brief Construct a new spawn_result_with_errcodes object.
         * @param[in] maxprocs the total number of spawned processes
         */
        spawn_result_with_errcodes(const int maxprocs) : errcodes_(maxprocs, -1) { }


    public:
        // ---------------------------------------------------------------------------------------------------------- //
        //                                          lookup spawn information                                          //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Returns the number of spawned processes.
         * @details Two possible behaviors:
         *          1. **hard** spawn: Either `maxprocs` processes are spawned (returning `maxprocs`) or the call to spawn results in
         *             an error (returning `0`).
         *          2. **soft** spawn: The info object may specify an arbitrary set \f$\{m_i : 0 \leq m_i \leq maxprocs \}\f$ of allowed
         *             values for the number of spawned processes. If one of these allowed numbers of processes \f$ m_i \f$ can be spawned,
         *             the call to spawn succeeds (returning \f$ m_i \f$). If it isn't possible to spawn one of the allowed number of
         *             processes, the call to spawn results in an error (returning `0`).
         * 
         * @return the number of spawned processes
         * @nodiscard
         *
         * @calls{ int MPI_Comm_remote_size(MPI_Comm comm, int *size);    // at most once }
         */
        [[nodiscard]]
        int number_of_spawned_processes() const {
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
         * @return `true` if the requested number of processes could be spawned, `false` otherwise
         * @nodiscard
         *
         * @calls{ int MPI_Comm_remote_size(MPI_Comm comm, int *size);    // at most once }
         */
        [[nodiscard]]
        bool all_processes_spawned() const {
            return errcodes_.size() == static_cast<typename decltype(errcodes_)::size_type>(this->number_of_spawned_processes());
        }
        /**
         * @brief Returns the intercommunicator between the original and the newly spawned group.
         * @return the intercommunicator
         * @nodiscard
         */
        [[nodiscard]]
        MPI_Comm intercommunicator() const noexcept {
            return intercomm_;
        }
        /**
         * @brief Returns the errcodes (one for each process) returned by the
         *        [*MPI_Comm_spawn*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node237.htm) respectively
         *        [*MPI_Comm_spawn_multiple*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node238.htm) call.
         * @return the errcodes
         * @nodiscard
         */
        [[nodiscard]]
        const std::vector<int>& errcodes() const noexcept {
            return errcodes_;
        }
        /**
         * @brief Returns the number of failed spawns and the respective errcode messages (including how often an errcode occurred).
         * @return the errcode messages
         * @nodiscard
         *
         * @calls{ int MPI_Error_string(int errorcode, char *string, int *resultlen);    // at most 'maxprocs' times }
         */
        [[nodiscard]]
        std::string error_list() const {
            // count the number of errors
            const auto failed_spawns = std::count_if(errcodes_.cbegin(), errcodes_.cend(),
                    [](const int err) { return err != MPI_SUCCESS; });

            // no errors occurred
            if (failed_spawns == 0) {
                return "0 errors occurred!";
            }

            fmt::memory_buffer buf;
            fmt::format_to(buf, "{} {} occurred!:\n", failed_spawns, failed_spawns == 1 ? "error" : "errors");

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
                    fmt::format_to(buf, "{:>5}x Failed to retrieve error string\n", count);
                } else {
                    char error_string[MPI_MAX_ERROR_STRING];
                    int resultlen;
                    MPI_Error_string(err, error_string, &resultlen);
                    fmt::format_to(buf, "{:>5}x {}\n", count, std::string_view(error_string, resultlen));
                }
            }

            using std::to_string;
            return to_string(buf);
        }
        
    private:
        std::vector<int> errcodes_;
        MPI_Comm intercomm_ = MPI_COMM_NULL;
    };


    /**
     * @nosubgrouping
     * @brief This class implements all functions that can be called on the result of @ref mpicxx::single_spawner::spawn() respectively
     *        @ref mpicxx::multiple_spawner::spawn().
     * @details Unlike @ref mpicxx::spawn_result_with_errcodes this class does **not** contain error codes.
     */
    class spawn_result {
        // befriend mpicxx::single_spawner
        friend class mpicxx::single_spawner;
        // befriend mpicxx::multiple_spawner
        friend class mpicxx::multiple_spawner;


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                constructor                                                 //
        // ---------------------------------------------------------------------------------------------------------- //
        /*
         * @brief Construct a new spawn_result object.
         * @param[in] maxprocs the total number of spawned processes.
         */
        spawn_result(const int maxprocs) : maxprocs_(maxprocs) { }


    public:
        // ---------------------------------------------------------------------------------------------------------- //
        //                                          lookup spawn information                                          //
        // ---------------------------------------------------------------------------------------------------------- //
        /**
         * @brief Returns the number of spawned processes.
         * @details Two possible behaviors:
         *          1. **hard** spawn: Either `maxprocs` processes are spawned (returning `maxprocs`) or the call to spawn results in
         *             an error (returning `0`).
         *          2. **soft** spawn: The info object may specify an arbitrary set \f$\{m_i : 0 \leq m_i \leq maxprocs \}\f$ of allowed
         *             values for the number of spawned processes. If one of these allowed numbers of processes \f$ m_i \f$ can be spawned,
         *             the call to spawn succeeds (returning \f$ m_i \f$). If it isn't possible to spawn one of the allowed number of
         *             processes, the call to spawn results in an error (returning `0`).
         *
         * @return the number of spawned processes
         * @nodiscard
         *
         * @calls{ int MPI_Comm_remote_size(MPI_Comm comm, int *size);    // at most once }
         */
        [[nodiscard]]
        int number_of_spawned_processes() const {
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
         * @return `true` if the requested number of processes could be spawned, `false` otherwise
         * @nodiscard
         *
         * @calls{ int MPI_Comm_remote_size(MPI_Comm comm, int *size);    // at most once }
         */
        [[nodiscard]]
        bool all_processes_spawned() const {
            return maxprocs_ == this->number_of_spawned_processes();
        }
        /**
         * @brief Returns the intercommunicator between the original and the newly spawned group.
         * @return the intercommunicator
         * @nodiscard
         */
        [[nodiscard]]
        MPI_Comm intercommunicator() const noexcept {
            return intercomm_;
        }
        
    private:
        int maxprocs_;
        MPI_Comm intercomm_ = MPI_COMM_NULL;
    };


    /**
     * @brief Returns the parent intercommunicator of the current process if the process was started with
     *        [*MPI_Comm_spawn*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node237.htm) or
     *        [*MPI_Comm_spawn_multiple*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node238.htm).
     * @return a [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional) containing the parent intercommunicator or
     *         [`std::nullopt`](https://en.cppreference.com/w/cpp/utility/optional/nullopt)
     * @nodiscard
     */
    [[nodiscard]]
    inline std::optional<MPI_Comm> parent_process() {
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