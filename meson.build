## Also update changelog in debian folder!
project(
    'xournalpp',
    'cpp',
    'c',
    version: '1.3.7+dev',
    meson_version: '>= 0.61.2',
    default_options: [
        'default_library=static',
        'cpp_std=c++20',
        'buildtype=debugoptimized',
    ],
)

add_project_arguments(
    '-DGDK_DISABLE_DEPRECATED',
    '-DGTK_DISABLE_DEPRECATED',
    '-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_40',
    '-DGDK_VERSION_MIN_REQUIRED=GDK_VERSION_3_18',
    '-DG_LOG_DOMAIN="xopp"',
    language: ['c', 'cpp'],
)

fs = import('fs')

if host_machine.system() == 'windows'
    add_project_arguments('-D_USE_MATH_DEFINES', language: ['c', 'cpp'])  # To enable M_PI constant
    add_project_arguments('-DNOMINMAX', language: ['c', 'cpp'])  # Avoid Windows min/max clashing with std's
    add_project_arguments('-DWIN32_LEAN_AND_MEAN', language: ['c', 'cpp'])    # Reduce the size of windows includes
elif host_machine.system() == 'darwin'
    jhbuild_lock_entries = fs.read('mac-setup/jhbuild-version.lock').strip().split(
        '\n',
    )
    foreach e : jhbuild_lock_entries
        es = e.split('=')
        if es[0] == 'deployment_target'
            xournalpp_deployment_target = es[1]
            break
        endif
    endforeach
    # I don't know of a compiler-independent way of doing this...
    compiler = meson.get_compiler('cpp')
    if compiler.has_argument('-mmacosx-version-min=11.0')
        # e.g. gcc, xcode
        add_project_arguments(
            '-mmacosx-version-min=' + xournalpp_deployment_target,
            language: ['c', 'cpp'],
        )
    elif compiler.has_argument('-mmacos-version-min=11.0')
        # e.g. clang
        add_project_arguments(
            '-mmacos-version-min=' + xournalpp_deployment_target,
            language: ['c', 'cpp'],
        )
    else
        warning(
            'Unable to set MacOSX minimal version to ' + xournalpp_deployment_target,
        )
    endif
endif

DO_INSTALL_DESKTOP_FILES = get_option('INSTALL_DESKTOP_FILES') and host_machine.system() not in [
    'windows',
    'cygwin',
    'darwin',
]

config = configuration_data()
config.set('PROJECT_NAME', meson.project_name())
config.set('PROJECT_VERSION', meson.project_version())
config.set('PROJECT_HOMEPAGE_URL', 'https://xournalpp.github.io/')

# string(['TIMESTAMP', 'PACKAGE_TIMESTAMP', '%Y%m%d.%H%M', 'UTC'])

## I18n ##
subdir('po')

# # # if 'ONLY_CONFIGURE_FOR_TARGET_POT'
# # #   # We dont need anything else for just generating the .pot file.
# # #   # This avoids having to install all the dependencies in the CI runs updating the translation files
# # #   # return()
# # # endif

# configure_file(['cmake/postinst.in', 'cmake/postinst', '@ONLY'])

##############################
# other externals
##############################
deps = []
deps += dependency('glib-2.0', version: '>= 2.40.0')
deps += dependency('gtk+-3.0', version: '>= 3.18.9')
deps += dependency('poppler-glib', version: '>= 0.41.0')
if host_machine.system() == 'darwin'
    deps += dependency('poppler-cpp', version: '>= 0.41.0')
endif
deps += dependency('gthread-2.0', version: '>= 2.4.0')
deps += dependency('libxml-2.0', version: '>= 2.0.0')
deps += dependency('libzip', version: '>= 1.0.1')
deps += dependency('librsvg-2.0', version: '>= 2.46')
deps += dependency('zlib')
deps += dependency('threads')

# Profiling
if get_option('ENABLE_PROFILING')
    config.set('ENABLE_PROFILING', true)
    deps += dependency('libprofiler', version: '>= 2.5')
    deps += dependency('libtcmalloc', version: '>= 2.5')
endif

if get_option('ENABLE_AUDIO')
    config.set('ENABLE_AUDIO', true)
    deps += dependency('portaudiocpp', version: '>= 12')
    deps += dependency('sndfile', version: '>= 1.0.25')
endif

if get_option('ENABLE_QPDF')
    config.set('ENABLE_QPDF', true)
    deps += dependency(
        'libqpdf',
        modules: ['qpdf::libqpdf'],
        version: '>= 10.6.0',
    )
endif

