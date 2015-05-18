## xournal-thumbnailer executable ##

add_executable (xournal-thumbnailer
  xournal-thumbnailer.cpp
  "${PROJECT_SOURCE_DIR}/src/util/XojPreviewExtractor.cpp"
)

target_link_libraries (xournal-thumbnailer
  ${Boost_LIBRARIES}
  ${Glibmm_LDFLAGS}
)

add_custom_command (TARGET xournal-thumbnailer POST_BUILD
  COMMAND strip xournal-thumbnailer
)

# Simple C++11 check
target_compile_features (xournal-thumbnailer PRIVATE cxx_variadic_templates)

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
  COMMAND "${PROJECT_BINARY_DIR}/src/xoj-preview-extractor/desktop-install.sh" uninstall

  COMMENT "Uninstall xournal-thumbnailer as xoj file thumbnail generator"
  SOURCES xournal-thumbnailer xournal-thumbnailer-xoj.schemas desktop-install.sh
)
