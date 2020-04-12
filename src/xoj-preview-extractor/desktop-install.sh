#!/bin/bash

export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-$1-rule xournalpp-thumbnailer-xoj.schemas
