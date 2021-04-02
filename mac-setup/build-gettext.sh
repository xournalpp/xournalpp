export PATH="/usr/bin:/bin"

gettextversion=0.21
rm -rf ./gettext
curl -L https://ftp.gnu.org/pub/gnu/gettext/gettext-$gettextversion.tar.xz -o gettext.tar.xy
tar -xJf ./gettext.tar.xy
rm ./gettext.tar.xy
mv gettext-$gettextversion gettext
cd gettext
./configure --prefix=$HOME/gtk/inst \
  --disable-csharp \
  --disable-java \
  --disable-native-java \
  --disable-openmp \
  --without-emacs \
  --with-included-gettext \
  --with-included-glib \
  --with-included-libcroco \
  --with-included-libunistring \
  --with-included-libxml

make -j8
make install