if get_option('ENABLE_PLUGINS')
    config.set('ENABLE_PLUGINS', true)
    deps += dependency('Lua')
endif
if get_option('ENABLE_GTK_SOURCEVIEW')
    config.set('ENABLE_GTK_SOURCEVIEW', true)
    deps += dependency('gtksourceview-4', version: '>= 4.0.0')
endif

if get_option('ENABLE_X11')
    config.set('ENABLE_X11', true)
    deps += dependency('x11')
endif

if get_option('ENABLE_CPPTRACE') and get_option('buildtype') != 'release'
    config.set('ENABLE_CPPTRACE', true)
    if get_option('DOWNLOAD_CPPTRACE')
        cmake = import('cmake')
        cpptrace_opts = cmake.subproject_options()
        cpptrace_opts.add_cmake_defines(
            {
                'CMAKE_POLICY_DEFAULT_CMP0219': 'NEW',  # Escape \ in paths (for Windows)
                'CMAKE_POLICY_DEFAULT_CMP0156': 'NEW',  #
                'CMAKE_POLICY_DEFAULT_CMP0179': 'NEW',  #
                'CMAKE_POLICY_DEFAULT_CMP0126': 'NEW',  #
                'CMAKE_POLICY_DEFAULT_CMP0200': 'NEW',  #
                'CPPTRACE_BUILD_SHARED': false,
                'CPPTRACE_USE_EXTERNAL_ZSTD': true,
            },
        )
        if (host_machine.system() == 'linux')
            # Otherwise it'll pull libdwarf on top, and this doesn't work on ubuntu 22
            cpptrace_opts.add_cmake_defines(
                {
                    'CPPTRACE_GET_SYMBOLS_WITH_LIBDL': true,
                },
            )
        endif
        cpptrace_opts.set_install(false)  # We do not want to install the static libs...
        cpptrace_subproj = cmake.subproject('cpptrace', options: cpptrace_opts)

        deps += cpptrace_subproj.dependency('cpptrace-lib')
    else
        deps += dependency('cpptrace')
    endif
endif

xournalpp_external_modules = declare_dependency(
    dependencies: deps,
)

HAVE_FLOAT_FROM_CHARS = meson.get_compiler('cpp').compiles(
    '''
     #include <charconv>
     #include <string_view>
     #include <system_error>
     int main() {
         const std::string_view sv = "7.381450";
         double val{};
         auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), val);
         return ptr != (sv.data() + sv.size()) || ec != std::errc{};
     }
''',
)
if HAVE_FLOAT_FROM_CHARS
    if get_option('ENABLE_FLOAT_FROM_CHARS')
        config.set10('ENABLE_FLOAT_FROM_CHARS', true)
    else
        config.set10('ENABLE_FLOAT_FROM_CHARS', false)
        warning(
            'Floating point std::from_chars is disabled, but available.',
            'To avoid performance penalty, set ENABLE_FLOAT_FROM_CHARS to ON.',
        )
    endif
else
    config.set10('ENABLE_FLOAT_FROM_CHARS', false)
    if get_option('ENABLE_FLOAT_FROM_CHARS')
        error(
            'Floating point std::from_chars is not supported. Use a newer standard library,',
            'or set ENABLE_FLOAT_FROM_CHARS to OFF to fall back to g_ascii_strtod.',
            'std::from_chars offers higher performance and should be used whenever possible.',
        )
    endif
endif

# target_link_libraries(['external_modules', 'INTERFACE', 'PkgConfig::ExternalModules', '$<$<TARGET_EXISTS:cpptrace::cpptrace>:cpptrace::cpptrace>',
# target_link_options(['external_modules', 'INTERFACE', '$<$<AND:$<NOT:$<PLATFORM_ID:Windows>>,$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>>:-rdynamic>'])

# Advanced development config
config.set('DEV_TOOLBAR_CONFIG', 'toolbar.ini')  # Toolbar config file name
config.set('DEV_SETTINGS_XML_FILE', 'settings.xml')  # Settings file name
config.set('DEV_DEFAULT_PALETTE_FILE', 'xournal.gpl')  # Default Color Palette
config.set('DEV_PRINT_CONFIG_FILE', 'print-config.ini')  # Print config file name
config.set('DEV_METADATA_FILE', 'metadata.ini')  # Metadata file name
config.set('DEV_ERRORLOG_DIR', 'errorlogs')  # Directory where errorlogfiles will be placed
config.set('DEV_FILE_FORMAT_VERSION', 5)  # File format version

