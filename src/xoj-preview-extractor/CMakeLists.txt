## xournalpp-thumbnailer executable ##

if (WIN32)
  # Windows does not support the thumbnailer like this
  return ()
endif ()

add_definitions (-DBUILD_THUMBNAILER)

add_executable (xournalpp-thumbnailer
  xournalpp-thumbnailer.cpp
  "${PROJECT_SOURCE_DIR}/src/util/GzUtil.cpp"
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

add_dependencies(xournalpp-thumbnailer std::filesystem)

target_link_libraries (xournalpp-thumbnailer
  ${ZLIB_LIBRARIES}
  ${Glib_LDFLAGS}
  ${ZIP_LDFLAGS}
  ${thumbnailer_GTK_LDFLAGS}
  std::filesystem
  util
)

set (THUMBNAILER_BIN "xournalpp-thumbnailer")

add_custom_command (TARGET xournalpp-thumbnailer POST_BUILD
  COMMAND ${CMAKE_STRIP} ${THUMBNAILER_BIN}
)

## Install ##

install (TARGETS xournalpp-thumbnailer
  RUNTIME DESTINATION bin
  COMPONENT xournalpp-thumbnailer
)

## Desktop install targets ##

configure_file (xournalpp-thumbnailer-xoj.schemas xournalpp-thumbnailer-xoj.schemas COPYONLY)

add_custom_target (desktop-install-thumbnailer
  COMMAND "${PROJECT_SOURCE_DIR}/src/xoj-preview-extractor/desktop-install.sh" install

  COMMENT "Install xournalpp-thumbnailer as xoj file thumbnail generator"
  SOURCES xournalpp-thumbnailer xournalpp-thumbnailer-xoj.schemas desktop-install.sh
)

add_custom_target (desktop-uninstall-thumbnailer
  COMMAND "${PROJECT_SOURCE_DIR}/src/xoj-preview-extractor/desktop-install.sh" uninstall

  COMMENT "Uninstall xournalpp-thumbnailer as xoj file thumbnail generator"
  SOURCES xournalpp-thumbnailer xournalpp-thumbnailer-xoj.schemas desktop-install.sh
)
