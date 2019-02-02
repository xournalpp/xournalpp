echo "prefix="
echo "exec_prefix="
echo "libdir="

incdir=`pwd`/sox-code/src

echo "includedir=$incdir"
echo ""
echo "Name: SoX"
echo "Description: Audio file format and effects library"
echo "Version: "
echo "URL: http://sox.sourceforge.net"
echo "Libs: -L${libdir} -lsox"
echo "Libs.private: "
echo "Cflags: -I\${includedir}"
