# GTK3_DIR

# General
# Todo: für alle pfade, suche gtk3-header and its version
# setup lib, and test if it works from the highest version to the lowest

# Smal subset:
# Do this for first found gtk3-header

# C:\gtk-build\gtk\x64\release\include\gtk-3.0\... :

find_package(Gtk3 CONFIG)
include(cmake-find-helpers)

if(TARGET Gtk3::gtk)
  find_debug_print(Gtk3::gtk)
  return()
endif()

find_pkg_config_package(Gtk3::gtk "gtk+-3.0")

if(TARGET Gtk3::gtk)
  find_debug_print(Gtk3::gtk)
  return()
endif()