echo "prefix="
echo "exec_prefix="

libdir1=`pwd`/sox-code/build/src
echo "libdir1=$libdir1"

libdir2=`pwd`/sox-code/build/libgsm
echo "libdir2=$libdir2"

libdir3=`pwd`/sox-code/build/lpc10
echo "libdir3=$libdir3"

incdir=`pwd`/sox-code/src
echo "includedir=$incdir"
echo ""
echo "Name: SoX"
echo "Description: Audio file format and effects library"
echo "Version: 14.4.2"
echo "URL: http://sox.sourceforge.net"
echo "Libs: -L\${libdir1} -L\${libdir2} -L\${libdir3} -llibsox -llpc10 -lgsm -lgomp"
echo "Libs.private: "
echo "Cflags: -I\${includedir}"
