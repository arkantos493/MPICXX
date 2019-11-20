### based on https://vicrucann.github.io/tutorials/quick-cmake-doxygen/ ###

# find doxygen
find_package(Doxygen REQUIRED dot)

# install doxyrest (so that the two config files don'T have to be changed)
set(DOXYREST_INSTALL_IN ${PROJECT_SOURCE_DIR}/third_party/install_doxyrest.sh)
set(DOXYREST_INSTALL_OUT ${CMAKE_CURRENT_BINARY_DIR}/install_doxyrest.sh)
configure_file(${DOXYREST_INSTALL_IN} ${DOXYREST_INSTALL_OUT} @ONLY)
execute_process(COMMAND sh ${DOXYREST_INSTALL_OUT})
set(DOXYREST_EXECUTABLE ${PROJECT_SOURCE_DIR}/third_party/doxyrest/bin/doxyrest)

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
    COMMAND
        ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    COMMAND
        ${DOXYREST_EXECUTABLE} -c ${DOXYREST_OUT}
    COMMAND
        ${SPHINX_EXECUTABLE} -q -b html -c ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/doc/rst_doxyrest ${PROJECT_SOURCE_DIR}/doc/html
    COMMENT
        "Generating API documentation with Doxygen -> Doxyrest -> Sphinx"
    VERBATIM
)

add_custom_target(doc DEPENDS doxygen.stamp)