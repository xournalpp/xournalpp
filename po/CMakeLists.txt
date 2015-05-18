set (ENABLE_NLS ON PARENT_SCOPE)

include (Gettext)

if (XGETTEXT_FOUND)

  set (potfile "${PROJECT_PACKAGE}.pot")

  gettext_create_pot ("${potfile}"
    PACKAGE ${PROJECT_PACKAGE}
    VERSION ${PROJECT_VERSION}
    OPTIONS --language=C++ --keyword=_ --keyword=N_ --keyword=C_:1c,2 --keyword=NC_:1c,2 -s

    SRCFILES
      "${PROJECT_SOURCE_DIR}/src/*.cpp"

    GLADEFILES
      "${PROJECT_SOURCE_DIR}/ui/*.glade"
  )

  gettext_create_translations ("${potfile}" ALL)

else (XGETTEXT_FOUND)
  message (ERROR "Gettext not found!")
endif (XGETTEXT_FOUND)

set (GETTEXT_PACKAGE ${PROJECT_PACKAGE} PARENT_SCOPE)
set (PACKAGE_LOCALE_DIR ${PACKAGE_DATA_DIR}/locale PARENT_SCOPE)
