find_program (PATH_LATEX "latex")
if (NOT PATH_LATEX)
  message (FATAL_ERROR "\"latex\" command not found! You must install latex before installing mathtex.")
endif (NOT PATH_LATEX)

find_program (PATH_DVIPNG "dvipng")
if (NOT PATH_DVIPNG)
  message (FATAL_ERROR "\"dvipng\" command not found! You must install dvipng before installing mathtex.")
endif (NOT PATH_DVIPNG)

include (CheckIncludeFiles)
check_include_files (fcntl.h HAVE_FCNTL_H)
check_include_files (stdlib.h HAVE_STDLIB_H)
check_include_files (string.h HAVE_STRING_H)
check_include_files (unistd.h HAVE_UNISTD_H)

configure_file (
  config.h.in
  config.h
  ESCAPE_QUOTES @ONLY
)

include_directories ("${CMAKE_BINARY_DIR}/src/mathtex")

add_executable (mathtex-xournalpp.cgi
  mathtex.c
)

install (TARGETS mathtex-xournalpp.cgi
  RUNTIME DESTINATION bin
  COMPONENT xournalpp-mathtex
)
