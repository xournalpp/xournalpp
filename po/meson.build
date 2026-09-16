# The source .desktop file - Only installed if DO_INSTALL_DESKTOP_FILE == true
xournalpp_desktop_in_file = meson.project_source_root() / 'desktop/com.github.xournalpp.xournalpp.desktop.in'
# An array whose entries have a source xml file and the subdirectory of datadir to install it in
# Only installed if DO_INSTALL_DESKTOP_FILE == true
xournalpp_xml_in_files_data = [
    {
        'file': meson.project_source_root() / 'resources-templates/com.github.xournalpp.xournalpp.appdata.xml.in',
        'install_subdir': 'metainfo',
    },
    {
        'file': meson.project_source_root() / 'resources-templates/com.github.xournalpp.xournalpp.xml.in',
        'install_subdir': 'mime/packages',
    },
]
# An array of source ini files
xournalpp_ini_in_files = [
    meson.project_source_root() / 'resources-templates/pagetemplates.ini.desktop.in',
    meson.project_source_root() / 'resources-templates/toolbar.ini.desktop.in',
]

xournalpp_xml_in_files = []
foreach d : xournalpp_xml_in_files_data
    xournalpp_xml_in_files += d['file']
endforeach

i18n = import('i18n')
fs = import('fs')

add_project_arguments(
    '-DGETTEXT_PACKAGE=' + meson.project_name(),
    '-DGETTEXT_PACKAGE_NAME=' + meson.project_name(),
    '-DGETTEXT_PACKAGE_VERSION=' + meson.project_version(),
    language: 'c',
)
config.set('GETTEXT_PACKAGE', meson.project_name())
config.set('ENABLE_NLS', true)

xgettext_default_args = [
    '--package-name=' + meson.project_name(),
    '--package-version=' + meson.project_version(),
    '--escape',
    '--from-code=UTF-8',
    '--add-comments=TRANSLATORS:',
    '--keyword=_F',
    '--keyword=C_F:1c,2',
    '--keyword=_',
    '--keyword=N_',
    '--keyword=C_:1c,2',
    '--keyword=NC_:1c,2',
]

xournalpp_i18n = i18n.gettext('xournalpp', args: xgettext_default_args)

xournalpp_languages_tgts = xournalpp_i18n[0]  # list of .mo files


if DO_INSTALL_DESKTOP_FILES
    foreach xml_file : xournalpp_xml_in_files_data
        xournalpp_languages_tgts += i18n.merge_file(
            input: xml_file['file'],
            po_dir: '.',
            output: fs.stem(xml_file['file']),
            install: true,
            install_dir: get_option('datadir') / xml_file['install_subdir'],
        )
    endforeach
    xournalpp_languages_tgts += i18n.merge_file(
        input: xournalpp_desktop_in_file,
        po_dir: '.',
        type: 'desktop',
        output: fs.stem(xournalpp_desktop_in_file),
        install: true,
        install_dir: get_option('datadir') / 'applications',
    )
endif

foreach ini_file : xournalpp_ini_in_files
    xournalpp_languages_tgts += i18n.merge_file(
        input: ini_file,
        po_dir: '.',
        type: 'desktop',
        output: fs.stem(fs.stem(ini_file)),  # remove .desktop.in
        install: true,
        install_dir: get_option('datadir') / meson.project_name() / 'ui',
    )
endforeach

alias_target('translations', xournalpp_languages_tgts)
