xournalpp_util_sources = [
'gtk4_helper.cpp',
'ElementRange.cpp',
'GtkUtil.cpp',
'GzUtil.cpp',
'Stacktrace.cpp',
'XojMsgBox.cpp',
'GzInputStream.cpp',
'Color.cpp',
'NamedColor.cpp',
'OutputStream.cpp',
'SaveNameUtils.cpp',
'PathUtil.cpp',
'Recolor.cpp',
'logger/Logger.cpp',
'PlaceholderString.cpp',
'raii/CStringWrapper.cpp',
'StringUtils.cpp',
'gdk4_helper.cpp',
'VersionInfo.cpp',
'Util.cpp',
'serializing/ObjectOutputStream.cpp',
'serializing/InputStreamException.cpp',
'serializing/HexObjectEncoding.cpp',
'serializing/ObjectEncoding.cpp',
'serializing/ObjectInputStream.cpp',
'serializing/BinObjectEncoding.cpp',
'ZipInputStream.cpp',
'XojPreviewExtractor.cpp',
'Assert.cpp',
'Range.cpp'
]

xournalpp_util_lib = static_library(
    'util',
    sources : xournalpp_util_sources,
    dependencies: [xournalpp_defaults, xournalpp_external_modules],
    include_directories: 'include'
    )

xournalpp_util = declare_dependency(
    link_with : xournalpp_util_lib,
    include_directories : 'include'
    )
