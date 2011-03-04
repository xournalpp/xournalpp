#!/bin/sh

xgettext -d xournalpp --join-existing --sort-output -k_ -s --keyword=translatable -o de.pot ../src/*.cpp ../src/view/*.cpp ../src/util/*.cpp ../src/undo/*.cpp ../src/pdf/*.cpp ../src/pdf/poppler/*.cpp  ../src/pdf/cairo/*.cpp ../src/model/*.cpp ../src/gui/*.cpp ../src/gui/dialog/*.cpp ../src/gui/*.cpp ../src/gui/sidebar/*.cpp ../src/gui/toolbarMenubar/*.cpp ../src/gui/widgets/*.cpp ../src/control/*.cpp ../src/control/settings/*.cpp ../src/control/shaperecognizer/*.cpp ../src/control/tools/*.cpp ../src/control/xml/*.cpp ../ui/*.glade

#msgfmt -c -v -o compiled/hello.mo translations/de/*.pot
# sudo  cp hello.mo /usr/share/locale/de_CH/LC_MESSAGES

