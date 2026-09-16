config_test = configuration_data()
config_test.set('PROJECT_SOURCE_DIR', meson.project_source_root())
configure_file(
    input: 'config-test.h.in',
    output: 'config-test.h',
    format: 'cmake@',
    configuration: config_test,
)

###############################################################################
# Define test-units
###############################################################################
xournalpp_test_sources = files(
    'unit_tests/control/ActionDatabaseTest.cpp',
    'unit_tests/control/LoadHandlerTest.cpp',
    'unit_tests/control/MetadataManagerTest.cpp',
    'unit_tests/control/SettingsTest.cpp',
    'unit_tests/control/ToolEnumsTest.cpp',
    'unit_tests/model/ColorPaletteTest.cpp',
    'unit_tests/model/DocumentNameTest.cpp',
    'unit_tests/model/ErasableStrokeTest.cpp',
    'unit_tests/model/ImageTest.cpp',
    'unit_tests/model/LineStyleTest.cpp',
    'unit_tests/model/StrokeStyleTest.cpp',
    'unit_tests/util/ColorTest.cpp',
    'unit_tests/util/ElementRangeTest.cpp',
    'unit_tests/util/I18nTest.cpp',
    'unit_tests/util/IntervalTest.cpp',
    'unit_tests/util/MatrixTest.cpp',
    'unit_tests/util/ObjectIOStreamTest.cpp',
    'unit_tests/util/PathTest.cpp',
    'unit_tests/util/RAIIWrappersTest.cpp',
    'unit_tests/util/RangeTest.cpp',
    'unit_tests/util/SaveNameUtilsTest.cpp',
    'unit_tests/util/StringUtilsTest.cpp',
    'unit_tests/util/TinySmallVectorTest.cpp',
    'unit_tests/util/XojPreviewExtractorTest.cpp',
    'main.cpp',
)

test_units_exe = executable(
    'test-units',
    sources: xournalpp_test_sources,
    dependencies: [
        xournalpp_defaults,
        xournalpp_util,
        xournalpp_core,
        xournalpp_external_modules,
        gtest,
    ],
    build_by_default: false,
)
test('test units', test_units_exe)

###############################################################################
# Define test-gtk-integration
###############################################################################
xournalpp_test_gtk_sources = files(
    'gtk_tests/dialog/GtkTest.cpp',
    'gtk_tests/dialog/SettingsDialogPaletteTabTest.cpp',
    'main.cpp',
)
test_gtk_integration_exe = executable(
    'test-gtk-integration',
    sources: xournalpp_test_gtk_sources,
    dependencies: [
        xournalpp_defaults,
        xournalpp_util,
        xournalpp_core,
        xournalpp_external_modules,
        gtest,
    ],
    build_by_default: false,
)

###############################################################################
# Discover and Register Tests
###############################################################################
# MacOs needs a bit longer to find the tests on azure
# Hence, set the timeout to 30s (default 5s)
# gtest_discover_tests(['test-units', 'DISCOVERY_TIMEOUT', '30', 'PROPERTIES', 'ENVIRONMENT', '${TEST_ENV_VARS}', '$<$<NOT:$<STREQUAL:${TEST_GDK_PIXBUF_LOADER_CACHE},', '', '>>:', 'DEPENDS', 'create-gdk-pixbuf-loader-cache>'])
# gtest_discover_tests(['test-gtk-integration', 'DISCOVERY_TIMEOUT', '30', 'PROPERTIES', 'ENVIRONMENT', '${TEST_ENV_VARS}', '$<$<NOT:$<STREQUAL:${TEST_GDK_PIXBUF_LOADER_CACHE},', '', '>>:', 'DEPENDS', 'create-gdk-pixbuf-loader-cache>'])


###############################################################################
# Define benchmarks: gtest-like app with benchmark units - not added to CTest or the `test` target
###############################################################################
xournalpp_benchmark_sources = files(
    'benchmarks/fileLoadBenchmarkTest.cpp',
    'main.cpp',
)
executable(
    'benchmark_run',
    sources: xournalpp_benchmark_sources,
    dependencies: [
        xournalpp_defaults,
        xournalpp_util,
        xournalpp_core,
        xournalpp_external_modules,
        gtest,
    ],
    override_options: ['buildtype=release'],
    build_by_default: false,
)
