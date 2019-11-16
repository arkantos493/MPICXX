# specify all source files for this test suite
set(TEST_SOURCES
    send_test.cpp
    isend_test.cpp
)

# create google test with MPI support
add_mpi_test(send "${TEST_SOURCES}" 4)
