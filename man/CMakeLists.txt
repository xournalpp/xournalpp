cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

if (WIN32 OR APPLE)
  # Windows does not support man pages and has no thumbnailer like this
  # MacOS does not have help2man so no man pages can be generated
  return ()
endif ()


option(WITH_MAN "Install manpages" ON)
option(MAN_COMPRESS "Compress man pages using gzip" ON)

if(WITH_MAN)
  find_program(HELP2MAN help2man DOC "Location of the help2man program")
  mark_as_advanced(HELP2MAN)

  if (NOT HELP2MAN)
    message(FATAL_ERROR "help2man is missing; man pages cannot be generated")
  endif()


  set(MAN_INCLUDE "${PROJECT_SOURCE_DIR}/man/extra.groff")
  set(MAN_PAGE "${CMAKE_CURRENT_BINARY_DIR}/xournalpp.1")
  set(MAN_PAGE_THUMB "${CMAKE_CURRENT_BINARY_DIR}/xournalpp-thumbnailer.1")

  add_custom_command(
    OUTPUT ${MAN_PAGE}
    COMMAND ${HELP2MAN} --no-info -i ${MAN_INCLUDE} -o ${MAN_PAGE} --version-string=${PROJECT_VERSION} --name="open-source hand note-taking program" ${CMAKE_BINARY_DIR}/xournalpp
    DEPENDS ${CMAKE_BINARY_DIR}/xournalpp
  )

  add_custom_command(
    OUTPUT ${MAN_PAGE_THUMB}
    COMMAND ${HELP2MAN} --no-info  --no-discard-stderr -o ${MAN_PAGE_THUMB} --version-string=${PROJECT_VERSION} --name="creates thumbnails" ${CMAKE_BINARY_DIR}/src/xoj-preview-extractor/xournalpp-thumbnailer
    DEPENDS ${CMAKE_BINARY_DIR}/src/xoj-preview-extractor/xournalpp-thumbnailer
  )

  if (NOT MAN_COMPRESS)
    add_custom_target(manpage ALL DEPENDS ${MAN_PAGE} ${MAN_PAGE_THUMB})
    install(FILES ${MAN_PAGE} ${MAN_PAGE_THUMB} DESTINATION "share/man/man1")
  else()
    set(MAN_FILE "${MAN_PAGE}.gz")
    set(MAN_FILE_THUMB "${MAN_PAGE_THUMB}.gz")

    add_custom_command(
      OUTPUT ${MAN_FILE} ${MAN_FILE_THUMB}
      COMMAND gzip -n -f -9 ${MAN_PAGE} ${MAN_PAGE_THUMB}
      DEPENDS ${MAN_PAGE} ${MAN_PAGE_THUMB}
    )

    add_custom_target(manpage ALL DEPENDS ${MAN_FILE} ${MAN_FILE_THUMB})
    install(FILES ${MAN_FILE} ${MAN_FILE_THUMB} DESTINATION "share/man/man1")
  endif()
endif()
