# Define the function
function(xoj_check_cxx_definition definition header variable)
    set(BINARY_DIR ${CMAKE_BINARY_DIR}/xoj_check_cxx_definition)
    set(SOURCE_FILE ${CMAKE_BINARY_DIR}/xoj_check_cxx_definition/${variable}.cpp)
    file(WRITE ${SOURCE_FILE} "#include <${header}> \n#if !(${definition}) \n#error \n#endif\nint main(){}")
    try_compile(${variable} 
                ${BINARY_DIR}
                ${SOURCE_FILE}
                CXX_STANDARD 20
                CXX_STANDARD_REQUIRED TRUE
                )
    if(${variable})
        message(STATUS "${header} defines ${definition}")
    else()
        message(STATUS "${header} does not define ${definition}")
    endif()
    file(REMOVE ${SOURCE_FILE})
endfunction()
