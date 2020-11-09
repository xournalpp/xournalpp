#!/bin/bash -xve

# Cleanup the macports build environment


# Copyright (C) 2020 Friedrich Beckmann
# Released under GNU General Public License, either version 3
# or any later option

topdir=$HOME/gtk

# This is the installation directory which will be used as macports prefix
# and as pspp configure prefix.
bundleinstall=$topdir/inst

export PATH=$bundleinstall/bin:$bundleinstall/sbin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin

for i in  {1..10}; do port -N uninstall leaves || true ; done

rm -rf $bundleinstall/var/macports
rm -rf $topdir/macports-ports
