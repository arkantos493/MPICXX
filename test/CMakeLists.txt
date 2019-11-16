# easily create mpi test cases
function(add_mpi_test name test_files num_mpi_procs)
    # add new test executable with the static main and the provided source test files
    add_executable(${name} ${CMAKE_SOURCE_DIR}/test/main.cpp ${test_files})
    # link test against own library and gtest
    target_link_libraries(${name} ${PROJECT_NAME} gtest)
    # add MPI parameter for the number of used processors
    set(test_parameters ${MPIEXEC_NUMPROC_FLAG} ${num_mpi_procs} "./${name}")
    # add test to run with MPI    
    add_test(NAME ${name} COMMAND ${MPIEXEC} ${test_parameters})
endfunction(add_mpi_test)


# add all test suites
add_subdirectory(send)
add_subdirectory(reduce)
