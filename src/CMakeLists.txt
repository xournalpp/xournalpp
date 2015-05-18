## Build poppler ##

add_subdirectory ("pdf/popplerdirect")
link_directories ("${PROJECT_SOURCE_DIR}/src/pdf/popplerdirect")

## Basic variables ##

include_directories (
  "${PROJECT_BINARY_DIR}/src"
  "${PROJECT_SOURCE_DIR}/src"
  "${PROJECT_SOURCE_DIR}/src/util"
  "${PROJECT_SOURCE_DIR}/src/pdf/popplerdirect/poppler-0.24.1_build/poppler"
  "${PROJECT_SOURCE_DIR}/src/pdf/popplerdirect/poppler-0.24.1"

  ${Gnome_INCLUDE_DIRS}
  ${Glibmm_INCLUDE_DIRS}
  ${OtherLibs_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OPENJPEG_INCLUDE_DIR}
  ${JPEG_INCLUDE_DIR}
)

# Build xournal-thumbnailer as it doesn't need CXX_FLAGS from below
add_subdirectory (xoj-preview-extractor)

option (DEBUG_COMPILE "Pass -Wall to CXX_FLAGS" OFF)
mark_as_advanced (FORCE DEBUG_COMPILE)

# Used for both util and xournalpp targets
set (CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -g -rdynamic -Wreturn-type -Wuninitialized -Wunused-value -Wunused-variable"
)

if (DEBUG_COMPILE)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif (DEBUG_COMPILE)

# Some package constants
set (CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -DPACKAGE_DATA_DIR=\\\"${PACKAGE_DATA_DIR}\\\" -DPACKAGE_LOCALE_DIR=\\\"${PACKAGE_LOCALE_DIR}\\\""
)

# Used to compile xournalpp and xournalpp-test
set (xournalpp_LDFLAGS
  util
  libpoppler.a
  libpoppler-glib.a

  ${Gnome_LDFLAGS}
  ${Glibmm_LDFLAGS}
  ${OtherLibs_LDFLAGS}
  ${Boost_LIBRARIES}
  ${OPENJPEG_LIBRARIES}
  ${JPEG_LIBRARIES}
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

add_dependencies (xournalpp-core
  poppler
  util
)

# Simple C++11 check
target_compile_features (xournalpp-core PRIVATE cxx_variadic_templates)

## xournalpp main program ##
add_executable (xournalpp
  $<TARGET_OBJECTS:xournalpp-core>
  Xournalpp.cpp
)

add_dependencies (xournalpp
  poppler
  util
)

target_link_libraries (xournalpp ${xournalpp_LDFLAGS})

# Simple C++11 check
target_compile_features (xournalpp PRIVATE cxx_variadic_templates)

install (TARGETS xournalpp
  RUNTIME DESTINATION bin
  COMPONENT xournalpp
)

if (ENABLE_CPPUNIT)
  add_subdirectory(${CMAKE_SOURCE_DIR}/test ${CMAKE_BINARY_DIR}/test)
endif (ENABLE_CPPUNIT)

if (ENABLE_MATHTEX)
  add_subdirectory (mathtex)
endif (ENABLE_MATHTEX)
