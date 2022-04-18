## xournalpp-thumbnailer executable ##

if (WIN32)
  # Windows does not support the thumbnailer like this
  return ()
endif ()

add_definitions (-DBUILD_THUMBNAILER)

if (DEV_ENABLE_GCOV)
  add_compile_options("--coverage")
  add_link_options("--coverage")
endif (DEV_ENABLE_GCOV)

add_executable (xournalpp-thumbnailer
        xournalpp-thumbnailer.cpp
        "${PROJECT_SOURCE_DIR}/src/util/GzUtil.cpp"
        "${PROJECT_SOURCE_DIR}/src/util/PlaceholderString.cpp"
        "${PROJECT_SOURCE_DIR}/src/util/StringUtils.cpp"
        "${PROJECT_SOURCE_DIR}/src/util/PathUtil.cpp"
        "${PROJECT_SOURCE_DIR}/src/util/Stacktrace.cpp"
        "${PROJECT_SOURCE_DIR}/src/util/XojPreviewExtractor.cpp"
        )

add_dependencies(xournalpp-thumbnailer std::filesystem)

target_link_libraries (xournalpp-thumbnailer
  util
  std::filesystem
  ${ZLIB_LIBRARIES}
  ${librsvg_LDFLAGS}
  ${Glib_LDFLAGS}
  ${ZIP_LDFLAGS}
  )

target_include_directories (xournalpp-thumbnailer PRIVATE ${librsvg_INCLUDE_DIRS})

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
