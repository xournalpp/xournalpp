find_program(CMAKE_GLIB_COMPILE_RESOURCES_PATH glib-compile-resources)

if (${CMAKE_GLIB_COMPILE_RESOURCES_PATH} EQUAL NOTFOUND)
    message(FATAL_ERROR "glib-compile-resources not found, make sure it is in the path")
endif ()

function(glib_compile_resources LIB_NAME CONFIG_DIR)
    file(GLOB_RECURSE AllFiles ${CONFIG_DIR} *.glade *.xml)

    string(TOLOWER ${LIB_NAME} OUT_PATH)
    set(OUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/res/${OUT_PATH})
    add_custom_command(
            OUTPUT ${OUT_PATH}.c ${OUT_PATH}.h
            COMMAND ${CMAKE_GLIB_COMPILE_RESOURCES_PATH} --target=${OUT_PATH} --generate-source --generate-header --sourcedir=${CONFIG_DIR}
            DEPENDS ${AllFiles}
    )

    add_library(${LIB_NAME} OBJECT ${OUT_PATH}.c ${OUT_PATH}.h)
    target_include_directories(${LIB_NAME} INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/res)

endfunction()