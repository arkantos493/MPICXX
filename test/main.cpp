#include <gtest/gtest.h>

#include <mpi.h>
#include "gtest-mpi-listener.hpp"

// based on https://github.com/LLNL/gtest-mpi-listener //

int main(int argc, char** argv) {
    // Filter out Google Test arguments
    ::testing::InitGoogleTest(&argc, argv);

    // Enabled death tests only if explicitly requested (broken with MPI)
    // -> normally disable them
    if (ENABLE_DEATH_TESTING == 0) {
        std::cout << "Disabled death tests" << std::endl;
        ::testing::GTEST_FLAG(filter) = "-*DeathTest.*";
    } else {
        std::cout << "Enabled death tests" << std::endl;
    }

    // Initialize MPI
    MPI_Init(&argc, &argv);

    // Add object that will finalize MPI on exit; Google Test owns this pointer
    ::testing::AddGlobalTestEnvironment(new GTestMPIListener::MPIEnvironment);

    // Get the event listener list.
    ::testing::TestEventListeners& listeners =
            ::testing::UnitTest::GetInstance()->listeners();

    // Remove default listener: the default printer and the default XML printer
    ::testing::TestEventListener *l =
            listeners.Release(listeners.default_result_printer());

    // Adds MPI listener; Google Test owns this pointer
    listeners.Append( new GTestMPIListener::MPIWrapperPrinter(l, MPI_COMM_WORLD) );

    // Run tests, then clean up and exit. RUN_ALL_TESTS() returns 0 if all tests
    // pass and 1 if some test fails.
    return RUN_ALL_TESTS();
}