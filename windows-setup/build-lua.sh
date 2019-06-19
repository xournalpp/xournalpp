#!/bin/bash

## Edit the following lines to update the version of Lua
LUA_MAJOR=5.3
LUA_VERSION=5.3.5
LUA_CHECKSUM=112eb10ff04d1b4c9898e121d6bdf54a81482447

PREFIX=/mingw64

printf "Downloading Lua ${LUA_VERSION}\n"
rm -f lua-${LUA_VERSION}.tar.gz
rm -rf lua-${LUA_VERSION}
wget -q https://www.lua.org/ftp/lua-${LUA_VERSION}.tar.gz

#Check the checksum of the downloaded archive
printf "Checking integrity of downloaded files..."
printf "${LUA_CHECKSUM} *lua-${LUA_VERSION}.tar.gz" | sha1sum -c --strict -
if [ $? != 0 ]; then
  exit 1
fi

echo "Unpacking Lua..."
tar -xzf lua-${LUA_VERSION}.tar.gz
rm lua-${LUA_VERSION}.tar.gz
cd lua-${LUA_VERSION}

echo ""
echo "Patching Lua..."
FILE="./lua_makefile.patch"
/bin/cat >$FILE <<EOM
--- Makefile	2016-12-20 17:26:08.000000000 +0100
+++ Makefile.new	2019-06-14 16:45:44.036566000 +0200
@@ -13 +13 @@
-INSTALL_TOP= /usr/local
+INSTALL_TOP= /mingw64
EOM
patch Makefile lua_makefile.patch
rm lua_makefile.patch

echo ""
echo "Building Lua..."
make PLAT=mingw
if [ $? != 0 ]; then
  echo ""
  echo "Build failed exiting..."
  exit 2
fi

echo ""
echo "Installing Lua..."
make install
if [ $? != 0 ]; then
  echo ""
  echo "Installation failed..."
  exit 3
fi

echo ""
echo "Generating pkg-config File..."

FILE="${PREFIX}/lib/pkgconfig/lua.pc"
/bin/cat <<EOM >$FILE
V=${LUA_MAJOR}
R=$LUA_VERSION}

prefix=${PREFIX}
exec_prefix=${prefix}
lib_name=lua${LUA_MAJOR}
libdir=${prefix}/lib/
includedir=${prefix}/include

#
# The following are intended to be used via "pkg-config --variable".

# Install paths for Lua modules.  For example, if a package wants to install
# Lua source modules to the /usr/local tree, call pkg-config with
# "--define-variable=prefix=/usr/local" and "--variable=INSTALL_LMOD".
INSTALL_LMOD=${prefix}/share/lua/${LUA_MAJOR}
INSTALL_CMOD=${prefix}/lib/lua/${LUA_MAJOR}

Name: Lua
Description: Lua language engine
Version: ${LUA_VERSION}
Libs: -L${libdir} -llua
Libs.private: -lm -ldl
Cflags: -I${includedir}
EOM

echo ""
echo "Installation of Lua was successful"
