# specify all source files for this test suite
set(TEST_SOURCES
        finalize.cpp
        initialize.cpp
        thread_support.cpp

        single_spawner/constructor.cpp
        single_spawner/command.cpp
        single_spawner/maxprocs.cpp
        single_spawner/spawn_info.cpp
        single_spawner/argv.cpp
        single_spawner/root.cpp
        single_spawner/communicator.cpp

        multiple_spawner/constructor/iterator_range_constructor.cpp
        multiple_spawner/constructor/initializer_list_constructor.cpp
        multiple_spawner/constructor/parameter_pack_constructor.cpp
        multiple_spawner/constructor/spawner_constructor.cpp

        multiple_spawner/command/iterator_range.cpp
        multiple_spawner/command/initializer_list.cpp
        multiple_spawner/command/parameter_pack.cpp
        multiple_spawner/command/ith.cpp
        multiple_spawner/command/getter.cpp

        multiple_spawner/argvs/iterator_range.cpp
        multiple_spawner/argvs/initializer_list.cpp
        multiple_spawner/argvs/parameter_pack.cpp
        multiple_spawner/argvs/remove.cpp
        multiple_spawner/argvs/getter.cpp

        multiple_spawner/maxprocs/iterator_range.cpp
        multiple_spawner/maxprocs/initializer_list.cpp
        multiple_spawner/maxprocs/parameter_pack.cpp
        multiple_spawner/maxprocs/ith.cpp
        multiple_spawner/maxprocs/getter.cpp

        multiple_spawner/spawn_info/iterator_range.cpp
        multiple_spawner/spawn_info/initializer_list.cpp
        multiple_spawner/spawn_info/parameter_pack.cpp
        multiple_spawner/spawn_info/ith.cpp
        multiple_spawner/spawn_info/getter.cpp

        multiple_spawner/root.cpp
        multiple_spawner/communicator.cpp
        multiple_spawner/sizes.cpp
)

# create google test with MPI support
add_mpi_test(startup "${TEST_SOURCES}" 2)
