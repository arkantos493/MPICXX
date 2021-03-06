### based on https://vicrucann.github.io/tutorials/quick-cmake-doxygen/ ###

# find doxygen
find_package(Doxygen REQUIRED dot)

# configure doxygen files
set(DOXYGEN_IN Doxyfile.in)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.out)
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)


add_custom_command(
    OUTPUT
        doxygen.stamp
    COMMAND
        ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/resources/copy_readme.cmake
    COMMAND
        ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    COMMAND
        ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/resources/append_to_file.cmake
    COMMENT
        "Generating API documentation with Doxygen"
    VERBATIM
)

add_custom_target(doc DEPENDS doxygen.stamp)