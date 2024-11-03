#
# cmake/Gettext.cmake
# Copyright (C) 2012, 2013, Valama development team
#
# Valama is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Valama is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.
#
##
#
# Heavily based on Jim Nelson's Gettext.cmake in Geary project:
# https://github.com/ypcs/geary
#
##
#
# With modifications by Bryan Tan to fix an issue with the translation target.
#
##
# Add find_package handler for gettext programs msgmerge, msgfmt, msgcat and
# xgettext.
##
# Constant:
# XGETTEXT_OPTIONS_DEFAULT: Provide common xgettext options.
# XGETTEXT_VALA_OPTIONS_DEFAULT: Provide common xgettext options for Vala files.
# XGETTEXT_GLADE_OPTIONS_DEFAULT: Provide common xgettext options for glade.
##
# The gettext_create_pot macro creates .pot files with xgettext from multiple
# source files.
# Provide target 'pot' to generate .pot file.
#
# Supported sections:
#
# PACKAGE (optional)
#   Gettext package name. Get exported to parent scope.
#   Default: ${PROJECT_NAME}
#
# VERSION (optional)
#   Gettext package version. Get exported to parent scope.
#   Default: ${${GETTEXT_PACKAGE_NAME}_VERSION}
#   (${GETTEXT_PACKAGE_NAME} is package name from option above)
#
# OPTIONS (optional)
#   Pass list of xgettext options (you can use XGETTEXT_OPTIONS_DEFAULT,
#   XGETTEXT_VALA_OPTIONS_DEFAULT and XGETTEXT_GLADE_OPTIONS_DEFAULT
#   constants).
#   Default: ${XGETTEXT_{,VALA,GLADE}_OPTIONS_DEFAULT}
#
# SRCFILES (optional/mandatory)
#   List of source files to extract gettext strings from. Globbing is
#  supported.
#
# GLADEFILES (optional/mandatory)
#   List of glade source files to extract gettext strings from. Globbing is
#   supported.
#
# DESKTOPFILES (optional/mandatory)
#   List of .desktop files to extract gettext strings from. Globbing is
#   supported.
#   intltool required.
#
# GSETTINGSFILES (optional/mandatory)
#   List of gsettings (.gschema.xml) files to extract gettext strings from.
#   Globbing is supported.
#   intltool required.
#
# Either SRCFILES or GLADEFILES or DESKTOPFILES or GSETTINGSFILES (or all)
# have to be filled with some files.
#
##
# The gettext_create_translations function generates .gmo files from .po files
# and install them as .mo files.
# Provide target 'translations' to build .gmo files.
#
# Supported sections:
#
# ALL (optional)
#   Make translations target a dependency of the 'all' target. (Build
#   translations with every build.)
#
# PODIR (optional)
#   Directory with .po files.
#   Default: ${CMAKE_CURRENT_SOURCE_DIR}
#
# LOCALEDIR (optional)
#   Base directory where to install translations.
#   Default: share/cmake
#
# LANGUAGES (optional)
#   List of language 'short names'. This is in generel the part before the .po.
#   With English locale this is e.g. 'en_GB' or 'en_US' etc.
#
# POFILES (optional)
#   List of .po files.
#
##
#
# The following call is a simple example (within project po directory):
#
#   include(Gettext)
#   if(XGETTEXT_FOUND)
#     set(potfile "${CMAKE_CURRENT_SOURCE_DIR}/my_project.pot")
#     gettext_create_pot("${potfile}"
#       SRCFILES
#         "${PROJECT_SOURCE_DIR}/src/*.vala"
#     )
#     gettext_create_translations("${potfile}" ALL)
#   endif()
#
##

#### Changed GLOB to GLOB_RECURSE on line 246

find_program(GETTEXT_MSGMERGE_EXECUTABLE msgmerge)
find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
find_program(GETTEXT_MSGCAT_EXECUTABLE msgcat)
find_program(XGETTEXT_EXECUTABLE xgettext)
mark_as_advanced(GETTEXT_MSGMERGE_EXECUTABLE)
mark_as_advanced(GETTEXT_MSGFMT_EXECUTABLE)
mark_as_advanced(GETTEXT_MSGCAT_EXECUTABLE)
mark_as_advanced(XGETTEXT_EXECUTABLE)

