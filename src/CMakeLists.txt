cmake_minimum_required(VERSION 3.11)

# Todo: add to target by generator expression
#if (DEV_ENABLE_GCOV)
#    add_definitions(--coverage)
#endif (DEV_ENABLE_GCOV)

##############################
# xoj default option target
##############################
add_library(defaults INTERFACE)
target_compile_options(defaults INTERFACE -Wall -Wreturn-type -Wuninitialized -Wunused-value -Wunused-variable)
target_include_directories(defaults INTERFACE ${PROJECT_BINARY_DIR}/src)
add_library(xoj::defaults ALIAS defaults)

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

add_executable(xournalpp core/Xournalpp.cpp)
target_link_libraries(xournalpp PRIVATE xoj::core)

install(TARGETS xournalpp
        RUNTIME DESTINATION bin
        COMPONENT xournalpp
        )

if (ENABLE_CPPUNIT)
    add_subdirectory(${CMAKE_SOURCE_DIR}/test ${CMAKE_BINARY_DIR}/test)
endif (ENABLE_CPPUNIT)

add_subdirectory(xoj-preview-extractor)