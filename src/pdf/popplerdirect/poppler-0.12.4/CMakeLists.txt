project(poppler)

cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

include(PopplerDefaults)
include(PopplerMacros)
include(MacroOptionalFindPackage)
find_package(PkgConfig)
include(MacroEnsureVersion)
include(MacroBoolTo01)

set(POPPLER_MAJOR_VERSION "0")
set(POPPLER_MINOR_VERSION "12")
set(POPPLER_MICRO_VERSION "4")
set(POPPLER_VERSION "${POPPLER_MAJOR_VERSION}.${POPPLER_MINOR_VERSION}.${POPPLER_MICRO_VERSION}")

# command line switches
option(ENABLE_XPDF_HEADERS "Install unsupported xpdf headers." OFF)
option(BUILD_GTK_TESTS "Whether compile the GTK+ test programs." ON)
option(BUILD_QT3_TESTS "Whether compile the Qt3 test programs." ON)
option(BUILD_QT4_TESTS "Whether compile the Qt4 test programs." ON)
option(ENABLE_SPLASH "Build the Splash graphics backend." ON)
option(ENABLE_UTILS "Compile poppler command line utils." ON)
option(ENABLE_ABIWORD "Build the Abiword backend." ON)
option(ENABLE_LIBOPENJPEG "Use libopenjpeg for JPX streams." ON)
option(ENABLE_LCMS "Use liblcms for color management." ON)
option(ENABLE_ZLIB "TODO" OFF)
option(USE_EXCEPTIONS "Throw exceptions to deal with not enough memory and similar problems." OFF)
option(USE_FIXEDPOINT "Use fixed point arithmetic" OFF)

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")

# Enable these unconditionally.
set(MULTITHREADED ON)
set(OPI_SUPPORT ON)
set(TEXTOUT_WORD_LIST ON)

# setting the minimum required versions for some components
set(QT3_MIN_VERSION "3.3.0")
set(QT4_MIN_VERSION "4.4.0")
set(CAIRO_VERSION "1.8.4")

macro_bool_to_01(ENABLE_SPLASH HAVE_SPLASH)
find_package(Freetype REQUIRED)
find_package(Fontconfig REQUIRED)
macro_optional_find_package(JPEG)
macro_optional_find_package(PNG)
if(JPEG_FOUND)
  set(ENABLE_LIBJPEG ${JPEG_FOUND})
endif(JPEG_FOUND)
macro_optional_find_package(Qt3)
macro_optional_find_package(Qt4)
macro_optional_find_package(GTK)
if(GLIB_FOUND)
  macro_optional_find_package(Cairo)
  set(HAVE_CAIRO ${CAIRO_FOUND})
  set(POPPLER_WITH_GDK ${GDK_FOUND})
  if(CAIRO_FOUND)
    set(CAIRO_REQ "cairo")
  endif(CAIRO_FOUND)
endif(GLIB_FOUND)
if(ENABLE_ZLIB)
  find_package(ZLIB)
  if(ZLIB_FOUND)
    set(HAVE_LIBZ ${ZLIB_FOUND})
  endif(ZLIB_FOUND)
  set(ENABLE_ZLIB ${ZLIB_FOUND})
endif(ENABLE_ZLIB)
if(ENABLE_ABIWORD)
  find_package(LibXml2)
  set(ENABLE_ABIWORD ${LIBXML2_FOUND})
endif(ENABLE_ABIWORD)
if(ENABLE_LIBOPENJPEG)
  find_package(LIBOPENJPEG)
  set(ENABLE_LIBOPENJPEG ${LIBOPENJPEG_FOUND})
endif(ENABLE_LIBOPENJPEG)
if(ENABLE_LCMS)
  find_package(LCMS)
  set(USE_CMS ${LCMS_FOUND})
endif(ENABLE_LCMS)

