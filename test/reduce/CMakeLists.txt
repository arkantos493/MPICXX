# specify all source files for this test suite
set(TEST_SOURCES
    reduce_test.cpp
)

# create google test with MPI support
add_mpi_test(reduce ${TEST_SOURCES} 4)
