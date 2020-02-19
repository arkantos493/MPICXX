/**
 * @file include/mpicxx/startup/finalization.hpp
 * @author Marcel Breyer
 * @date 2020-02-19
 *
 * @brief Implements wrapper around the MPI finalization functions.
 */

#ifndef MPICXX_FINALIZATION_HPP
#define MPICXX_FINALIZATION_HPP

#include <mpi.h>


namespace mpicxx {

    inline void finalize() {
        MPI_Finalize();
    }

    [[nodiscard]] inline bool finalized() {
        int flag;
        MPI_Finalized(&flag);
        return static_cast<bool>(flag);
    }

}

#endif // MPICXX_FINALIZATION_HPP