add_definitions(-DHAVE_CONFIG_H=1 ${FONTCONFIG_DEFINITIONS})
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/fofi
  ${CMAKE_CURRENT_SOURCE_DIR}/goo
  ${CMAKE_CURRENT_SOURCE_DIR}/poppler
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/poppler
  ${FREETYPE_INCLUDE_DIRS}
)
set(HAVE_FREETYPE_H ON)

if(ENABLE_ZLIB)
  include_directories(${ZLIB_INCLUDE_DIR})
endif(ENABLE_ZLIB)
if(JPEG_FOUND)
  include_directories(${JPEG_INCLUDE_DIR})
  set(ENABLE_LIBJPEG ON)
endif(JPEG_FOUND)
if(PNG_FOUND)
  include_directories(${PNG_INCLUDE_DIR})
  set(ENABLE_LIBPNG ON)
endif(PNG_FOUND)
if(ENABLE_ABIWORD)
  include_directories(${LIBXML2_INCLUDE_DIR})
  add_definitions(${LIBXML2_DEFINITIONS})
endif(ENABLE_ABIWORD)
if(LIBOPENJPEG_FOUND)
  include_directories(${LIBOPENJPEG_INCLUDE_DIR})
endif(LIBOPENJPEG_FOUND)
if(LCMS_FOUND)
  include_directories(${LCMS_INCLUDE_DIR})
endif(LCMS_FOUND)

if(DEFINED COMPILE_WARNINGS)
else(DEFINED COMPILE_WARNINGS)
  set(COMPILE_WARNINGS "yes")
endif(DEFINED COMPILE_WARNINGS)
string(TOLOWER "${COMPILE_WARNINGS}" _comp_warnings)
if(_comp_warnings STREQUAL "no")
  add_definitions(${DEFAULT_COMPILE_WARNINGS_NO})
endif(_comp_warnings STREQUAL "no")
if(_comp_warnings STREQUAL "yes")
  add_definitions(${DEFAULT_COMPILE_WARNINGS_YES})
endif(_comp_warnings STREQUAL "yes")
if(_comp_warnings STREQUAL "kde")
  set(CMAKE_CXX_FLAGS "${DEFAULT_COMPILE_WARNINGS_KDE} ${CMAKE_CXX_FLAGS}")
endif(_comp_warnings STREQUAL "kde")


include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(poppler-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/poppler/poppler-config.h)


