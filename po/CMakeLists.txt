cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

set(ENABLE_NLS ON PARENT_SCOPE)

include(Gettext)

if (XGETTEXT_FOUND)

  set(potfile "${PROJECT_NAME}.pot")

  gettext_create_pot ("${potfile}"
          PACKAGE ${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    CPP_OPTIONS --language=C++ --from-code=UTF-8
            --keyword=_ --keyword=_F --keyword=N_
            --keyword=C_:1c,2 --keyword=C_F:1c,2 --keyword=NC_:1c,2

    OPTIONS --from-code=UTF-8

    SRCFILES
      "${PROJECT_SOURCE_DIR}/src/*.cpp"

    GLADEFILES
      "${PROJECT_SOURCE_DIR}/ui/*.ui"

    DESKTOPFILES
      "${PROJECT_SOURCE_DIR}/desktop/com.github.xournalpp.xournalpp.desktop.in"

    INIFILES
      "${PROJECT_SOURCE_DIR}/resources-templates/pagetemplates.ini.in;${PROJECT_SOURCE_DIR}/resources-templates/toolbar.ini.in"

    XMLFILES
      "${PROJECT_SOURCE_DIR}/resources-templates/com.github.xournalpp.xournalpp.appdata.xml.in;${PROJECT_SOURCE_DIR}/resources-templates/com.github.xournalpp.xournalpp.xml.in"

    # The variable ITSDIR is used to find the .loc and .its files in po/its/
    # This is only necessary for Ubuntu 20.04. On more up-to-date distributions, the package shared-mime-info contains those .loc and .its files.
    ITSDIR
      "${PROJECT_SOURCE_DIR}/po/"
  )

  gettext_create_translations ("${potfile}" ALL NOUPDATE)

else (XGETTEXT_FOUND)
  message (ERROR "Gettext not found!")
endif (XGETTEXT_FOUND)

set(GETTEXT_PACKAGE ${PROJECT_NAME} PARENT_SCOPE)
set(PACKAGE_LOCALE_DIR ${PACKAGE_DATA_DIR}/locale PARENT_SCOPE)

