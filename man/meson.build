if host_machine.system() == 'linux' and get_option('WITH_MAN')
    help2man = find_program('help2man')
    man_include = meson.current_source_dir() / 'extra.groff'
    man_page = 'xournalpp.1'
    man_page_thumb = 'xournalpp-thumbnailer.1'
    xournalpp_man_page = custom_target('man_page',
        command : [help2man, '--no-info', '-i', man_include, '-o', meson.current_build_dir() / man_page, '--version-string=' + meson.project_version(), '--name="open-source hand note-taking program"', meson.project_build_root() / 'src/xournalpp'],
        depends : xournalpp_exe,
        output : man_page,
        install: true,
        install_dir : get_option('mandir') / 'man1'
    )
    xournalpp_thumb_man_page = custom_target('man_page_thumb',
        command : [help2man, '--no-info', '--no-discard-stderr', '-o', meson.current_build_dir() / man_page_thumb, '--version-string=' + meson.project_version(), '--name="creates thumbnails"', meson.project_build_root() / 'src/xoj-preview-extractor/xournalpp-thumbnailer'],
        depends : xournalpp_thumbnailer_exe,
        output : man_page_thumb,
        install : true,
        install_dir : get_option('mandir') / 'man1'
    )
    alias_target('manpage',
        xournalpp_man_page, xournalpp_thumb_man_page)
endif
