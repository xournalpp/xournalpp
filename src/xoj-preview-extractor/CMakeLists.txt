## xournal-thumbnailer executable ##

if (WIN32)
  # Windows does not support the thumbniler like this
  return ()
endif ()

add_definitions (-DBUILD_THUMBNAILER)

add_executable (xournal-thumbnailer
  xournal-thumbnailer.cpp
  "${PROJECT_SOURCE_DIR}/src/util/GzUtil.cpp"
  "${PROJECT_SOURCE_DIR}/src/util/Path.cpp"
  "${PROJECT_SOURCE_DIR}/src/util/PlaceholderString.cpp"
  "${PROJECT_SOURCE_DIR}/src/util/StringUtils.cpp"
  "${PROJECT_SOURCE_DIR}/src/util/XojPreviewExtractor.cpp"
)

# MacOS workaround: "-framework ____" flags need to be passed as one string.
# See https://sourceforge.net/p/plplot/mailman/message/19574644/ for more details
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  string(REPLACE "-framework;" "-framework " thumbnailer_GTK_LDFLAGS "${GTK_LDFLAGS}")
else()
  set(thumbnailer_GTK_LDFLAGS "${GTK_LDFLAGS}")
endif()

target_link_libraries (xournal-thumbnailer
  ${ZLIB_LIBRARIES}
  ${Glib_LDFLAGS}
  ${ZIP_LDFLAGS}
  ${thumbnailer_GTK_LDFLAGS}
)

set (THUMBNAILER_BIN "xournal-thumbnailer")

add_custom_command (TARGET xournal-thumbnailer POST_BUILD
  COMMAND ${CMAKE_STRIP} ${THUMBNAILER_BIN}
)

## Install ##

install (TARGETS xournal-thumbnailer
  RUNTIME DESTINATION bin
  COMPONENT xournal-thumbnailer
)

## Desktop install targets ##

configure_file (xournal-thumbnailer-xoj.schemas xournal-thumbnailer-xoj.schemas COPYONLY)

add_custom_target (desktop-install-thumbnailer
  COMMAND "${PROJECT_SOURCE_DIR}/src/xoj-preview-extractor/desktop-install.sh" install

  COMMENT "Install xournal-thumbnailer as xoj file thumbnail generator"
  SOURCES xournal-thumbnailer xournal-thumbnailer-xoj.schemas desktop-install.sh
)

add_custom_target (desktop-uninstall-thumbnailer
  COMMAND "${PROJECT_SOURCE_DIR}/src/xoj-preview-extractor/desktop-install.sh" uninstall

  COMMENT "Uninstall xournal-thumbnailer as xoj file thumbnail generator"
  SOURCES xournal-thumbnailer xournal-thumbnailer-xoj.schemas desktop-install.sh
)
