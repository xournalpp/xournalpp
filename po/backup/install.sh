#!/bin/sh

msgfmt -c -v -o de.mo de.pot

mkdir /usr/local/share/locale/de_CH/
mkdir /usr/local/share/locale/de_CH/LC_MESSAGES/
cp de.mo /usr/local/share/locale/de_CH/LC_MESSAGES/xournalpp.mo