set(poppler_SRCS
  goo/gfile.cc
  goo/gmempp.cc
  goo/GooHash.cc
  goo/GooList.cc
  goo/GooTimer.cc
  goo/GooString.cc
  goo/gmem.cc
  goo/FixedPoint.cc
  goo/PNGWriter.cc
  goo/gstrtod.cc
  fofi/FoFiBase.cc
  fofi/FoFiEncodings.cc
  fofi/FoFiTrueType.cc
  fofi/FoFiType1.cc
  fofi/FoFiType1C.cc
  poppler/Annot.cc
  poppler/Array.cc
  poppler/BuiltinFont.cc
  poppler/BuiltinFontTables.cc
  poppler/Catalog.cc
  poppler/CharCodeToUnicode.cc
  poppler/CMap.cc
  poppler/DateInfo.cc
  poppler/Decrypt.cc
  poppler/Dict.cc
  poppler/Error.cc
  poppler/FileSpec.cc
  poppler/FontEncodingTables.cc
  poppler/Form.cc
  poppler/FontInfo.cc
  poppler/Function.cc
  poppler/Gfx.cc
  poppler/GfxFont.cc
  poppler/GfxState.cc
  poppler/GlobalParams.cc
  poppler/JArithmeticDecoder.cc
  poppler/JBIG2Stream.cc
  poppler/Lexer.cc
  poppler/Link.cc
  poppler/NameToCharCode.cc
  poppler/Object.cc
  poppler/OptionalContent.cc
  poppler/Outline.cc
  poppler/OutputDev.cc
  poppler/Page.cc
  poppler/PageTransition.cc
  poppler/Parser.cc
  poppler/PDFDoc.cc
  poppler/PDFDocEncoding.cc
  poppler/PopplerCache.cc
  poppler/ProfileData.cc
  poppler/PreScanOutputDev.cc
  poppler/PSTokenizer.cc
  poppler/Stream.cc
  poppler/UnicodeMap.cc
  poppler/UnicodeTypeTable.cc
  poppler/XRef.cc
  poppler/PSOutputDev.cc
  poppler/TextOutputDev.cc
  poppler/PageLabelInfo.cc
  poppler/SecurityHandler.cc
  poppler/Sound.cc
  poppler/XpdfPluginAPI.cc
  poppler/Movie.cc
)
set(poppler_LIBS ${FREETYPE_LIBRARIES} ${FONTCONFIG_LIBRARIES})
if(ENABLE_SPLASH)
  set(poppler_SRCS ${poppler_SRCS}
    poppler/SplashOutputDev.cc
    splash/Splash.cc
    splash/SplashBitmap.cc
    splash/SplashClip.cc
    splash/SplashFTFont.cc
    splash/SplashFTFontEngine.cc
    splash/SplashFTFontFile.cc
    splash/SplashFont.cc
    splash/SplashFontEngine.cc
    splash/SplashFontFile.cc
    splash/SplashFontFileID.cc
    splash/SplashPath.cc
    splash/SplashPattern.cc
    splash/SplashScreen.cc
    splash/SplashState.cc
    splash/SplashT1Font.cc
    splash/SplashT1FontEngine.cc
    splash/SplashT1FontFile.cc
    splash/SplashXPath.cc
    splash/SplashXPathScanner.cc
  )
endif(ENABLE_SPLASH)
if(JPEG_FOUND)
  set(poppler_SRCS ${poppler_SRCS}
    poppler/DCTStream.cc
  )
  set(poppler_LIBS ${poppler_LIBS} ${JPEG_LIBRARIES})
endif(JPEG_FOUND)
if(ENABLE_ZLIB)
  set(poppler_SRCS ${poppler_SRCS}
    poppler/FlateStream.cc
  )
  set(poppler_LIBS ${poppler_LIBS} ${ZLIB_LIBRARIES})
endif(ENABLE_ZLIB)
if(LIBOPENJPEG_FOUND)
  set(poppler_SRCS ${poppler_SRCS}
    poppler/JPEG2000Stream.cc
  )
  set(poppler_LIBS ${poppler_LIBS} ${LIBOPENJPEG_LIBRARIES})
else (LIBOPENJPEG_FOUND)
  set(poppler_SRCS ${poppler_SRCS}
    poppler/JPXStream.cc
  )
endif(LIBOPENJPEG_FOUND)
if(USE_CMS)
  set(poppler_LIBS ${poppler_LIBS} ${LCMS_LIBRARIES})
endif(USE_CMS)
if(ENABLE_ABIWORD)
  set(poppler_SRCS ${poppler_SRCS}
    poppler/ABWOutputDev.cc
  )
  set(poppler_LIBS ${poppler_LIBS} ${LIBXML2_LIBRARIES})
endif(ENABLE_ABIWORD)
if(WIN32)
  # gdi32 is needed under win32
  set(poppler_LIBS ${poppler_LIBS} gdi32)
endif(WIN32)

if(MSVC)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(poppler_SRCS ${poppler_SRCS} poppler/GlobalParamsWin.cc)
add_library(poppler STATIC ${poppler_SRCS})
else(MSVC)
add_library(poppler SHARED ${poppler_SRCS})
endif(MSVC)
set_target_properties(poppler PROPERTIES VERSION 5.0.0 SOVERSION 5)
if (ENABLE_LIBPNG)
target_link_libraries(poppler ${poppler_LIBS} ${PNG_LIBRARIES})
else (ENABLE_LIBPNG)
target_link_libraries(poppler ${poppler_LIBS})
endif (ENABLE_LIBPNG)
install(TARGETS poppler RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX} ARCHIVE DESTINATION lib${LIB_SUFFIX})

