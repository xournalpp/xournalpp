if (WIN32)

    function(configure_icon_generation icon_ico_var)
        find_package(ImageMagick REQUIRED COMPONENTS convert)
        set(ICON_SVG ${PROJECT_SOURCE_DIR}/ui/pixmaps/com.github.xournalpp.xournalpp.svg)
        set(ICON_ICO ${CMAKE_CURRENT_BINARY_DIR}/win32/xournalpp.ico)
        set(ICON_SIZES 16 32 48 256)

        foreach(SIZE IN LISTS ICON_SIZES)
            set(RESIZED_ICON ${PROJECT_BINARY_DIR}/xournalpp-icon-${SIZE}.png)
            add_custom_command(
                    OUTPUT ${RESIZED_ICON}
                    COMMAND rsvg-convert ${ICON_SVG} --width=${SIZE} --height=${SIZE} -o ${RESIZED_ICON}
                    DEPENDS ${ICON_SVG})
            set(ICON_DEPS ${ICON_DEPS} ${RESIZED_ICON})
        endforeach()

        add_custom_command(
                OUTPUT ${ICON_ICO}
                COMMAND convert ${ICON_DEPS} ${ICON_ICO} &&
                    ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/Xournalpp.cpp  # trigger relink if icon changed
                DEPENDS ${ICON_DEPS})

        # Return path to icon
        set(${icon_ico_var} ${ICON_ICO} PARENT_SCOPE)
    endfunction()

    function(configure_exe_rc rc_file_var)
        set(RC_FILE ${CMAKE_CURRENT_BINARY_DIR}/win32/xpp.rc)

        set(CMAKE_RC_COMPILER_INIT windres)
        enable_language(RC)
        set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -i <SOURCE> -o <OBJECT>")

        function(generate_win32_exe_comments var)
            string(APPEND COMMENTS "Build configuration: ${CMAKE_BUILD_TYPE}\\r\\n")
            string(APPEND COMMENTS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
            set(${var} ${COMMENTS} PARENT_SCOPE)
        endfunction()

        generate_win32_exe_comments(WIN32_EXE_COMMENTS)
        configure_file(win32/xpp.rc.in ${RC_FILE})

        # Return path to resource file
        set(${rc_file_var} ${RC_FILE} PARENT_SCOPE)
    endfunction()

    configure_icon_generation(ICON_ICO)
    add_custom_target(xournalpp_icon DEPENDS ${ICON_ICO})
    
    configure_exe_rc(RC_FILE)
    
    # Windows specific source files and dependencies
    file(GLOB win32_SOURCES win32/*.cpp win32/*.h)
    target_sources(xournalpp PRIVATE ${win32_SOURCES} ${RC_FILE})
    target_include_directories(xournalpp PRIVATE .)
    add_dependencies(xournalpp xournalpp_icon)

endif()
