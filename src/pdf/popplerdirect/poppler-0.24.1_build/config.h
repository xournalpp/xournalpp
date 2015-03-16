/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Build against libcurl. */
/* #undef ENABLE_LIBCURL */

/* Use libjpeg instead of builtin jpeg decoder. */
#define ENABLE_LIBJPEG 1

/* Use libopenjpeg instead of builtin jpeg2000 decoder. */
/* #undef ENABLE_LIBOPENJPEG */

/* Build against libpng. */
#define ENABLE_LIBPNG 1

/* Build against libtiff. */
#define ENABLE_LIBTIFF 1

/* Do not hardcode the library location */
/* #undef ENABLE_RELOCATABLE */

/* Use zlib instead of builtin zlib decoder. */
/* #undef ENABLE_ZLIB */

/* Use cairo for rendering. */
#define HAVE_CAIRO 1

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Have FreeType2 include files */
#define HAVE_FREETYPE_H 1

/* Define to 1 if you have the `fseek64' function. */
/* #undef HAVE_FSEEK64 */

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#define HAVE_FSEEKO 1

/* Define to 1 if you have the `ftell64' function. */
/* #undef HAVE_FTELL64 */

/* Defines if gettimeofday is available on your system */
#define HAVE_GETTIMEOFDAY 1

/* Defines if gmtime_r is available on your system */
#define HAVE_GMTIME_R 1

/* Define if you have the iconv() function and it works. */
#define HAVE_ICONV 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `tiff' library (-ltiff). */
/* #undef HAVE_LIBTIFF */

/* Define to 1 if you have the `z' library (-lz). */
/* #undef HAVE_LIBZ */

/* Defines if localtime_r is available on your system */
#define HAVE_LOCALTIME_R 1

/* Define to 1 if you have the `lseek64' function. */
#define HAVE_LSEEK64 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mkstemp' function. */
#define HAVE_MKSTEMP 1

/* Define to 1 if you have the `mkstemps' function. */
#define HAVE_MKSTEMPS 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <openjpeg.h> header file. */
/* #undef HAVE_OPENJPEG_H */

/* Define to 1 if you have the `popen' function. */
#define HAVE_POPEN 1

/* Define to 1 if you have the `pread64' function. */
#define HAVE_PREAD64 1

/* Define if you have POSIX threads libraries and header files. */
#define HAVE_PTHREAD 1

/* Have PTHREAD_PRIO_INHERIT. */
#define HAVE_PTHREAD_PRIO_INHERIT 1

/* Defines if rand_r is available on your system */
#define HAVE_RAND_R 1

/* Use splash for rendering. */
#define HAVE_SPLASH 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcat_s' function. */
/* #undef HAVE_STRCAT_S */

/* Define to 1 if you have the `strcpy_s' function. */
/* #undef HAVE_STRCPY_S */

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/mman.h> header file. */
#define HAVE_SYS_MMAN_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <tiffio.h> header file. */
#define HAVE_TIFFIO_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <zlib.h> header file. */
/* #undef HAVE_ZLIB_H */

/* Define as const if the declaration of iconv() needs const. */
#define ICONV_CONST 

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Enable multithreading support. */
#define MULTITHREADED 1

/* Generate OPI comments in PS output. */
#define OPI_SUPPORT 1

/* Name of package */
#define PACKAGE "poppler"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "https://bugs.freedesktop.org/enter_bug.cgi?product=poppler"

/* Define to the full name of this package. */
#define PACKAGE_NAME "poppler"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "poppler 0.24.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "poppler"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.24.1"

/* Poppler data dir */
#define POPPLER_DATADIR "/usr/local/share/poppler"

/* Support for curl based doc builder is compiled in. */
/* #undef POPPLER_HAS_CURL_SUPPORT */

/* Defines the poppler version */
#define POPPLER_VERSION "0.24.1"

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Enable word list support. */
#define TEXTOUT_WORD_LIST 1

/* Defines if use cms */
/* #undef USE_CMS */

/* Use fixed point arithmetic in the Splash backend */
/* #undef USE_FIXEDPOINT */

/* Use single precision arithmetic in the Splash backend */
/* #undef USE_FLOAT */

/* Defines if use lcms1 */
/* #undef USE_LCMS1 */

/* Version number of package */
#define VERSION "0.24.1"

/* Use fontconfig font configuration backend */
#define WITH_FONTCONFIGURATION_FONTCONFIG 1

/* Use win32 font configuration backend */
/* #undef WITH_FONTCONFIGURATION_WIN32 */

/* OpenJPEG with the OPJ_DPARAMETERS_IGNORE_PCLR_CMAP_CDEF_FLAG flag. */
/* #undef WITH_OPENJPEG_IGNORE_PCLR_CMAP_CDEF_FLAG */

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define to 1 if the X Window System is missing or not being used. */
/* #undef X_DISPLAY_MISSING */

/*
 * jpeg.h needs HAVE_BOOLEAN, when the system uses boolean in system
 * headers and I'm too lazy to write a configure test as long as only
 * unixware is related
 */
#ifdef _UNIXWARE
#define HAVE_BOOLEAN
#endif


/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */
