find_program(DOXYREST_EXECUTABLE NAMES doxyrest
    DOC "Doxyrest documentation generator"
)
 
include(FindPackageHandleStandardArgs)
 
find_package_handle_standard_args(Doxyrest DEFAULT_MSG
    DOXYREST_EXECUTABLE
)
 
mark_as_advanced(DOXYREST_EXECUTABLE)
