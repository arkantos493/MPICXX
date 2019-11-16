# find doxygen
find_package(Doxygen REQUIRED dot)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")
# find doxyrest
find_package(Doxyrest REQUIRED)
# find Sphinx
find_package(Sphinx REQUIRED)

# configure doxygen files
set(DOXYGEN_IN Doxyfile.in)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.in)
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

# configure doxyrest files
set(DOXYREST_IN doxyrest-config.lua)
set(DOXYREST_OUT ${CMAKE_CURRENT_BINARY_DIR}/doxyrest-config.lua)
configure_file(${DOXYREST_IN} ${DOXYREST_OUT} @ONLY)

# configure sphinx files
set(SPHINX_CONFIG_IN conf.py)
set(SPHINX_CONFIG_OUT ${CMAKE_CURRENT_BINARY_DIR}/conf.py)
configure_file(${SPHINX_CONFIG_IN} ${SPHINX_CONFIG_OUT} @ONLY)

add_custom_command(
    OUTPUT
        doxygen.stamp
    DEPENDS
        Doxyfile.in
    COMMAND
        ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    COMMAND
        ${DOXYREST_EXECUTABLE} -c ${DOXYREST_OUT}
    COMMAND
        ${SPHINX_EXECUTABLE} -b html -c ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/doc/rst_doxyrest ${PROJECT_SOURCE_DIR}/doc/html
    COMMAND
        cmake -E touch doxygen.stamp
    COMMENT
        "Generating API documentation with Doxygen -> Doxyrest -> Sphinx"
    VERBATIM
)

add_custom_target(doc DEPENDS doxygen.stamp)

