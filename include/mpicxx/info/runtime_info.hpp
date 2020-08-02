/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-19
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Implements functions to query various information during runtime.
 */

#ifndef MPICXX_RUNTIME_INFO_HPP
#define MPICXX_RUNTIME_INFO_HPP

#include <mpi.h>

#include <optional>
#include <string>

namespace mpicxx {

    /**
     * @brief Returns the maximum possible number of processes.
     * @return a [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional) containing the maximum possible number of processes
     *         or [`std::nullopt`](https://en.cppreference.com/w/cpp/utility/optional/nullopt) if no value could be retrieved
     * @nodiscard
     *
     * @note It may be possible that less than `universe_size` processes can be spawned if processes are already running.
     *
     * @calls{ int MPI_Comm_get_attr(MPI_Comm comm, int comm_keyval, void *attribute_val, int *flag);    // exactly once }
     */
    [[nodiscard]]
    inline std::optional<int> universe_size() {
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
     * @brief Returns the name of the processor this code is currently running on.
     * @return the name of the processor this code is running on
     * @nodiscard
     *
     * @calls{ int MPI_Get_processor_name(char *name, int *resultlen);    // exactly once }
     */
    [[nodiscard]]
    inline std::string processor_name() {
        char name[MPI_MAX_PROCESSOR_NAME];
        int resultlen;
        MPI_Get_processor_name(name, &resultlen);
        return std::string(name, resultlen);
    }

}

#endif // MPICXX_RUNTIME_INFO_HPP