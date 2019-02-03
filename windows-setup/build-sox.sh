#!/bin/bash

# go to script directory
cd "${0%/*}"

# Generation does not work...
# So do this manually...
./sox-pc.sh > /mingw64/lib/pkgconfig/sox.pc

git clone https://git.code.sf.net/p/sox/code sox-code

cd sox-code
git checkout tags/sox-14.4.2 -b sox-14.4.2

mkdir build
cd build

echo "TODO Manual changes required:"
echo "Open CMakeLists.txt and remove -fstack-protector after"
echo "if(CMAKE_COMPILER_IS_GNUCC)"
echo "after this run the rest of the script manually!"
exit;
#if(CMAKE_COMPILER_IS_GNUCC)
#	add_definitions(-Wall -W -Wmissing-prototypes -Wstrict-prototypes -pedantic -Wno-format -Wno-long-long)
#endif(CMAKE_COMPILER_IS_GNUCC)


cmake -G "MSYS Makefiles" -DCMAKE_INSTALL_PREFIX=/mingw64 ..

make -j2
#make install

