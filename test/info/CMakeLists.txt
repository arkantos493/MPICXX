# specify all source files for this test suite
set(TEST_SOURCES
        info_env_test.cpp
        info_construction_test.cpp
        info_assignment_test.cpp
        info_access_test.cpp
        info_iterator_test.cpp
        info_capacity_test.cpp
        info_lookup_test.cpp
        info_non-member_functions_test.cpp
)

# create google test with MPI support
add_mpi_test(info "${TEST_SOURCES}" 1)
