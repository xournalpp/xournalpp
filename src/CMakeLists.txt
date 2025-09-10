cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

# Todo: add to target by generator expression


##############################
# xoj default option target
##############################

add_library(defaults INTERFACE)
target_include_directories(defaults INTERFACE ${PROJECT_BINARY_DIR}/src)
add_library(xoj::defaults ALIAS defaults)
if (MSVC) # clang-cl.exe, cl.exe
  target_compile_definitions(defaults INTERFACE NOMINMAX WIN32_LEAN_AND_MEAN)
  target_compile_options(defaults INTERFACE -utf-8)
else(MSVC) # gnu clang with gnu driver, 
  target_compile_options(defaults INTERFACE -Wall -Wreturn-type -Wuninitialized -Wunused-value -Wunused-variable -Wconversion)
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
# xournalpp-wrapper: catches crashes
##############################

# The crash dialog's ui file is stored as gresource to make sure the wrapper is standalone
find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

set(XOURNAL_WRAPPER_GRESOURCE_C   xournalpp-wrapper.gresource.c)
set(XOURNAL_WRAPPER_GRESOURCE_XML exe/xournalpp-wrapper.gresource.xml)

add_custom_command(
    OUTPUT ${XOURNAL_WRAPPER_GRESOURCE_C}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${GLIB_COMPILE_RESOURCES}
    ARGS
        --generate-source
        --sourcedir=${CMAKE_SOURCE_DIR}
        --target=${CMAKE_CURRENT_BINARY_DIR}/${XOURNAL_WRAPPER_GRESOURCE_C}
        ${XOURNAL_WRAPPER_GRESOURCE_XML}
    VERBATIM
    MAIN_DEPENDENCY ${XOURNAL_WRAPPER_GRESOURCE_XML}
    DEPENDS
        ${CMAKE_SOURCE_DIR}/ui/crashDialog.glade
)

add_custom_target(
    xournalpp-wrapper-resource
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${XOURNAL_WRAPPER_GRESOURCE_C}
)

add_executable(xournalpp-wrapper exe/Xournalpp-wrapper.cpp ${CMAKE_CURRENT_BINARY_DIR}/${XOURNAL_WRAPPER_GRESOURCE_C})
set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/${XOURNAL_WRAPPER_GRESOURCE_C}
    PROPERTIES GENERATED TRUE
)

add_dependencies(xournalpp-wrapper xournalpp-wrapper-resource)
target_link_libraries(xournalpp-wrapper
        PUBLIC xoj::defaults xoj::util xoj::external_modules std::filesystem cxx17)
set_target_properties(xournalpp-wrapper PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        MACOSX_BUNDLE $<IF:$<PLATFORM_ID:Darwin>,TRUE,${CMAKE_MACOSX_BUNDLE}>
        WIN32_EXECUTABLE $<IF:$<PLATFORM_ID:Windows>,TRUE,${CMAKE_WIN32_EXECUTABLE}>
        )
target_include_directories(xournalpp-wrapper PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


##############################
# xournalpp main program
##############################

add_executable(xournalpp exe/Xournalpp.cpp)
target_link_libraries(xournalpp PRIVATE xoj::core xoj::util)
set_target_properties(xournalpp PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        MACOSX_BUNDLE $<IF:$<PLATFORM_ID:Darwin>,TRUE,${CMAKE_MACOSX_BUNDLE}>
        WIN32_EXECUTABLE $<IF:$<PLATFORM_ID:Windows>,TRUE,${CMAKE_WIN32_EXECUTABLE}>
        )

###############################
# Generate icons
# Todo(fabian): move this into an own windows specific cmake script
# e.g. exe/win32/CMakeLists.txt
###############################

###############################
# Add MacOS specific files
# Todo(Roland): consider moving this into an own MacOS specific cmake script
# e.g. exe/macos/CMakeLists.txt
##############################

if (APPLE)
  # MacOS specific source files
  file (GLOB xournalpp-sources-osx exe/osx/*.cpp exe/osx/*.h)
  target_sources(xournalpp PRIVATE ${xournalpp-sources-osx})
  target_include_directories(xournalpp PRIVATE exe)
endif()

if (WIN32)
  # generate .ico from .png
  find_package(ImageMagick REQUIRED COMPONENTS magick)
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
                COMMAND magick convert ${ICON_DEPS} ${CMAKE_CURRENT_BINARY_DIR}/win32/xournalpp.ico
                DEPENDS ${ICON_DEPS}
  )
  unset(ICON_SIZES)
  unset(ICON_DEPS)
  add_custom_target(xournalpp_icon DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/win32/xournalpp.ico)
  add_dependencies(xournalpp xournalpp_icon)
  add_dependencies(xournalpp-wrapper xournalpp_icon)
  
  # windows specific source files
  target_sources(xournalpp PRIVATE exe/win32/console.cpp)
  target_sources(xournalpp-wrapper PRIVATE exe/win32/console.cpp)
  if (MSVC)
    target_sources(xournalpp PRIVATE exe/win32/gui_main.cpp)
    target_sources(xournalpp-wrapper PRIVATE exe/win32/gui_main.cpp)
  endif()
  
  # windows specific appinfo
  if(NOT MSVC)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -i <SOURCE> -o <OBJECT>")
    set(CMAKE_RC_COMPILER_INIT windres)
  endif()
  enable_language(RC)

  configure_file(exe/win32/xpp.rc.in ${CMAKE_CURRENT_BINARY_DIR}/win32/xpp.rc)
  target_sources(xournalpp PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/win32/xpp.rc)
  target_sources(xournalpp-wrapper PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/win32/xpp.rc)
endif ()
# Until here
################################

install(TARGETS xournalpp-wrapper
        RUNTIME DESTINATION bin
        COMPONENT xournalpp-wrapper
        )
install(TARGETS xournalpp
        RUNTIME DESTINATION bin
        COMPONENT xournalpp
        )

add_subdirectory(xoj-preview-extractor)