# Get git info
git = find_program('git', required: false, native: true)
if git.found() and fs.exists(meson.project_source_root() / '.git')
    config.set(
        'GIT_BRANCH',
        run_command(git, 'rev-parse', '--abbrev-ref', 'HEAD', check: true).stdout().strip(),
    )
    xournalpp_git_origin = run_command(
        git,
        'remote',
        'get-url',
        'origin',
        check: true,
    ).stdout().strip()

    config.set('GIT_ORIGIN_URL', xournalpp_git_origin)
    xournalpp_git_origin_components = xournalpp_git_origin.split(':')[1].split(
        '/',
    )

    config.set('GIT_ORIGIN_OWNER', xournalpp_git_origin_components[-2])
    config.set('GIT_ORIGIN_REPO', fs.stem(xournalpp_git_origin_components[-1]))

    xournalpp_commit_hash = run_command(
        git,
        'rev-parse',
        '--short=8',
        'HEAD',
        check: true,
    ).stdout().strip()
    if run_command(git, 'diff-index', '--quiet', 'HEAD', check: false).returncode() != 0
        xournalpp_commit_hash += '-dirty'
    endif

    config.set('RELEASE_IDENTIFIER', xournalpp_commit_hash)
else
    config.set('RELEASE_IDENTIFIER', meson.project_version())
endif

config.set(
    'PROJECT_CRASHREPORT',
    'https://github.com/xournalpp/xournalpp/issues/new?template=crash_report.yml',
)

# Generate VERSION file for packaging scripts
config.set('PACKAGE_ARCH', host_machine.cpu_family())
configure_file(
    input: 'cmake/VERSION.in',
    configuration: config,
    format: 'cmake@',
    output: 'VERSION',
)

## Source building ##
subdir('src')

## Tests ##
if get_option('ENABLE_GTEST')
    if get_option('DOWNLOAD_GTEST')
        cmake = import('cmake')
        gtest_opts = cmake.subproject_options()
        gtest_opts.add_cmake_defines(
            {
                'CMAKE_POLICY_DEFAULT_CMP0219': 'NEW',  # Escape \ in paths (for Windows)
                'CMAKE_POLICY_DEFAULT_CMP0156': 'NEW',  #
                'CMAKE_POLICY_DEFAULT_CMP0179': 'NEW',  #
                'CMAKE_POLICY_DEFAULT_CMP0126': 'NEW',  #
                'CMAKE_POLICY_DEFAULT_CMP0200': 'NEW',  #
                'CMAKE_POLICY_DEFAULT_CMP0128': 'NEW',
                'CMAKE_CXX_STANDARD': get_option('cpp_std').substring(3),
            },
        )
        gtest_opts.set_install(false)  # We do not want gtest to be installed
        gtest_subproj = cmake.subproject('gtest', options: gtest_opts)
        gtest = gtest_subproj.dependency('gtest')
    else
        gtest = dependency('gtest')
    endif
    subdir('test')
endif


## Man page generation ##
subdir('man')

# Install resources
install_subdir('ui', install_dir: get_option('datadir') / meson.project_name())
install_subdir(
    'plugins',
    install_dir: get_option('datadir') / meson.project_name(),
)
install_subdir(
    'resources',
    install_dir: get_option('datadir') / meson.project_name(),
)
install_subdir(
    'palettes',
    install_dir: get_option('datadir') / meson.project_name(),
)
# Install desktop shortcuts for Linux
if DO_INSTALL_DESKTOP_FILES
    install_data(
        'ui/pixmaps/com.github.xournalpp.xournalpp.svg',
        install_dir: get_option('datadir') / 'icons/hicolor/scalable/apps',
    )
    install_data(
        'ui/pixmaps/application-x-xopp.svg',
        install_dir: get_option('datadir') / 'icons/hicolor/scalable/mimetypes',
    )
    install_data(
        'ui/pixmaps/application-x-xopt.svg',
        install_dir: get_option('datadir') / 'icons/hicolor/scalable/mimetypes/',
    )
    install_data(
        'ui/pixmaps/application-x-xojpp.svg',
        install_dir: get_option('datadir') / 'icons/hicolor/scalable/mimetypes/',
    )
    install_data(
        'ui/pixmaps/gnome-mime-application-x-xopp.svg',
        install_dir: get_option('datadir') / 'icons/hicolor/scalable/mimetypes/',
    )
    install_data(
        'ui/pixmaps/gnome-mime-application-x-xopt.svg',
        install_dir: get_option('datadir') / 'icons/hicolor/scalable/mimetypes/',
    )
    install_data(
        'desktop/com.github.xournalpp.xournalpp.thumbnailer',
        install_dir: get_option('datadir') / 'thumbnailers',
    )
endif
