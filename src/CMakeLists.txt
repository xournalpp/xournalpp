## Build poppler ##

add_subdirectory ("pdf/popplerdirect/workaround")

## Basic variables ##

include_directories (
  "${PROJECT_BINARY_DIR}/src"
  "${PROJECT_SOURCE_DIR}/src"
  "${PROJECT_SOURCE_DIR}/src/util"

  ${xournalpp_INCLUDE_DIRS}
)

# Build xournal-thumbnailer as it doesn't need CXX_FLAGS from below
add_subdirectory (xoj-preview-extractor)

# Used for both util and xournalpp targets
set (CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -g -rdynamic -Wreturn-type -Wuninitialized -Wunused-value -Wunused-variable"
)

option (DEBUG_COMPILE "Pass -Wall to CXX_FLAGS" OFF)
mark_as_advanced (FORCE DEBUG_COMPILE)

if (DEBUG_COMPILE)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif (DEBUG_COMPILE)

if (DEV_ENABLE_GCOV)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
endif (DEV_ENABLE_GCOV)

# Some package constants
set (CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -DPACKAGE_DATA_DIR=\\\"${PACKAGE_DATA_DIR}\\\" -DPACKAGE_LOCALE_DIR=\\\"${PACKAGE_LOCALE_DIR}\\\""
)

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
  gui/*.cpp
  model/*.cpp
  undo/*.cpp
  view/*.cpp
)

# Here GLOB_RECURSE wouldn't be safe
unset (xournalpp_SOURCES)
file (GLOB xournalpp_SOURCES
  pdf/cairo/*.cpp
  pdf/popplerdirect/*.cpp
  pdf/popplerdirect/poppler/*.cpp
)

# Concatenate SOURCES lists
set (xournalpp_SOURCES ${xournalpp_SOURCES_RECURSE} ${xournalpp_SOURCES})
list (REMOVE_ITEM xournalpp_SOURCES ${PROJECT_SOURCE_DIR}/src/model/LayerListener.cpp) # TOCHECK

## Core library ##

# Used for xournalpp and xournalpp-test
add_library (xournalpp-core OBJECT ${xournalpp_SOURCES})
add_dependencies (xournalpp-core util)

if (BUILD_BOOST)
  add_dependencies (xournalpp-core boost)
endif (BUILD_BOOST)

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

if (ENABLE_MATHTEX)
  add_subdirectory (mathtex)
endif (ENABLE_MATHTEX)