if(XGETTEXT_EXECUTABLE)
  execute_process(
    COMMAND
      "${XGETTEXT_EXECUTABLE}" "--version"
    OUTPUT_VARIABLE
      gettext_version
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(gettext_version MATCHES "^xgettext \\(.*\\) [0-9]")
    string(REGEX REPLACE "^xgettext \\([^\\)]*\\) ([0-9\\.]+[^ \n]*).*" "\\1" GETTEXT_VERSION_STRING "${gettext_version}")
  endif()
  unset(gettext_version)
endif()


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Gettext
  REQUIRED_VARS
    XGETTEXT_EXECUTABLE
    GETTEXT_MSGMERGE_EXECUTABLE
    GETTEXT_MSGFMT_EXECUTABLE
    GETTEXT_MSGCAT_EXECUTABLE
  VERSION_VAR
    GETTEXT_VERSION_STRING
)

if(XGETTEXT_EXECUTABLE AND GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE AND GETTEXT_MSGCAT_EXECUTABLE)
  set(XGETTEXT_FOUND TRUE)
  # Export variable to use it as status info.
  set(TRANSLATION_BUILD TRUE PARENT_SCOPE)
else()
  set(XGETTEXT_FOUND FALSE)
  set(TRANSLATION_BUILD FALSE PARENT_SCOPE)
endif()


set(XGETTEXT_OPTIONS_DEFAULT
  "--escape"
  "--add-comments=TRANSLATORS:"  #TODO: Make this configurable.
  "--from-code=UTF-8"
)
set(XGETTEXT_VALA_OPTIONS_DEFAULT
  "--language=C"
  "--keyword=_"
  "--keyword=N_"
  "--keyword=C_:1c,2"
  "--keyword=NC_:1c,2"
)
set(XGETTEXT_GLADE_OPTIONS_DEFAULT
  "--language=Glade"
  "--omit-header"
)
set(XGETTEXT_INTLTOOL_OPTIONS
  "--language=C"
  "--keyword=N_:1"
)
set(_INTLTOOL_DESKTOPFILES)


