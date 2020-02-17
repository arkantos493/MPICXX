# specify all source files for this test suite
set(TEST_SOURCES
        clock.cpp
)

# create google test with MPI support
add_mpi_test(chrono "${TEST_SOURCES}" 1)
