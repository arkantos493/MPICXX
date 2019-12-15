# specify all source files for this test suite
set(TEST_SOURCES
        construction_and_destruction/default_construction.cpp
        construction_and_destruction/copy_construction.cpp
        construction_and_destruction/move_construction.cpp
        construction_and_destruction/iterator_range_construction.cpp
        construction_and_destruction/initializer_list_construction.cpp
        construction_and_destruction/mpi_info_construction.cpp

        info_env_test.cpp
        info_assignment_test.cpp
        info_access_test.cpp
        info_iterator_test.cpp
        info_capacity_test.cpp
        info_modifier_test.cpp
        info_lookup_test.cpp
        info_non-member_functions_test.cpp
        info_additional_functions_test.cpp
)

# create google test with MPI support
add_mpi_test(info "${TEST_SOURCES}" 1)
