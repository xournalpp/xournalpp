/* poppler/poppler-config.h.  Generated from poppler-config.h.in by configure.  */
//================================================= -*- mode: c++ -*- ====
//
// poppler-config.h
//
// Copyright 1996-2004 Glyph & Cog, LLC
//
//========================================================================

#ifndef POPPLER_CONFIG_H
#define POPPLER_CONFIG_H

// We duplicate some of the config.h #define's here since they are
// used in some of the header files we install.  The #ifndef/#endif
// around #undef look odd, but it's to silence warnings about
// redefining those symbols.

/* Enable multithreading support. */
#ifndef MULTITHREADED
#define MULTITHREADED 1
#endif

/* Enable exceptions. */
#ifndef USE_EXCEPTIONS
/* #undef USE_EXCEPTIONS */
#endif

/* Use fixedpoint. */
#ifndef USE_FIXEDPOINT
/* #undef USE_FIXEDPOINT */
#endif

/* Include support for OPI comments. */
#ifndef OPI_SUPPORT
#define OPI_SUPPORT 1
#endif

/* Enable word list support. */
#ifndef TEXTOUT_WORD_LIST
#define TEXTOUT_WORD_LIST 1
#endif

// Also, there's a couple of preprocessor symbols in the header files
// that are used but never defined: DISABLE_OUTLINE, DEBUG_MEM and

//------------------------------------------------------------------------
// version
//------------------------------------------------------------------------

// copyright notice
#define popplerCopyright "Copyright 2005-2009 The Poppler Developers - http://poppler.freedesktop.org"
#define xpdfCopyright "Copyright 1996-2004 Glyph & Cog, LLC"

//------------------------------------------------------------------------
// popen
//------------------------------------------------------------------------

#if defined(_MSC_VER) || defined(__BORLANDC__)
#define popen _popen
#define pclose _pclose
#endif

#if defined(VMS) || defined(VMCMS) || defined(DOS) || defined(OS2) || defined(__EMX__) || defined(_WIN32) || defined(__DJGPP__) || defined(MACOS)
#define POPEN_READ_MODE "rb"
#else
#define POPEN_READ_MODE "r"
#endif

//------------------------------------------------------------------------
// Win32 stuff
//------------------------------------------------------------------------

#if defined(_WIN32) && !defined(_MSC_VER)
#include <windef.h>
#else
#define CDECL
#endif

//------------------------------------------------------------------------
// Compiler
//------------------------------------------------------------------------

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define GCC_PRINTF_FORMAT(fmt_index, va_index) \
	__attribute__((__format__(__printf__, fmt_index, va_index)))
#else
#define GCC_PRINTF_FORMAT(fmt_index, va_index)
#endif


#endif /* POPPLER_CONFIG_H */
