cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

set(ENABLE_NLS ON PARENT_SCOPE)

include(Gettext)

if (XGETTEXT_FOUND)

  set(potfile "${PROJECT_NAME}.pot")

  gettext_create_pot ("${potfile}"
          PACKAGE ${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    OPTIONS --language=C++ --from-code=UTF-8
            --keyword=_ --keyword=_F --keyword=N_
            --keyword=C_:1c,2 --keyword=C_F:1c,2 --keyword=NC_:1c,2

    SRCFILES
      "${PROJECT_SOURCE_DIR}/src/*.cpp"

    GLADEFILES
      "${PROJECT_SOURCE_DIR}/ui/*.glade"
  )

  gettext_create_translations ("${potfile}" ALL NOUPDATE)

else (XGETTEXT_FOUND)
  message (ERROR "Gettext not found!")
endif (XGETTEXT_FOUND)

set(GETTEXT_PACKAGE ${PROJECT_NAME} PARENT_SCOPE)
set(PACKAGE_LOCALE_DIR ${PACKAGE_DATA_DIR}/locale PARENT_SCOPE)

