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
add_definitions(-g -Wreturn-type -Wuninitialized -Wunused-value -Wunused-variable -Wconversion)

if (NOT WIN32 AND (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
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

if (WIN32)
  # generate .ico from .png
  set (ICON_SIZES 16 32 48 256)
  foreach (SIZE IN LISTS ICON_SIZES)
    set (RESIZED_ICON ${PROJECT_BINARY_DIR}/xournalpp-icon-${SIZE}.png)
    add_custom_command (
	  OUTPUT ${RESIZED_ICON}
	  COMMAND rsvg-convert ${PROJECT_SOURCE_DIR}/ui/pixmaps/com.github.xournalpp.xournalpp.svg --width=${SIZE} --height=${SIZE} -o ${RESIZED_ICON}
	)
    set (ICON_DEPS ${ICON_DEPS} ${RESIZED_ICON})
	unset (RESIZED_ICON)
  endforeach()
  add_custom_command (
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/win32/xournalpp.ico
	COMMAND convert ${ICON_DEPS} ${CMAKE_CURRENT_BINARY_DIR}/win32/xournalpp.ico
	DEPENDS ${ICON_DEPS}
  )
  unset (ICON_SIZES)
  unset (ICON_DEPS)
  add_custom_target (xournalpp_icon DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/win32/xournalpp.ico)

  # Concatenate SOURCES lists
  set (xournalpp_SOURCES ${xournalpp_SOURCES_RECURSE} ${xournalpp_SOURCES})
  unset (xournalpp_SOURCES_RECURSE)

  file (GLOB xournalpp_SOURCES_RECURSE
    win32/*.cpp
  )
endif()

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

if (WIN32)
  set (CMAKE_RC_COMPILER_INIT windres)
  enable_language (RC)
  set (CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -i <SOURCE> -o <OBJECT>")
  add_dependencies(xournalpp xournalpp_icon)
  configure_file(win32/xpp.rc.in win32/xpp.rc)
  target_sources(xournalpp PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/win32/xpp.rc)
endif ()

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

