# specify all source files for this test suite
set(TEST_SOURCES
        version.cpp
)

# create google test with MPI support
add_mpi_test(version "${TEST_SOURCES}" 1)