if(ENABLE_XPDF_HEADERS)
  install(FILES
    poppler/Annot.h
    poppler/Array.h
    poppler/BuiltinFont.h
    poppler/BuiltinFontTables.h
    poppler/Catalog.h
    poppler/CharCodeToUnicode.h
    poppler/CMap.h
    poppler/DateInfo.h
    poppler/Decrypt.h
    poppler/Dict.h
    poppler/Error.h
    poppler/FileSpec.h
    poppler/FontEncodingTables.h
    poppler/FontInfo.h
    poppler/Form.h
    poppler/Function.cc
    poppler/Function.h
    poppler/Gfx.h
    poppler/GfxFont.h
    poppler/GfxState.h
    poppler/GfxState_helpers.h
    poppler/GlobalParams.h
    poppler/JArithmeticDecoder.h
    poppler/JBIG2Stream.h
    poppler/Lexer.h
    poppler/Link.h
    poppler/Movie.h
    poppler/NameToCharCode.h
    poppler/Object.h
    poppler/OptionalContent.h
    poppler/Outline.h
    poppler/OutputDev.h
    poppler/Page.h
    poppler/PageTransition.h
    poppler/Parser.h
    poppler/PDFDoc.h
    poppler/PDFDocEncoding.h
    poppler/PopplerCache.h
    poppler/ProfileData.h
    poppler/PreScanOutputDev.h
    poppler/PSTokenizer.h
    poppler/Stream-CCITT.h
    poppler/Stream.h
    poppler/UnicodeMap.h
    poppler/UnicodeMapTables.h
    poppler/UnicodeTypeTable.h
    poppler/UnicodeCClassTables.h
    poppler/UnicodeCompTables.h
    poppler/UnicodeDecompTables.h
    poppler/XRef.h
    poppler/CharTypes.h
    poppler/CompactFontTables.h
    poppler/ErrorCodes.h
    poppler/NameToUnicodeTable.h
    poppler/PSOutputDev.h
    poppler/TextOutputDev.h
    poppler/SecurityHandler.h
    poppler/UTF8.h
    poppler/XpdfPluginAPI.h
    poppler/Sound.h
    ${CMAKE_CURRENT_BINARY_DIR}/poppler/poppler-config.h
    DESTINATION include/poppler)
  install(FILES
    goo/GooHash.h
    goo/GooList.h
    goo/GooTimer.h
    goo/GooMutex.h
    goo/GooString.h
    goo/GooVector.h
    goo/gtypes.h
    goo/gmem.h
    goo/gfile.h
    goo/FixedPoint.h
    goo/gstrtod.h
    DESTINATION include/poppler/goo)
  install(FILES
    fofi/FoFiBase.h
    fofi/FoFiEncodings.h
    fofi/FoFiTrueType.h
    fofi/FoFiType1.h
    fofi/FoFiType1C.h
    DESTINATION include/poppler/fofi)
  if(LIBOPENJPEG_FOUND)
    install(FILES
      poppler/JPEG2000Stream.h
      DESTINATION include/poppler)
  else(LIBOPENJPEG_FOUND)
    install(FILES
      poppler/JPXStream.h
      DESTINATION include/poppler)
  endif(LIBOPENJPEG_FOUND)
  if(ENABLE_SPLASH)
    install(FILES
      poppler/SplashOutputDev.h
      DESTINATION include/poppler)
    install(FILES
      splash/Splash.h
      splash/SplashBitmap.h
      splash/SplashClip.h
      splash/SplashErrorCodes.h
      splash/SplashFTFont.h
      splash/SplashFTFontEngine.h
      splash/SplashFTFontFile.h
      splash/SplashFont.h
      splash/SplashFontEngine.h
      splash/SplashFontFile.h
      splash/SplashFontFileID.h
      splash/SplashGlyphBitmap.h
      splash/SplashMath.h
      splash/SplashPath.h
      splash/SplashPattern.h
      splash/SplashScreen.h
      splash/SplashState.h
      splash/SplashT1Font.h
      splash/SplashT1FontEngine.h
      splash/SplashT1FontFile.h
      splash/SplashTypes.h
      splash/SplashXPath.h
      splash/SplashXPathScanner.h
      DESTINATION include/poppler/splash)
  endif(ENABLE_SPLASH)