if(XGETTEXT_FOUND)
  macro(gettext_create_pot potfile)
    cmake_parse_arguments(ARGS "" "PACKAGE;VERSION;WORKING_DIRECTORY"
      "OPTIONS;VALA_OPTIONS;GLADE_OPTIONS;SRCFILES;GLADEFILES;DESKTOPFILES;GSETTINGSFILES" ${ARGN})

    if(ARGS_PACKAGE)
      set(package_name "${ARGS_PACKAGE}")
    elseif(GETTEXT_PACKAGE)
      set(package_name "${GETTEXT_PACKAGE}")
    else()
      set(package_name "${PROJECT_NAME}")
    endif()

    if(ARGS_VERSION)
      set(package_version "${ARGS_VERSION}")
    elseif(VERSION)
      set(package_version "${VERSION}")
    else()
      set(package_version "${${package_name}_VERSION}")
    endif()
    # Export for status information.
    set(GETTEXT_PACKAGE_NAME "${package_name}" PARENT_SCOPE)
    set(GETTEXT_PACKAGE_VERSION "${package_version}" PARENT_SCOPE)

    if(NOT ARGS_WORKING_DIRECTORY)
      set(ARGS_WORKING_DIRECTORY "../")
    endif()

    set(xgettext_options "--package-name" "${package_name}")
    if(package_version)
      list(APPEND xgettext_options "--package-version" "${package_version}")
    endif()
    if(ARGS_OPTIONS)
      list(APPEND xgettext_options ${ARGS_OPTIONS})
    else()
      list(APPEND xgettext_options ${XGETTEXT_OPTIONS_DEFAULT})
    endif()

    if(ARGS_XGETTEXT_VALA_OPTIONS_DEFAULT)
      set(xgettext_vala_options ${ARGS_XGETTEXT_VALA_OPTIONS_DEFAULT})
    else()
      set(xgettext_vala_options ${XGETTEXT_VALA_OPTIONS_DEFAULT})
    endif()
    if(ARGS_XGETTEXT_GLADE_OPTIONS_DEFAULT)
      set(xgettext_glade_options ${ARGS_XGETTEXT_GLADE_OPTIONS_DEFAULT})
    else()
      set(xgettext_glade_options ${XGETTEXT_GLADE_OPTIONS_DEFAULT})
    endif()

    if(ARGS_SRCFILES OR ARGS_GLADEFILES OR ARGS_DESKTOPFILES OR ARGS_GSETTINGSFILES)
      set(src_list)
      set(src_list_abs)
      foreach(globexpr ${ARGS_SRCFILES})
        if(NOT IS_ABSOLUTE "${globexpr}")
          get_filename_component(absDir "${ARGS_WORKING_DIRECTORY}" ABSOLUTE)
          set(globexpr "${absDir}/${globexpr}")
        endif()
        set(tmpsrcfiles)
        file(GLOB_RECURSE tmpsrcfiles ${globexpr})
        if (tmpsrcfiles)
          foreach(absFile ${tmpsrcfiles})
            file(RELATIVE_PATH relFile "${CMAKE_CURRENT_SOURCE_DIR}" "${absFile}")
            list(APPEND src_list "${relFile}")
            list(APPEND src_list_abs "${absFile}")
          endforeach()
        else()
          file(RELATIVE_PATH relFile "${CMAKE_CURRENT_SOURCE_DIR}" "${globexpr}")
          list(APPEND src_list "${relFile}")
          list(APPEND src_list_abs "${globexpr}")
        endif()
      endforeach()

      set(glade_list)
      set(glade_list_abs)
      foreach(globexpr ${ARGS_GLADEFILES})
        if(NOT IS_ABSOLUTE "${globexpr}")
          get_filename_component(absDir "${ARGS_WORKING_DIRECTORY}" ABSOLUTE)
          set(globexpr "${absDir}/${globexpr}")
        endif()
        set(tmpgladefiles)
        file(GLOB tmpgladefiles ${globexpr})
        if (tmpgladefiles)
          foreach(absFile ${tmpgladefiles})
            file(RELATIVE_PATH relFile "${CMAKE_CURRENT_SOURCE_DIR}" "${absFile}")
            list(APPEND glade_list "${relFile}")
            list(APPEND glade_list_abs "${absFile}")
          endforeach()
        else()
          file(RELATIVE_PATH relFile "${CMAKE_CURRENT_SOURCE_DIR}" "${globexpr}")
          list(APPEND glade_list "${relFile}")
          list(APPEND glade_list_abs "${globexpr}")
        endif()
      endforeach()

      if(ARGS_DESKTOPFILES OR ARGS_GSETTINGSFILES)
        find_package(Intltool REQUIRED)
      endif()
      set(desktop_list)
      set(desktop_list_abs)
      set(desktop_list_b)
      set(desktop_list_b_abs)
      set(desktop_list_h)
      set(desktop_list_h_abs)
      foreach(globexpr ${ARGS_DESKTOPFILES})
        if(NOT IS_ABSOLUTE "${globexpr}")
          get_filename_component(absDir "${ARGS_WORKING_DIRECTORY}" ABSOLUTE)
          set(globexpr "${absDir}/${globexpr}")
        endif()
        set(tmpdesktopfiles)
        file(GLOB tmpdesktopfiles ${globexpr})
        if (tmpdesktopfiles)
          foreach(absFile ${tmpdesktopfiles})
            file(RELATIVE_PATH relFile "${CMAKE_CURRENT_SOURCE_DIR}" "${absFile}")
            list(APPEND desktop_list "${relFile}")
            list(APPEND desktop_list_abs "${absFile}")

            file(RELATIVE_PATH relFile_b "${PROJECT_SOURCE_DIR}" "${absFile}")
            list(APPEND desktop_list_b "${relFile_b}")
            get_filename_component(absFile_b "${PROJECT_BINARY_DIR}/${relFile_b}" ABSOLUTE)
            list(APPEND desktop_list_b_abs "${absFile_b}")
            file(RELATIVE_PATH relFile_h "${CMAKE_CURRENT_SOURCE_DIR}" "${absFile_b}.h")
            list(APPEND desktop_list_h "${relFile_h}")
            list(APPEND desktop_list_h_abs "${absFile_b}.h")
            add_custom_command(
              OUTPUT
                "${absFile_b}"
              COMMAND
                "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${relFile}" "${absFile_b}"
              DEPENDS
                "${absFile}"
              VERBATIM
            )
          endforeach()
        else()
          file(RELATIVE_PATH relFile "${CMAKE_CURRENT_SOURCE_DIR}" "${globexpr}")
          list(APPEND desktop_list "${relFile}")
          list(APPEND desktop_list_abs "${globexpr}")

          file(RELATIVE_PATH relFile_b "${PROJECT_SOURCE_DIR}" "${globexpr}")
          list(APPEND desktop_list_b "${relFile_b}")
          get_filename_component(absFile_b "${PROJECT_BINARY_DIR}/${relFile_b}" ABSOLUTE)
          list(APPEND desktop_list_b_abs "${absFile_b}")
          file(RELATIVE_PATH relFile_h "${CMAKE_CURRENT_SOURCE_DIR}" "${absFile_b}.h")
          list(APPEND desktop_list_h "${relFile_h}")
          list(APPEND desktop_list_h_abs "${absFile_b}.h")
          add_custom_command(
            OUTPUT
              "${absFile_b}"
            COMMAND
              "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${relFile}" "${absFile_b}"
            DEPENDS
              "${absFile}"
            VERBATIM
          )
        endif()
      endforeach()
      set(_INTLTOOL_DESKTOPFILES ${desktop_list_b})

      set(gsettings_list)
      set(gsettings_list_abs)
      set(gsettings_list_b)
      set(gsettings_list_b_abs)
      set(gsettings_list_h)
      set(gsettings_list_h_abs)
      foreach(globexpr ${ARGS_GSETTINGSFILES})
        if(NOT IS_ABSOLUTE "${globexpr}")
          get_filename_component(absDir "${ARGS_WORKING_DIRECTORY}" ABSOLUTE)
          set(globexpr "${absDir}/${globexpr}")
        endif()
        set(tmpgsettingsfiles)
        file(GLOB tmpgsettingsfiles ${globexpr})
        if (tmpgsettingsfiles)
          foreach(absFile ${tmpgsettingsfiles})
            file(RELATIVE_PATH relFile "${CMAKE_CURRENT_SOURCE_DIR}" "${absFile}")
            list(APPEND gsettings_list "${relFile}")
            list(APPEND gsettings_list_abs "${absFile}")

            file(RELATIVE_PATH relFile_b "${PROJECT_SOURCE_DIR}" "${absFile}")
            list(APPEND gsettings_list_b "${relFile_b}")
            get_filename_component(absFile_b "${PROJECT_BINARY_DIR}/${relFile_b}" ABSOLUTE)
            list(APPEND gsettings_list_b_abs "${absFile_b}")
            file(RELATIVE_PATH relFile_h "${CMAKE_CURRENT_SOURCE_DIR}" "${absFile_b}.h")
            list(APPEND gsettings_list_h "${relFile_h}")
            list(APPEND gsettings_list_h_abs "${absFile_b}.h")
            add_custom_command(
              OUTPUT
                "${absFile_b}"
              COMMAND
                "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${relFile}" "${absFile_b}"
              DEPENDS
                "${absFile}"
              VERBATIM
            )
          endforeach()
        else()
          file(RELATIVE_PATH relFile "${CMAKE_CURRENT_SOURCE_DIR}" "${globexpr}")
          list(APPEND gsettings_list "${relFile}")
          list(APPEND gsettings_list_abs "${globexpr}")

          file(RELATIVE_PATH relFile_b "${PROJECT_SOURCE_DIR}" "${globexpr}")
          list(APPEND gsettings_list_b "${relFile_b}")
          get_filename_component(absFile_b "${PROJECT_BINARY_DIR}/${relFile_b}" ABSOLUTE)
          list(APPEND gsettings_list_b_abs "${absFile_b}")
          file(RELATIVE_PATH relFile_h "${CMAKE_CURRENT_SOURCE_DIR}" "${absFile_b}.h")
          list(APPEND gsettings_list_h "${relFile_h}")
          list(APPEND gsettings_list_h_abs "${absFile_b}.h")
          add_custom_command(
            OUTPUT
              "${absFile_b}"
            COMMAND
              "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${relFile}" "${absFile_b}"
            VERBATIM
          )
        endif()
      endforeach()


      if(ARGS_SRCFILES)
        add_custom_command(
          OUTPUT
            "${CMAKE_CURRENT_BINARY_DIR}/_source.pot"
          COMMAND
            "${XGETTEXT_EXECUTABLE}" ${xgettext_options} ${xgettext_vala_options} "-o" "${CMAKE_CURRENT_BINARY_DIR}/_source.pot" ${src_list}
          COMMAND
            # Make sure file exists even if no translatable strings available.
            "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/_source.pot"
          DEPENDS
            ${src_list_abs}
          WORKING_DIRECTORY
            "${CMAKE_CURRENT_SOURCE_DIR}"
          VERBATIM
      )
      else()
        add_custom_command(
          OUTPUT
            "${CMAKE_CURRENT_BINARY_DIR}/_source.pot"
          COMMAND
            "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/_source.pot"
          VERBATIM
        )
      endif()
      if(ARGS_GLADEFILES)
        add_custom_command(
          OUTPUT
            "${CMAKE_CURRENT_BINARY_DIR}/_glade.pot"
          COMMAND
            "${XGETTEXT_EXECUTABLE}" ${xgettext_options} ${xgettext_glade_options} "-o" "${CMAKE_CURRENT_BINARY_DIR}/_glade.pot" ${glade_list}
          COMMAND
            "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/_glade.pot"
          DEPENDS
            ${glade_list_abs}
          WORKING_DIRECTORY
            "${CMAKE_CURRENT_SOURCE_DIR}"
          VERBATIM
        )
      else()
        add_custom_command(
          OUTPUT
            "${CMAKE_CURRENT_BINARY_DIR}/_glade.pot"
          COMMAND
            "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/_glade.pot"
          VERBATIM
        )
      endif()
      if(ARGS_DESKTOPFILES)
        add_custom_command(
          OUTPUT
            ${desktop_list_h_abs}
          COMMAND
            "${INTLTOOL_EXTRACT_EXECUTABLE}" ${INTLTOOL_OPTIONS_DEFAULT} "--type" "gettext/ini" ${desktop_list_b}
          DEPENDS
            ${desktop_list_b_abs}
          WORKING_DIRECTORY
            "${PROJECT_BINARY_DIR}"
          VERBATIM
        )
        add_custom_command(
          OUTPUT
            "${CMAKE_CURRENT_BINARY_DIR}/_desktop.pot"
          COMMAND
            "${XGETTEXT_EXECUTABLE}" ${xgettext_options} ${XGETTEXT_INTLTOOL_OPTIONS} "-o" "${CMAKE_CURRENT_BINARY_DIR}/_desktop.pot" ${desktop_list_h}
          COMMAND
            "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/_desktop.pot"
          DEPENDS
            ${desktop_list_h_abs}
          WORKING_DIRECTORY
            "${CMAKE_CURRENT_SOURCE_DIR}"
          VERBATIM
        )
      else()
        add_custom_command(
          OUTPUT
            "${CMAKE_CURRENT_BINARY_DIR}/_desktop.pot"
          COMMAND
            "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/_desktop.pot"
          VERBATIM
        )
      endif()
      if(ARGS_GSETTINGSFILES)
        add_custom_command(
          OUTPUT
            ${gsettings_list_h_abs}
          COMMAND
            "${INTLTOOL_EXTRACT_EXECUTABLE}" ${INTLTOOL_OPTIONS_DEFAULT} "--type" "gettext/gsettings" ${gsettings_list_b}
          DEPENDS
            ${gsettings_list_b_abs}
          WORKING_DIRECTORY
            "${PROJECT_BINARY_DIR}"
          VERBATIM
        )
        add_custom_command(
          OUTPUT
            "${CMAKE_CURRENT_BINARY_DIR}/_gsettings.pot"
          COMMAND
            "${XGETTEXT_EXECUTABLE}" ${xgettext_options} ${XGETTEXT_INTLTOOL_OPTIONS} "-o" "${CMAKE_CURRENT_BINARY_DIR}/_gsettings.pot" ${gsettings_list_h}
          COMMAND
            "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/_gsettings.pot"
          DEPENDS
            ${gsettings_list_h_abs}
          WORKING_DIRECTORY
            "${CMAKE_CURRENT_SOURCE_DIR}"
          VERBATIM
        )
      else()
        add_custom_command(
          OUTPUT
            "${CMAKE_CURRENT_BINARY_DIR}/_gsettings.pot"
          COMMAND
            "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/_gsettings.pot"
          VERBATIM
        )
      endif()

      add_custom_target(pot
        COMMAND
          "${GETTEXT_MSGCAT_EXECUTABLE}" "-o" "${potfile}" "--use-first" "${CMAKE_CURRENT_BINARY_DIR}/_source.pot" "${CMAKE_CURRENT_BINARY_DIR}/_glade.pot" "${CMAKE_CURRENT_BINARY_DIR}/_desktop.pot" "${CMAKE_CURRENT_BINARY_DIR}/_gsettings.pot"
        DEPENDS
          "${CMAKE_CURRENT_BINARY_DIR}/_source.pot"
          "${CMAKE_CURRENT_BINARY_DIR}/_glade.pot"
          "${CMAKE_CURRENT_BINARY_DIR}/_desktop.pot"
          "${CMAKE_CURRENT_BINARY_DIR}/_gsettings.pot"
        WORKING_DIRECTORY
          "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT
          "Extract translatable messages to ${potfile}"
        VERBATIM
      )
    endif()
  endmacro()


  function(gettext_create_translations potfile)
    cmake_parse_arguments(ARGS "ALL;NOUPDATE;DESKTOPFILES_INSTALL"
        "PODIR;LOCALEDIR" "LANGUAGES;POFILES" ${ARGN})

    get_filename_component(_potBasename "${potfile}" NAME_WE)
    get_filename_component(_absPotFile "${potfile}" ABSOLUTE)

    if(ARGS_ALL)
      set(make_all "ALL")
    else()
      set(make_all)
    endif()

    if(ARGS_LOCALEDIR)
      set(_localedir "${ARGS_LOCALEDIR}")
    elseif(localedir)
      set(_localedir "${localedir}")
    else()
      set(_localedir "share/locale")
    endif()

    set(langs)
    list(APPEND langs ${ARGS_LANGUAGES})

    foreach(globexpr ${ARGS_POFILES})
      set(tmppofiles)
      file(GLOB tmppofiles ${globexpr})
      if(tmppofiles)
        foreach(tmppofile ${tmppofiles})
          string(REGEX REPLACE ".*/([a-zA-Z_]+)(\\.po)?$" "\\1" lang "${tmppofile}")
          list(APPEND langs "${lang}")
        endforeach()
      else()
          string(REGEX REPLACE ".*/([a-zA-Z_]+)(\\.po)?$" "\\1" lang "${globexpr}")
          list(APPEND langs "${lang}")
      endif()
    endforeach()

    if(NOT langs AND NOT ARGS_PODIR)
      set(ARGS_PODIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    if(ARGS_PODIR)
      file(GLOB pofiles "${ARGS_PODIR}/*.po")
      foreach(pofile ${pofiles})
        string(REGEX REPLACE ".*/([a-zA-Z_]+)\\.po$" "\\1" lang "${pofile}")
        list(APPEND langs "${lang}")
      endforeach()
    endif()

    if(langs)
      list(REMOVE_DUPLICATES langs)
    endif()


    set(_gmoFile)
    set(_gmoFiles)
    foreach (lang ${langs})
      get_filename_component(_absFile "${lang}.po" ABSOLUTE)
      set(_gmoFile "${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo")

      if(ARGS_NOUPDATE)
        set(_absFile_new "${CMAKE_CURRENT_BINARY_DIR}/${lang}.po")
        add_custom_command(
          OUTPUT
            "${_absFile_new}"
          COMMAND
            "${CMAKE_COMMAND}" -E copy "${_absFile}" "${_absFile_new}"
          DEPENDS
            "${_absPotFile}"
            "${_absFile}"
          VERBATIM
        )
        set(_absFile "${_absFile_new}")
      endif()
      add_custom_command(
        OUTPUT
          "${_gmoFile}"
        COMMAND
          "${GETTEXT_MSGMERGE_EXECUTABLE}" "--quiet" "--update" "--backup=none" "${_absFile}" "${_absPotFile}"
        COMMAND
          "${GETTEXT_MSGFMT_EXECUTABLE}" "-o" "${_gmoFile}" "${_absFile}"
        DEPENDS
          "${_absPotFile}"
          "${_absFile}"
        WORKING_DIRECTORY
          "${CMAKE_CURRENT_BINARY_DIR}"
        VERBATIM
      )

      install(
        FILES
          "${_gmoFile}"
        DESTINATION
          "${_localedir}/${lang}/LC_MESSAGES"
        RENAME
          "${_potBasename}.mo"
      )
      list(APPEND _gmoFiles "${_gmoFile}")
    endforeach()

    set(desktopfiles)
    if(langs AND _INTLTOOL_DESKTOPFILES)
      file(RELATIVE_PATH cursrcdir_rel "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
      if(cursrcdir_rel STREQUAL "")
        set(cursrcdir_rel ".")
      endif()
      foreach(desktopfiletmp ${_INTLTOOL_DESKTOPFILES})
        string(REGEX REPLACE "(\\.desktop).*$" "\\1" desktopfile "${desktopfiletmp}")
        set(desktopfile_abs "${PROJECT_BINARY_DIR}/${desktopfile}")
        list(APPEND desktopfiles "${desktopfile_abs}")
        file(RELATIVE_PATH desktopfile_rel "${CMAKE_CURRENT_BINARY_DIR}" "${desktopfile_abs}")
        file(RELATIVE_PATH desktopfiletmp_rel "${CMAKE_CURRENT_BINARY_DIR}" "${PROJECT_BINARY_DIR}/${desktopfiletmp}")
        add_custom_command(
          OUTPUT
            "${desktopfile_abs}"
          COMMAND
            "${INTLTOOL_MERGE_EXECUTABLE}" ${INTLTOOL_OPTIONS_DEFAULT} "--desktop-style" "${cursrcdir_rel}" "${desktopfiletmp_rel}" "${desktopfile_rel}"
          DEPENDS
            "${PROJECT_BINARY_DIR}/${desktopfiletmp}"
          VERBATIM
        )
        if(ARGS_DESKTOPFILES_INSTALL AND NOT cursrcdir_rel STREQUAL ".")
          add_custom_command(
            OUTPUT
              "${CMAKE_SOURCE_DIR}/${desktopfile}"
              "desktopinstall.stamp"
            COMMAND
              "${CMAKE_COMMAND}" -E copy_if_different "${desktopfile_abs}" "${CMAKE_SOURCE_DIR}/${desktopfile}"
            COMMAND
              "${CMAKE_COMMAND}" -E touch "desktopinstall.stamp"
            DEPENDS
              "${desktopfile_abs}"
            VERBATIM
          )
        else()
          add_custom_command(
            OUTPUT
              "desktopinstall.stamp"
            COMMAND
              "${CMAKE_COMMAND}" -E touch "desktopinstall.stamp"
            DEPENDS
              "${desktopfile_abs}"
            VERBATIM
          )
        endif()
        list(APPEND desktopfiles "desktopinstall.stamp")
        install(
          FILES
            "${desktopfile_abs}"
          DESTINATION
            "share/applications"
        )
      endforeach()
    endif()

    add_custom_target(translations
      ${make_all}
      DEPENDS
        ${_gmoFiles}
        ${desktopfiles}
      COMMENT
        "Build translations."
    )
  endfunction()
endif()

# vim: set ai ts=2 sts=2 et sw=2
