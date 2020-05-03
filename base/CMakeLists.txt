file(GLOB_RECURSE xournalpp_sources ${CMAKE_CURRENT_SOURCE_DIR}/src *.cpp *.cc *.c)
file(GLOB_RECURSE xournalpp_include_pup ${CMAKE_CURRENT_SOURCE_DIR}/public *.hpp *.h)
file(GLOB_RECURSE xournalpp_include_priv ${CMAKE_CURRENT_SOURCE_DIR}/src *.hpp *.h)

## xournalpp main program ##
add_library(xournalpp_core STATIC ${xournalpp_sources})
target_include_directories(xournalpp_core PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public-header}>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
        )


add_executable(xournalpp exe/xournalppmain.cpp)
target_link_libraries(xournalpp PRIVATE xournalpp_core)

install(TARGETS xournalpp
        RUNTIME DESTINATION bin
        COMPONENT xournalpp)

