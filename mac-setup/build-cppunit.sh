# go to script directory
cd "${0%/*}"

export PATH="$HOME/.new_local/bin:$PATH"
export LIBRARY_PATH="$HOME/gtk/inst/lib:$LIBRARY_PATH"

curl -L  http://dev-www.libreoffice.org/src/cppunit-1.14.0.tar.gz -o cppunit.tar.gz
tar xzf cppunit.tar.gz

cd cppunit

./autogen.sh
./configure
make
make install
