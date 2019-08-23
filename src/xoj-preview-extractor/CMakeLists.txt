## xournal-thumbnailer executable ##

if (WIN32)
    # Windows does not support the thumbniler like this
    return()
endif ()

add_definitions(-DBUILD_THUMBNAILER)

add_executable(xournal-thumbnailer
        xournal-thumbnailer.cpp
        "${PROJECT_SOURCE_DIR}/src/util/GzUtil.cpp"
        "${PROJECT_SOURCE_DIR}/src/util/Path.cpp"
        "${PROJECT_SOURCE_DIR}/src/util/PlaceholderString.cpp"
        "${PROJECT_SOURCE_DIR}/src/util/StringUtils.cpp"
        "${PROJECT_SOURCE_DIR}/src/util/XojPreviewExtractor.cpp"
        )

target_link_libraries(xournal-thumbnailer
        ${ZLIB_LIBRARIES}
        ${Glib_LDFLAGS}
        ${ZIP_LDFLAGS}
        )

set(THUMBNAILER_BIN "xournal-thumbnailer")

add_custom_command(TARGET xournal-thumbnailer POST_BUILD
        COMMAND ${CMAKE_STRIP} ${THUMBNAILER_BIN}
        )

## Install ##

install(TARGETS xournal-thumbnailer
        RUNTIME DESTINATION bin
        COMPONENT xournal-thumbnailer
        )

## Desktop install targets ##

configure_file(xournal-thumbnailer-xoj.schemas xournal-thumbnailer-xoj.schemas COPYONLY)

add_custom_target(desktop-install-thumbnailer
        COMMAND "${PROJECT_SOURCE_DIR}/src/xoj-preview-extractor/desktop-install.sh" install

        COMMENT "Install xournal-thumbnailer as xoj file thumbnail generator"
        SOURCES xournal-thumbnailer xournal-thumbnailer-xoj.schemas desktop-install.sh
        )

add_custom_target(desktop-uninstall-thumbnailer
        COMMAND "${PROJECT_SOURCE_DIR}/src/xoj-preview-extractor/desktop-install.sh" uninstall

        COMMENT "Uninstall xournal-thumbnailer as xoj file thumbnail generator"
        SOURCES xournal-thumbnailer xournal-thumbnailer-xoj.schemas desktop-install.sh
        )
