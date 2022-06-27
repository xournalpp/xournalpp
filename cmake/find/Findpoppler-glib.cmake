find_package(poppler-glib CONFIG QUIET)
include(cmake-find-helpers)

if(TARGET poppler::poppler-glib)
    find_debug_print(poppler::poppler)
    return()
endif()

# vcpkg
set(vcpkg_libs unofficial::poppler::poppler-cpp;unofficial::poppler::poppler-private)
find_vcpkg_package(poppler::poppler unofficial-poppler ${vcpkg_libs})

if(TARGET poppler::poppler)
    find_debug_print(poppler::poppler)
    return()
endif()

find_pkg_config_package(poppler::poppler "poppler")

if(TARGET poppler::poppler)
    find_debug_print(poppler::poppler)
    return()
endif()