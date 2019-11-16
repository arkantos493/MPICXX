#include <gtest/gtest.h>

#include <mpicxx/reduce/reduce.hpp>
#include <mpi.h>
#include <iostream>

TEST(StrCompare, ReduceTest) {
    Reduce reduce;

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    std::cout << "World_size reduce: " << world_size << std::endl;
    EXPECT_STREQ(reduce.world().c_str(), "World");
}
