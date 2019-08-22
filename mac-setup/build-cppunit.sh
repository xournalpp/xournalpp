# go to script directory
cd "${0%/*}"

export PATH="$HOME/.new_local/bin:$PATH"
export LIBRARY_PATH="$HOME/gtk/inst/lib:$LIBRARY_PATH"

curl -L  http://dev-www.libreoffice.org/src/cppunit-1.14.0.tar.gz -o cppunit-1.14.0.tar.gz
tar xzf cppunit-1.14.0.tar.gz

cd cppunit-1.14.0

./autogen.sh
./configure --prefix=$HOME/gtk/inst
make
make install
