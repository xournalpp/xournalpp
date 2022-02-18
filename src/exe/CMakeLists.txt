# Generate icons
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
  file (GLOB win32_SOURCES win32/*.cpp win32/*.h)
  target_sources(xournalpp PRIVATE ${win32_SOURCES})
  target_include_directories(xournalpp PRIVATE .)

  # windows specific appinfo
  set(CMAKE_RC_COMPILER_INIT windres)
  enable_language(RC)
  set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -i <SOURCE> -o <OBJECT>")

  configure_file(win32/xpp.rc.in ${CMAKE_CURRENT_BINARY_DIR}/win32/xpp.rc)
  target_sources(xournalpp PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/win32/xpp.rc)
endif ()
