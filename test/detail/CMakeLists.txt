# specify all source files for this test suite
set(TEST_SOURCES
        assert.cpp
        source_location.cpp
)

# create google test with MPI support
add_mpi_test(detail "${TEST_SOURCES}" 1)
