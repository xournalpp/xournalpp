add_subdirectory (workaround)

add_custom_target (poppler
  COMMAND "${PROJECT_SOURCE_DIR}/src/pdf/popplerdirect/workaround/workaround.sh" build
)

add_custom_target (poppler-clean
  COMMAND "${PROJECT_SOURCE_DIR}/src/pdf/popplerdirect/workaround/workaround.sh" clean
)
