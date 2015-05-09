if (ENABLE_LIBOPENJPEG)
  set (ENABLE_LIBOPENJPEG_AC "yes")
else (ENABLE_LIBOPENJPEG)
  set (ENABLE_LIBOPENJPEG_AC "no")
endif (ENABLE_LIBOPENJPEG)

configure_file (
  "${PROJECT_SOURCE_DIR}/src/pdf/popplerdirect/workaround/workaround.sh.in"
  "${PROJECT_SOURCE_DIR}/src/pdf/popplerdirect/workaround/workaround.sh"
  @ONLY
)
