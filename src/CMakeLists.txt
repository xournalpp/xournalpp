cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

# Todo: add to target by generator expression


##############################
# xoj default option target
##############################

add_library(defaults INTERFACE)
target_include_directories(defaults INTERFACE ${PROJECT_BINARY_DIR}/src)
if (MSVC)
    target_compile_definitions(defaults INTERFACE NOMINMAX WIN32_LEAN_AND_MEAN)
elseif(GNU OR CLANG) 
    target_compile_options(defaults INTERFACE -Wall -Wreturn-type -Wuninitialized -Wunused-value -Wunused-variable -Wconversion)
endif ()
# Todo (support-v3.10): guard this variable with cmake_dependent_option for CMake 3.10, and NOT(GCC)
# then downgrade cmake_minimum_required to CMake 3.10
if (DEV_ENABLE_GCOV)
target_compile_options(defaults INTERFACE --coverage)
target_link_options(defaults INTERFACE --coverage) #cmake v3.13
endif (DEV_ENABLE_GCOV)

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

if (WIN32)
  # generate .ico from .png
  find_package(ImageMagick REQUIRED COMPONENTS convert)
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
  unset(ICON_SIZES)
  unset(ICON_DEPS)
  add_custom_target(xournalpp_icon DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/win32/xournalpp.ico)
  add_dependencies(xournalpp xournalpp_icon)
  
  # windows specific source files
  file (GLOB xournalpp-sources-win32 exe/win32/*.cpp exe/win32/*.h)
  target_sources(xournalpp PRIVATE ${xournalpp-sources-win32})
  target_compile_options(xournalpp PRIVATE -mwindows)
  target_include_directories(xournalpp PRIVATE exe)

  # windows specific appinfo
  #set(CMAKE_RC_COMPILER_INIT windres)
  enable_language(RC)
  #message(STATUS "CMAKE_RC_COMPILER ${CMAKE_RC_COMPILER_ID}")
  #set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -i <SOURCE> -o <OBJECT>")

  configure_file(exe/win32/xpp.rc.in ${CMAKE_CURRENT_BINARY_DIR}/win32/xpp.rc)
  target_sources(xournalpp PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/win32/xpp.rc)
endif ()
# Until here
################################

install(TARGETS xournalpp
        RUNTIME DESTINATION bin
        COMPONENT xournalpp
        )

add_subdirectory(xoj-preview-extractor)
