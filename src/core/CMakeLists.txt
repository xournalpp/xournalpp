cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

file(GLOB_RECURSE xournalpp_sources *.cpp *.h)
set(xournalpp_include_dirs "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(xournalpp-core STATIC ${xournalpp_sources})
target_compile_features(xournalpp-core PUBLIC ${PROJECT_CXX_FEATURES})
target_link_libraries(xournalpp-core
        PRIVATE xoj::util ${xournalpp_LDFLAGS}
        PUBLIC xoj::defaults xoj::external_modules std::filesystem cxx17
        )
target_include_directories(xournalpp-core PUBLIC ${xournalpp_include_dirs})
add_library(xoj::core ALIAS xournalpp-core)
