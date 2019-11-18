# specify all source files for this test suite
set(TEST_SOURCES
    source_location_test.cpp
#    assert_test.cpp
)

# create google test with MPI support
add_mpi_test(utility "${TEST_SOURCES}" 1)
