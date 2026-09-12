xournalpp_main_sources += 'exe/win32/console.cpp'
xournalpp_wrapper_sources += 'exe/win32/console.cpp'

if meson.get_compiler('cpp').get_id() == 'msvc'
    xournalpp_main_sources += 'exe/win32/gui_main.cpp'
    xournalpp_wrapper_sources += 'exe/win32/gui_main.cpp'
endif

image_magick = find_program('magick')

xournalpp_windows_icon = custom_target('ico',
    command: [image_magick, '-density', '300', '-define', 'icon:auto-resize=256,128,96,64,48,32,24,16', '-background', 'none', meson.project_source_root() / 'ui/pixmaps/com.github.xournalpp.xournalpp.svg', '@OUTPUT@'],
    output : 'xournalpp.ico'
)

rc_config = configuration_data()
rc_config.set('PROJECT_VERSION', meson.project_version())
version_without_suffix = meson.project_version().split('+')[0].split('-')[0].split('~')[0]
version_components = version_without_suffix.split('.')
rc_config.set('PROJECT_VERSION_MAJOR', version_components[0])
rc_config.set('PROJECT_VERSION_MINOR', version_components[1])
rc_config.set('PROJECT_VERSION_PATCH', version_components[2])

xournalpp_rc_file = configure_file(input : 'xpp.rc.in',
                                   output : 'xpp.rc',
                                   configuration: rc_config,
                                   format : 'cmake')

windows = import('windows')
xournalpp_windows_resouces = windows.compile_resources(
    xournalpp_rc_file,
    depends : xournalpp_windows_icon,
    include_directories : meson.current_build_dir()
)

xournalpp_main_sources += xournalpp_windows_resouces
xournalpp_wrapper_sources += xournalpp_windows_resouces
