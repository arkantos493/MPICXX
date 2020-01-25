# specify all source files for this test suite
set(TEST_SOURCES
        env.cpp

        construction_and_destruction/default_construction.cpp
        construction_and_destruction/copy_construction.cpp
        construction_and_destruction/move_construction.cpp
        construction_and_destruction/iterator_range_construction.cpp
        construction_and_destruction/initializer_list_construction.cpp
        construction_and_destruction/mpi_info_construction.cpp

        assignment/copy_assignment.cpp
        assignment/move_assignment.cpp
        assignment/initializer_list_assignment.cpp

        capacity/empty.cpp
        capacity/size.cpp
        capacity/max_size.cpp

        modifier/at.cpp
        modifier/operator[].cpp
        modifier/clear.cpp
        modifier/swap.cpp

        lookup/count.cpp
        lookup/find.cpp
        lookup/contains.cpp
        lookup/equal_range.cpp

        non-member_functions/equality.cpp
        non-member_functions/inequality.cpp
        non-member_functions/swap.cpp
        non-member_functions/erase_if.cpp

        additional_functions/keys.cpp
        additional_functions/values.cpp
        additional_functions/max_key_size.cpp
        additional_functions/max_value_size.cpp

        info_iterator_test.cpp
        info_modifier_test.cpp
)

# create google test with MPI support
add_mpi_test(info "${TEST_SOURCES}" 1)
