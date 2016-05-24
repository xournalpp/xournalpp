set (WORKAROUND_DIR "${CMAKE_SOURCE_DIR}/src/pdf/popplerdirect/workaround")

if (BUILD_POPPLER)

  set (POPPLER_SOURCES_PREFIX "${POPPLER_PREFIX}/src/poppler")

else ()

  set (POPPLER_SOURCES_PREFIX "poppler-${POPPLER_VERSION}")

  if (NOT EXISTS "${WORKAROUND_DIR}/poppler-${POPPLER_VERSION}")
    message ("Downloading and unpacking poppler sources (version ${POPPLER_VERSION})...")
    file (DOWNLOAD "http://poppler.freedesktop.org/poppler-${POPPLER_VERSION}.tar.xz"
                   "${WORKAROUND_DIR}/poppler.tar.xz")
  
    find_program (PATH_TAR tar)
    if (NOT PATH_TAR)
      message (FATAL_ERROR "tar not found (needed to unpack poppler sources)")
    endif (NOT PATH_TAR)
  
    execute_process (COMMAND ${PATH_TAR} -xf "poppler.tar.xz" WORKING_DIRECTORY "${WORKAROUND_DIR}")
    file (REMOVE "${WORKAROUND_DIR}/poppler.tar.xz")
  
    message ("Done")
  else (NOT EXISTS "${WORKAROUND_DIR}/poppler-${POPPLER_VERSION}")
    message ("Poppler sources exist")
  endif (NOT EXISTS "${WORKAROUND_DIR}/poppler-${POPPLER_VERSION}")

endif ()
  
configure_file (
  "${WORKAROUND_DIR}/workaround.h.in"
  "${WORKAROUND_DIR}/workaround.h"
)
