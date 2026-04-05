cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

add_library(xournalpp-optional-components-interface INTERFACE)
target_include_directories(xournalpp-optional-components-interface INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(xournalpp-load-runtime-library STATIC LibraryHandle.cpp LibraryHandle.h)
target_link_libraries(xournalpp-load-runtime-library PUBLIC xoj::util)
target_link_libraries(xournalpp-optional-components-interface INTERFACE xournalpp-load-runtime-library)

option(ENABLE_AUDIO "Compile with Audio recording/playing support" ON)
if (ENABLE_AUDIO)
    message(STATUS "Enable Audio support")
    pkg_check_modules(ExternalAudioModules REQUIRED IMPORTED_TARGET "portaudiocpp >= 12;sndfile >= 1.0.25")

    file(GLOB_RECURSE xournalpp_optional_runtime_deps_sources audio/*.cpp audio/*.h)

    add_library(xournalpp-audio SHARED ${xournalpp_optional_runtime_deps_sources})
    target_compile_features(xournalpp-audio PRIVATE ${PROJECT_CXX_FEATURES})
    target_compile_features(xournalpp-audio PRIVATE cxx_std_20)
    target_link_libraries(xournalpp-audio
            PRIVATE xoj::util
                    xoj::defaults
                    PkgConfig::ExternalAudioModules
            )
    target_include_directories(xournalpp-audio PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    add_dependencies(xournalpp-optional-components-interface INTERFACE xournalpp-audio)

    install(TARGETS xournalpp-audio)
else ()
    message(STATUS "Disable Audio support")
endif ()

add_library(xoj::optional-components ALIAS xournalpp-optional-components-interface)
