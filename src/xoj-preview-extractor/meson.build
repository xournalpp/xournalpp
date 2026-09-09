## xournalpp-thumbnailer executable ##
if get_option('ENABLE_THUMBNAILER') and not ['windows', 'cygwin'].contains(host_machine.system())
    xournalpp_thumbnailer_exe = executable(
        'xournalpp-thumbnailer',
        'xournalpp-thumbnailer.cpp',
        dependencies: [xournalpp_defaults, xournalpp_util, xournalpp_external_modules],
        install : true)
    # target_compile_features(['xournalpp-thumbnailer', 'PRIVATE', 'cxx_std_20'])
    # target_link_libraries(['xournalpp-thumbnailer', 'util', zlib_libraries, librsvg_ldflags, glib_ldflags, zip_ldflags])
    # target_include_directories(['xournalpp-thumbnailer', 'PRIVATE', librsvg_include_dirs])
    ## Install ##
    # install(['TARGETS', 'xournalpp-thumbnailer', 'RUNTIME', 'DESTINATION', 'bin', 'COMPONENT', 'xournalpp-thumbnailer'])
    ## Desktop install targets ##
    # configure_file(['xournalpp-thumbnailer-xoj.schemas', 'xournalpp-thumbnailer-xoj.schemas', 'COPYONLY'])
    # add_custom_target(['desktop-install-thumbnailer', 'COMMAND', '${PROJECT_SOURCE_DIR}/src/xoj-preview-extractor/desktop-install.sh', 'install', 'COMMENT', 'Install xournalpp-thumbnailer as xoj file thumbnail generator', 'SOURCES', 'xournalpp-thumbnailer', 'xournalpp-thumbnailer-xoj.schemas', 'desktop-install.sh'])
    # add_custom_target(['desktop-uninstall-thumbnailer', 'COMMAND', '${PROJECT_SOURCE_DIR}/src/xoj-preview-extractor/desktop-install.sh', 'uninstall', 'COMMENT', 'Uninstall xournalpp-thumbnailer as xoj file thumbnail generator', 'SOURCES', 'xournalpp-thumbnailer', 'xournalpp-thumbnailer-xoj.schemas', 'desktop-install.sh'])
endif