endif(ENABLE_XPDF_HEADERS)


if(ENABLE_UTILS)
  add_subdirectory(utils)
endif(ENABLE_UTILS)
if(GLIB_FOUND)
  add_subdirectory(glib)
endif(GLIB_FOUND)
if(QT3_FOUND)
  add_subdirectory(qt)
endif(QT3_FOUND)
add_subdirectory(test)
if(QT4_FOUND)
  add_subdirectory(qt4)
endif(QT4_FOUND)

set(PKG_CONFIG_VERSION_0_18 TRUE)
if(PKGCONFIG_EXECUTABLE)
  exec_program(${PKGCONFIG_EXECUTABLE} ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _output_VAR)
  macro_ensure_version("0.18" "${_output_VAR}" PKG_CONFIG_VERSION_0_18)
endif(PKGCONFIG_EXECUTABLE)
if(PKG_CONFIG_VERSION_0_18)
  set(PC_REQUIRES "")
  set(PC_REQUIRES_PRIVATE "Requires.private: poppler = ${POPPLER_VERSION}")
else(PKG_CONFIG_VERSION_0_18)
  set(PC_REQUIRES "poppler = ${POPPLER_VERSION}")
  set(PC_REQUIRES_PRIVATE "")
endif(PKG_CONFIG_VERSION_0_18)

poppler_create_install_pkgconfig(poppler.pc lib${LIB_SUFFIX}/pkgconfig)
if(ENABLE_SPLASH)
  poppler_create_install_pkgconfig(poppler-splash.pc lib${LIB_SUFFIX}/pkgconfig)
endif(ENABLE_SPLASH)
if(QT3_FOUND)
  poppler_create_install_pkgconfig(poppler-qt.pc lib${LIB_SUFFIX}/pkgconfig)
endif(QT3_FOUND)
if(QT4_FOUND)
  poppler_create_install_pkgconfig(poppler-qt4.pc lib${LIB_SUFFIX}/pkgconfig)
endif(QT4_FOUND)
if(GLIB_FOUND)
  poppler_create_install_pkgconfig(poppler-glib.pc lib${LIB_SUFFIX}/pkgconfig)
endif(GLIB_FOUND)
if(CAIRO_FOUND)
  poppler_create_install_pkgconfig(poppler-cairo.pc lib${LIB_SUFFIX}/pkgconfig)
endif(CAIRO_FOUND)


message("Building Poppler with support for:")
show_end_message("splash output" ENABLE_SPLASH)
show_end_message("cairo output" CAIRO_FOUND)
show_end_message("abiword output" ENABLE_ABIWORD)
show_end_message("qt wrapper" QT3_FOUND)
show_end_message("qt4 wrapper" QT4_FOUND)
show_end_message("glib wrapper" GLIB_FOUND)
# message("  use gtk-doc:        $enable_gtk_doc") # TODO
show_end_message("use libjpeg" ENABLE_LIBJPEG)
show_end_message("use libpng" ENABLE_LIBPNG)
show_end_message("use zlib" ENABLE_ZLIB)
show_end_message("use libopenjpeg" LIBOPENJPEG_FOUND)
show_end_message("use cms" USE_CMS)
show_end_message("command line utils" ENABLE_UTILS)
