#include <gtest/gtest.h>

#include <mpicxx/send/send.hpp>
#include <mpi.h>
#include <iostream>

TEST(StrCompare, ISendTest) {
    Send send;

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    std::cout << "World_size isend: " << world_size << std::endl;
    EXPECT_STREQ(send.world().c_str(), "World");
}
