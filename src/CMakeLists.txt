cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

# Todo: add to target by generator expression


##############################
# xoj default option target
##############################

add_library(defaults INTERFACE)
target_compile_options(defaults INTERFACE -Wall -Wreturn-type -Wuninitialized -Wunused-value -Wunused-variable -Wconversion)
target_include_directories(defaults INTERFACE ${PROJECT_BINARY_DIR}/src)
add_library(xoj::defaults ALIAS defaults)
if (MSVC)
  target_compile_definitions(defaults INTERFACE NOMINMAX WIN32_LEAN_AND_MEAN)
endif ()
# Todo (support-v3.10): guard this variable with cmake_dependent_option for CMake 3.10, and NOT(GCC)
# then downgrade cmake_minimum_required to CMake 3.10
if (DEV_ENABLE_GCOV)
  target_compile_options(defaults INTERFACE --coverage)
  target_link_options(defaults INTERFACE --coverage) #cmake v3.13
endif (DEV_ENABLE_GCOV)

##############################
# util
##############################

add_subdirectory(util)

##############################
# xournalpp-core
##############################

add_subdirectory(core)

##############################
# xournalpp main program
##############################


add_executable(xournalpp exe/Xournalpp.cpp)
target_link_libraries(xournalpp PRIVATE xoj::core xoj::util)
set_target_properties(xournalpp PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        MACOSX_BUNDLE $<IF:$<PLATFORM_ID:Darwin>,TRUE,${CMAKE_MACOSX_BUNDLE}>
        WIN32_EXECUTABLE $<IF:$<PLATFORM_ID:Windows>,TRUE,${CMAKE_WIN32_EXECUTABLE}>
        )

add_subdirectory(exe)

install(TARGETS xournalpp
        RUNTIME DESTINATION bin
        COMPONENT xournalpp
        )

add_subdirectory(xoj-preview-extractor)
