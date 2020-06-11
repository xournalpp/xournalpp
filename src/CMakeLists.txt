## Build poppler ##

## Basic variables ##

include_directories (
  "${PROJECT_BINARY_DIR}/src"
  "${PROJECT_SOURCE_DIR}/src"
  "${PROJECT_SOURCE_DIR}/src/util"

  ${xournalpp_INCLUDE_DIRS}
)

# Build xournalpp-thumbnailer as it doesn't need CXX_FLAGS from below
add_subdirectory (xoj-preview-extractor)

# Used for both util and xournalpp targets
add_definitions (-g -Wreturn-type -Wuninitialized -Wunused-value -Wunused-variable)

if (WIN32)
  # optimize for size (the Windows .exe is really big)
  set (xournalpp_LDFLAGS ${xournalpp_LDFLAGS} "-Os -s")
elseif (APPLE OR LINUX)
  # Nothing to do for APPLE and linux
else ()
  set(xournalpp_LDFLAGS ${xournalpp_LDFLAGS} -rdynamic)
endif ()

option (DEBUG_COMPILE "Pass -Wall to CXX_FLAGS" OFF)
mark_as_advanced (FORCE DEBUG_COMPILE)

if (DEBUG_COMPILE)
  add_definitions (-Wall)
endif (DEBUG_COMPILE)

if (DEV_ENABLE_GCOV)
  add_definitions (--coverage)
endif (DEV_ENABLE_GCOV)

# Used to compile xournalpp and xournalpp-test
set (xournalpp_LDFLAGS
  util
  ${xournalpp_LDFLAGS}
)

## Build util lib ##
add_subdirectory (util)

## Sources ##

# These dirs are xournalpp only so it's safe to add then recursively
unset (xournalpp_SOURCES_RECURSE)
file (GLOB_RECURSE xournalpp_SOURCES_RECURSE
  control/*.cpp
  enums/*.cpp
  gui/*.cpp
  io/*.cpp
  model/*.cpp
  plugin/*.cpp
  undo/*.cpp
  view/*.cpp
)

# Here GLOB_RECURSE wouldn't be safe
unset (xournalpp_SOURCES)
file (GLOB xournalpp_SOURCES
  pdf/base/*.cpp
)

# Concatenate SOURCES lists
set (xournalpp_SOURCES ${xournalpp_SOURCES_RECURSE} ${xournalpp_SOURCES})
unset (xournalpp_SOURCES_RECURSE)

file (GLOB xournalpp_SOURCES_RECURSE
  pdf/popplerapi/*.cpp
)

# Concatenate SOURCES lists
set (xournalpp_SOURCES ${xournalpp_SOURCES_RECURSE} ${xournalpp_SOURCES})
unset (xournalpp_SOURCES_RECURSE)

## Core library ##

# Used for xournalpp and xournalpp-test
add_library (xournalpp-core OBJECT ${xournalpp_SOURCES})
add_dependencies (xournalpp-core util)
target_compile_features (xournalpp-core PUBLIC ${PROJECT_CXX_FEATURES})
# Can't use target_link_directories since we require CMake >= 3.10
target_include_directories (xournalpp-core PRIVATE $<TARGET_PROPERTY:util,INTERFACE_INCLUDE_DIRECTORIES>)

## xournalpp main program ##
add_executable (xournalpp
  $<TARGET_OBJECTS:xournalpp-core>
  Xournalpp.cpp
)

add_dependencies (xournalpp
  xournalpp-core
  util
)

target_link_libraries (xournalpp ${xournalpp_LDFLAGS})

install (TARGETS xournalpp
  RUNTIME DESTINATION bin
  COMPONENT xournalpp
)

if (ENABLE_CPPUNIT)
  add_subdirectory (${CMAKE_SOURCE_DIR}/test ${CMAKE_BINARY_DIR}/test)
endif (ENABLE_CPPUNIT)

