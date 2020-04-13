# specify all source files for this test suite
set(TEST_SOURCES
        finalize.cpp
        initialize.cpp
        thread_support.cpp
#        single_spawner.cpp
#        multi_spawner.cpp

        single_spawner/constructor.cpp
        single_spawner/command.cpp
        single_spawner/maxprocs.cpp
        single_spawner/spawn_info.cpp
)

# create google test with MPI support
add_mpi_test(startup "${TEST_SOURCES}" 2)
