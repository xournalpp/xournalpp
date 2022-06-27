
find_package(librsvg CONFIG QUIET)
include(cmake-find-helpers)

if(TARGET librsvg::librsvg)
    find_debug_print(librsvg::librsvg)
    return()
endif()

# vcpkg
set(vcpkg_libs unofficial::librsvg::librsvg-cpp;unofficial::librsvg::librsvg-private)
find_vcpkg_package(librsvg::librsvg unofficial-librsvg vcpkg_libs)

if(TARGET librsvg::librsvg)
    find_debug_print(librsvg::librsvg)
    return()
endif()

find_pkg_config_package(librsvg::librsvg "librsvg-2.0")

if(TARGET librsvg::librsvg)
    find_debug_print(librsvg::librsvg)
    return()
endif()