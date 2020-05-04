find_program(CMAKE_GLIB_COMPILE_RESOURCES_PATH glib-compile-resources)

if (${CMAKE_GLIB_COMPILE_RESOURCES_PATH} EQUAL NOTFOUND)
    message(FATAL_ERROR "glib-compile-resources not found, make sure it is in the path")
endif ()

function(glib_compile_resources LIB_NAME CONFIG_DIR XML_FILE)
    file(GLOB_RECURSE AllFiles ${CONFIG_DIR})

    string(TOLOWER ${LIB_NAME} OUT_NAME)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/res)
    set(OUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/res/${OUT_NAME}.c)
    add_custom_command(
            OUTPUT ${OUT_PATH}
            COMMAND ${CMAKE_GLIB_COMPILE_RESOURCES_PATH} --target=${OUT_PATH} --generate --sourcedir=${CONFIG_DIR} ${XML_FILE}
            DEPENDS ${AllFiles} ${XML_FILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/res
    )

    add_library(${LIB_NAME} OBJECT ${OUT_PATH})
    pkg_check_modules(glib_dep REQUIRED IMPORTED_TARGET GLOBAL
            "glib-2.0 >= 2.32.0")
    target_link_libraries(${LIB_NAME} PkgConfig::glib_dep)
    target_include_directories(${LIB_NAME} INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/res)

endfunction()