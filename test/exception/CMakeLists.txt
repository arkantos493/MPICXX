# specify all source files for this test suite
set(TEST_SOURCES
        exception.cpp
)

# create google test with MPI support
add_mpi_test(exception "${TEST_SOURCES}" 1)
