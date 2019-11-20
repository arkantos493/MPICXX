# specify all source files for this test suite
set(TEST_SOURCES
    info_construction_test.cpp
)

# create google test with MPI support
add_mpi_test(info ${TEST_SOURCES} 1)
