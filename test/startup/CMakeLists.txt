# specify all source files for this test suite
set(TEST_SOURCES
        finalize.cpp
        initialize.cpp
)

# create google test with MPI support
add_mpi_test(startup "${TEST_SOURCES}" 1)
