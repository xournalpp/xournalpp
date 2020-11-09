#!/bin/bash -xve

# Install macports and all dependencies for xournalpp in $HOME/gtk/inst
# macports is all recompiled because it uses the quartz backend instead of x11

portindexversion=8e06b840b02731c3ef712f4aa6c4ff138a64a604

# Copyright (C) 2020 Friedrich Beckmann
# Released under GNU General Public License, either version 3
# or any later option

echo `date`

# Check if we are on MacOS
if ! test `uname` = "Darwin"; then
    echo "This only works on MacOS"
    exit
fi

# Check if XCode is installed - Assume clang indicates xcode.
if ! test -f /usr/bin/clang; then
    echo "/usr/bin/clang not found - please install XCode CLT"
    exit
fi

topdir=$HOME/gtk

# This is the installation directory which will be used as macports prefix
# and as pspp configure prefix.
bundleinstall=$topdir/inst

export PATH=$bundleinstall/bin:$bundleinstall/sbin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin

# Target macports install directory for the pspp bundle
if test -d $bundleinstall; then
    echo "Found existing macports directory $bundleinstall - continue"
else
    echo "Creating Macports installation in $bundleinstall"
    mkdir -p $bundleinstall
    # Install macports
    rm -rf /tmp/macports
    mkdir /tmp/macports
    pushd /tmp/macports
    macportsversion=2.6.3
    curl https://distfiles.macports.org/MacPorts/MacPorts-$macportsversion.tar.gz -O
    tar xvzf Macports-$macportsversion.tar.gz
    cd Macports-$macportsversion
    ./configure --prefix=$bundleinstall \
                --with-applications-dir=$bundleinstall/Applications \
                --with-no-root-privileges
    make
    make install
    popd
    rm -rf /tmp/macports
    # Modify the default variants to use quartz
    echo "-x11 +no_x11 +quartz" > $bundleinstall/etc/macports/variants.conf
    # Make the build compatible with previous OSX Versions
    # echo "macosx_deployment_target 10.7" >> $bundleinstall/etc/macports/macports.conf
    # dbus tries to install startup items which are under superuser account
    echo "startupitem_install no"  >> $bundleinstall/etc/macports/macports.conf
    echo "buildfromsource always" >> $bundleinstall/etc/macports/macports.conf
    # Activate step failed due to bsdtar problem 
    echo "hfscompression no"  >> $bundleinstall/etc/macports/macports.conf
    # Fix the macports version for reproducible builds
    pushd $topdir
    git clone --single-branch https://github.com/macports/macports-ports.git
    cd macports-ports
    git checkout $portindexversion
    portindex
    popd
    echo "file://$topdir/macports-ports" > $bundleinstall/etc/macports/sources.conf
    echo "rsync://rsync.macports.org/macports/release/tarballs/ports.tar [default]" >> $bundleinstall/etc/macports/sources.conf
fi

echo `date`
# Install the packages for pspp
port -v selfupdate
port upgrade outdated || true
# Install the build dependencies


buildports="pkgconfig  \
  gtk3 \
  gtk-doc \
  adwaita-icon-theme \
  cmake \
  automake \
  cppunit \
  autoconf \
  poppler \
  libzip \
  portaudio \
  libsndfile"

port -N install $buildports
port -N setrequested $buildports

# gtk-mac-integration needs gettext 0.20 but macports has still 0.19
# so you need to uninstall gettext via macports and install it via the script
# build-gettext.sh

port -Nf uninstall gettext
