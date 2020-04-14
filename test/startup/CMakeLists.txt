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
)

# create google test with MPI support
add_mpi_test(startup "${TEST_SOURCES}" 1)
