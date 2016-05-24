## xournal-thumbnailer executable ##

add_executable (xournal-thumbnailer
  xournal-thumbnailer.cpp
  "${PROJECT_SOURCE_DIR}/src/util/XojPreviewExtractor.cpp"
)

target_link_libraries (xournal-thumbnailer
  ${Boost_LIBRARIES}
  ${Glibmm_LDFLAGS}
  ${ZLIB_LIBRARIES}
  ${Gnome_LDFLAGS}
)

if (BUILD_BOOST)
  add_dependencies (xournal-thumbnailer boost)
endif (BUILD_BOOST)

set (THUMBNAILER_BIN "xournal-thumbnailer")
if (WIN32)
  set (THUMBNAILER_BIN "${THUMBNAILER_BIN}.exe")
endif (WIN32)

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
