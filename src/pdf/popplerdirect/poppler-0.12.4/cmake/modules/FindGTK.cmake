# - try to find GTK (and glib)
# Once done this will define
#
#  GLIB_FOUND - system has GLib
#  GLIB_CFLAGS - the GLib CFlags
#  GLIB_LIBRARIES - Link these to use GLib
#
# Copyright 2008 Pino Toscano, <pino@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindPackageHandleStandardArgs)

if (NOT WIN32)
  include(UsePkgConfig)

  pkgconfig(glib-2.0 _LibGLIB2IncDir _LibGLIB2LinkDir GLIB2LinkFlags GLIB2Cflags)
  pkgconfig(gdk-2.0 _LibGDK2IncDir _LibGDK2LinkDir GDK2LinkFlags GDK2Cflags)
  set (GLIB_FOUND FALSE)
  if (_LibGLIB2IncDir)

    exec_program(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=2.6 glib-2.0 RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull)
    if(_return_VALUE STREQUAL "0")
      set(_glib_FOUND TRUE)
    endif(_return_VALUE STREQUAL "0")
  endif (_LibGLIB2IncDir)

  if (_LibGDK2IncDir)
    exec_program(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=2.4.0 gdk-2.0 RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull)
    if(_return_VALUE STREQUAL "0")
      set(_gdk_FOUND TRUE)
    endif(_return_VALUE STREQUAL "0")
  endif (_LibGDK2IncDir)

  if (_glib_FOUND)
    set (GLIB2_CFLAGS ${GLIB2Cflags})
    set (GLIB2_LIBRARIES ${GLIB2LinkFlags})
  endif (_glib_FOUND)

  if (_gdk_FOUND)
    set (GDK2_CFLAGS ${GDK2Cflags})
    set (GDK2_LIBRARIES ${GDK2LinkFlags})
  endif (_gdk_FOUND)

  find_package_handle_standard_args(GLib DEFAULT_MSG GLIB2_LIBRARIES GLIB2_CFLAGS)
  find_package_handle_standard_args(GDK DEFAULT_MSG GDK2_LIBRARIES GDK2_CFLAGS)

  pkgconfig(gtk+-2.0 _LibGTK2IncDir _LibGTK2LinkDir GTK2LinkFlags GTK2Cflags)
  pkgconfig(gdk-pixbuf-2.0 _LibGDK2PixbufIncDir _LibGDK2PixbufLinkDir GDK2PixbufLinkFlags GDK2PixbufCflags)
  pkgconfig(gthread-2.0 _LibGThread2IncDir _LibGThread2LinkDir GThread2LinkFlags GThread2Cflags)

  if (_LibGTK2IncDir AND _LibGDK2PixbufIncDir AND  _LibGThread2IncDir)
    exec_program(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=2.8.0 gtk+-2.0 RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull)
    if(_return_VALUE STREQUAL "0")
      set(_gtk_FOUND TRUE)
    endif(_return_VALUE STREQUAL "0")

    if (_gtk_FOUND)
      set (GTK2_CFLAGS ${GTK2Cflags} ${GDK2PixbufCflags} ${GThread2Cflags})
      set (GTK2_LIBRARIES ${GTK2LinkFlags} ${GDK2PixbufLinkFlags} ${GThread2LinkFlags})
    endif (_gtk_FOUND)

    find_package_handle_standard_args(GTK DEFAULT_MSG GTK2_LIBRARIES GTK2_CFLAGS)

  endif (_LibGTK2IncDir AND _LibGDK2PixbufIncDir AND _LibGThread2IncDir)

endif(NOT WIN32)

mark_as_advanced(
  GLIB2_CFLAGS
  GLIB2_LIBRARIES
  GDK2_CFLAGS
  GDK2_LIBRARIES
  GTK2_CFLAGS
  GTK2_LIBRARIES
)

