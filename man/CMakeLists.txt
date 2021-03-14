if (WIN32)
  # Windows does not support man pages and has no thumbnailer like this
  return ()
endif ()

find_program(HELP2MAN help2man DOC "Location of the help2man program")
mark_as_advanced(HELP2MAN)

if (HELP2MAN)
  set(MAN_INCLUDE "${PROJECT_SOURCE_DIR}/man/extra.groff")
  set(MAN_PAGE "${CMAKE_CURRENT_BINARY_DIR}/xournalpp.1")
  set(MAN_FILE "${CMAKE_CURRENT_BINARY_DIR}/xournalpp.1.gz")

  add_custom_command(
      OUTPUT ${MAN_FILE}
      COMMAND ${HELP2MAN} --no-info -i ${MAN_INCLUDE} -o ${MAN_PAGE} --version-string=${PROJECT_VERSION} --name="open-source hand note-taking program" ${CMAKE_BINARY_DIR}/src/xournalpp
      COMMAND gzip -n -f -9 ${MAN_PAGE}
      DEPENDS ${CMAKE_BINARY_DIR}/src/xournalpp
  )

  set(MAN_PAGE_THUMBNAILER "${CMAKE_CURRENT_BINARY_DIR}/xournalpp-thumbnailer.1")
  set(MAN_FILE_THUMBNAILER "${CMAKE_CURRENT_BINARY_DIR}/xournalpp-thumbnailer.1.gz")

  add_custom_command(
      OUTPUT ${MAN_FILE_THUMBNAILER}
      COMMAND ${HELP2MAN} --no-info  --no-discard-stderr -o ${MAN_PAGE_THUMBNAILER} --version-string=${PROJECT_VERSION} --name="creates thumbnails" ${CMAKE_BINARY_DIR}/src/xoj-preview-extractor/xournalpp-thumbnailer
      COMMAND gzip -n -f -9 ${MAN_PAGE_THUMBNAILER}
      DEPENDS ${CMAKE_BINARY_DIR}/src/xoj-preview-extractor/xournalpp-thumbnailer
  )

  add_custom_target(manpage ALL DEPENDS ${MAN_FILE} ${MAN_FILE_THUMBNAILER})
  install(FILES ${MAN_FILE} ${MAN_FILE_THUMBNAILER} DESTINATION "share/man/man1")
else(HELP2MAN)
  message("help2man is missing; no man pages will be generated")
endif(HELP2MAN)
