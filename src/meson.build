
configure_file(input : 'config.h.meson.in', output : 'config.h', configuration: config)
configure_file(input : 'config-debug.h.meson.in', output : 'config-debug.h', configuration: config)
configure_file(input : 'config-dev.h.meson.in', output : 'config-dev.h', configuration: config)
configure_file(input : 'config-git.h.meson.in', output : 'config-git.h', configuration: config)
configure_file(input : 'config-paths.h.meson.in', output : 'config-paths.h', configuration: config)
configure_file(input : 'config-features.h.meson.in', output : 'config-features.h', configuration: config)

xournalpp_defaults = declare_dependency(
    include_directories : '.'
)
##############################
# util
##############################
subdir('util')
##############################
# xournalpp-core
##############################
subdir('core')

##############################
# xournalpp-wrapper: catches crashes
##############################
# The crash dialog's ui file is stored as gresource to make sure the wrapper is standalone
gnome = import('gnome')
xournalpp_wrapper_resource = gnome.compile_resources(
    'xournalpp_wrapper_resource',
    'exe/xournalpp-wrapper.gresource.xml',
    source_dir : meson.project_source_root())

xournalpp_wrapper_sources = [
    xournalpp_wrapper_resource,
    'exe/Xournalpp-wrapper.cpp'
]
xournalpp_main_sources = ['exe/Xournalpp.cpp']

# This will add sources to the arrays
if host_machine.system() == 'darwin'
    subdir('exe/osx')
elif ['windows', 'cygwin'].contains(host_machine.system())
    subdir('exe/win32')
endif

xournalpp_wrapper_exe = executable(
    'xournalpp-wrapper',
    sources : xournalpp_wrapper_sources,
    dependencies: [xournalpp_defaults, xournalpp_util, dependency('glib-2.0'), dependency('gtk+-3.0')],
    install : true)

##############################
# xournalpp main program
##############################
xournalpp_exe = executable(
    'xournalpp',
    sources : xournalpp_main_sources,
    dependencies : [xournalpp_defaults, xournalpp_util, xournalpp_core, xournalpp_external_modules],
    install : true
    )
# target_link_libraries(['xournalpp', 'PRIVATE', 'xoj::core', 'xoj::util'])
# set_target_properties(['xournalpp', 'PROPERTIES', 'RUNTIME_OUTPUT_DIRECTORY', '${CMAKE_BINARY_DIR}', 'MACOSX_BUNDLE', '$<IF:$<PLATFORM_ID:Darwin>,TRUE,${CMAKE_MACOSX_BUNDLE}>', 'WIN32_EXECUTABLE', '$<IF:$<PLATFORM_ID:Windows>,TRUE,${CMAKE_WIN32_EXECUTABLE}>'])

subdir('xoj-preview-extractor')
