# specify all source files for this test suite
set(TEST_SOURCES
        error_code.cpp
        error_category.cpp
)

# create google test with MPI support
add_mpi_test(error "${TEST_SOURCES}" 1)
