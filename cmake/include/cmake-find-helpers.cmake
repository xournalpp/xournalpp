function(detail_handle_internal target_name search_name vcpkg_package_names)
    list(APPEND vcpkg_package_names ${ARGN})
    set(FOUND TRUE)

    message(STATUS "target_name: ${target_name}")
    message(STATUS "search_name: ${search_name}")
    message(STATUS "vcpkg_package_names: ${vcpkg_package_names}")

    foreach(vcpkg_package_name ${vcpkg_package_names})
        if(NOT TARGET ${vcpkg_package_name})
            message(STATUS "NOT TARGET ${vcpkg_package_name}")
            set(FOUND FALSE)
        endif()
    endforeach()

    message(STATUS "FOUND: ${FOUND}")

    if(FOUND)
        string(REGEX REPLACE "[^a-zA-Z0-9\-]+" "_" target_name_tmp ${target_name})
        message(STATUS "target_name_tmp: ${target_name_tmp}")
        string(CONCAT target_name_tmp _vcpkg_tmp)
        message(STATUS "target_name_tmp: ${target_name_tmp}")

        add_library(${target_name_tmp} INTERFACE)

        foreach(vcpkg_package_name ${vcpkg_package_names})
            message(STATUS "vcpkg_package_name: ${vcpkg_package_name}")
            target_link_libraries(${target_name_tmp} INTERFACE ${vcpkg_package_name})
        endforeach()

        message(STATUS "target_name: ${target_name}")
        add_library(${target_name} ALIAS ${target_name_tmp})
    endif()
endfunction()

function(find_vcpkg_package target_name vcpkg_search_name vcpkg_package_names)
    find_package(${vcpkg_search_name} CONFIG)
    list(APPEND vcpkg_package_names ${ARGN})
    detail_handle_internal(${target_name} ${vcpkg_search_name} ${vcpkg_package_names})
endfunction()

# pkg-config
function(find_pkg_config_package target_name pkg_config_search_name)
    message(STATUS "pkg-config: ${pkg_config_search_name}")
    message(STATUS "target: ${target_name}")
    find_package(PkgConfig)
    pkg_check_modules(${pkg_config_search_name}
        IMPORTED_TARGET
        ${pkg_config_search_name})

    if(TARGET PkgConfig::${pkg_config_search_name})
        add_library(${pkg_config_search_name}_pkg_int INTERFACE IMPORTED GLOBAL)
        target_link_libraries(${pkg_config_search_name}_pkg_int INTERFACE PkgConfig::${pkg_config_search_name})
        add_library(${target_name} ALIAS ${pkg_config_search_name}_pkg_int)
    endif()
endfunction()

function(find_debug_print targetname)
    get_target_property(${targetname}_header_path ${targetname} INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(${targetname}_library_path ${targetname} INTERFACE_LINK_LIBRARIES)

    message(STATUS "${targetname}_header_path = ${${targetname}_header_path}")
    message(STATUS "${targetname}_library_path = ${${targetname}_library_path}")
endfunction()
