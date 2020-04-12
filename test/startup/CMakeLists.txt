# specify all source files for this test suite
set(TEST_SOURCES
        finalize.cpp
        initialize.cpp
        thread_support.cpp
        single_spawner.cpp
        multi_spawner.cpp
)

# create google test with MPI support
add_mpi_test(startup "${TEST_SOURCES}" 2)
