# Adopted from https://github.com/xbmc/xbmc

# Distro name and codename
if(NOT DISTRO_CODENAME)
  find_program(LSB_RELEASE_CMD lsb_release)
  if(NOT LSB_RELEASE_CMD)
    if (WIN32)
	  set (DISTRO_CODENAME "Windows")
	else ()
      message(WARNING "include/Version.cmake: Can't find lsb_release in your path. Setting DISTRO_CODENAME to unknown.")
      set(DISTRO_CODENAME unknown)
	endif ()
  else()
    execute_process(COMMAND ${LSB_RELEASE_CMD} -cs
                    OUTPUT_VARIABLE DISTRO_CODENAME
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${LSB_RELEASE_CMD} -is
                    OUTPUT_VARIABLE DISTRO_NAME
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()
endif()

# Parses git info and sets variables used to identify the build
# Arguments:
#   stamp variable name to return
# Optional Arguments:
#   FULL: generate git HEAD commit in the form of 'YYYYMMDD-hash'
#         if git tree is dirty, value is set in the form of 'YYYYMMDD-hash-dirty'
#         if no git tree is found, value is set in the form of 'YYYYMMDD-nogitfound'
#         if FULL is not given, stamp is generated following the same process as above
#         but without 'YYYYMMDD'
# On return:
#   Variable is set with generated stamp to PARENT_SCOPE
function(core_find_git_rev stamp)
  # allow manual setting GIT_VERSION
  if(DEFINED GIT_VERSION)
    set(${stamp} ${GIT_VERSION} PARENT_SCOPE)
  else()
    find_package(Git)
    if(GIT_FOUND AND EXISTS ${CMAKE_SOURCE_DIR}/.git)
      # get tree status i.e. clean working tree vs dirty (uncommitted or unstashed changes, etc.)
      execute_process(COMMAND ${GIT_EXECUTABLE} update-index --ignore-submodules -q --refresh
                      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
      execute_process(COMMAND ${GIT_EXECUTABLE} diff-files --ignore-submodules --quiet --
                      RESULT_VARIABLE status_code
                      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        if(NOT status_code)
          execute_process(COMMAND ${GIT_EXECUTABLE} diff-index --ignore-submodules --quiet HEAD --
                        RESULT_VARIABLE status_code
                        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        endif()
        # get HEAD commit SHA-1
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short=8 HEAD
                        OUTPUT_VARIABLE HASH
                        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        string(REPLACE "\"" "" HASH ${HASH})
		string(STRIP ${HASH} HASH)

        if(status_code)
          string(CONCAT HASH ${HASH} "-dirty")
        endif()
		message(${HASH})

      # get HEAD commit date
      execute_process(COMMAND ${GIT_EXECUTABLE} log --no-show-signature -1 --pretty=format:"%cd" --date=short HEAD
                      OUTPUT_VARIABLE DATE
                      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
      string(REPLACE "\"" "" DATE ${DATE})
      string(REPLACE "-" "" DATE ${DATE})
    else()
      if(EXISTS ${CMAKE_SOURCE_DIR}/BUILDDATE)
        file(STRINGS ${CMAKE_SOURCE_DIR}/BUILDDATE DATE LIMIT_INPUT 8)
      else()
        string(TIMESTAMP DATE "%Y%m%d" UTC)
      endif()
      if(EXISTS ${CMAKE_SOURCE_DIR}/VERSION)
        file(STRINGS ${CMAKE_SOURCE_DIR}/VERSION HASH LIMIT_INPUT 16)
      else()
        set(HASH "")
      endif()
    endif()
    cmake_parse_arguments(arg "FULL" "" "" ${ARGN})
    if(arg_FULL)
      set(${stamp} ${DATE}-${HASH} PARENT_SCOPE)
    else()
      set(${stamp} ${HASH} PARENT_SCOPE)
    endif()
  endif()
endfunction()
