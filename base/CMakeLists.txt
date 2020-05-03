file(GLOB_RECURSE xournalpp_sources ${CMAKE_CURRENT_SOURCE_DIR}/src *.cpp *.cc *.c)
set(xournalpp_pup_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(xournalpp_priv_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)

## xournalpp main program ##
add_library(xournalpp_core STATIC ${xournalpp_sources})
target_include_directories(xournalpp_core PUBLIC
        $<BUILD_INTERFACE:${xournalpp_pup_include_dir}>
        PRIVATE ${xournalpp_priv_include_dir}
        )

target_link_libraries(xournalpp_core CXX Lager Threads xournalpp_deff PkgConfig::xournalpp_deps Boost::boost)

add_executable(xournalpp exe/xournalppmain.cpp)
target_link_libraries(xournalpp PRIVATE xournalpp_core)

install(TARGETS xournalpp
        RUNTIME DESTINATION bin
        COMPONENT xournalpp)
