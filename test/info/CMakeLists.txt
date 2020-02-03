# specify all source files for this test suite
set(TEST_SOURCES
        env.cpp

        constructor_and_destructor/default_constructor.cpp
        constructor_and_destructor/copy_constructor.cpp
        constructor_and_destructor/move_constructor.cpp
        constructor_and_destructor/iterator_range_constructor.cpp
        constructor_and_destructor/initializer_list_constructor.cpp
        constructor_and_destructor/mpi_info_constructor.cpp

        assignment/copy_assignment.cpp
        assignment/move_assignment.cpp
        assignment/initializer_list_assignment.cpp

        capacity/empty.cpp
        capacity/size.cpp
        capacity/max_size.cpp

        modifier/at.cpp
        modifier/array_subscript_operator.cpp
        modifier/clear.cpp
        modifier/erase.cpp
        modifier/extract.cpp
        modifier/merge.cpp
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

#        info_iterator_test.cpp
#        info_modifier_test.cpp
)

# create google test with MPI support
add_mpi_test(info "${TEST_SOURCES}" 1)
