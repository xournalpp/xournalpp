# go to script directory
cd "${0%/*}"

export PATH="$HOME/.local/bin:$PATH"
export LIBRARY_PATH="$HOME/gtk/inst/lib:$LIBRARY_PATH"

curl -L  http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz -o pa_stable_v190600_20161030.tgz
tar xzf pa_stable_*.tgz

cd portaudio

./configure --enable-cxx --prefix=$HOME/gtk/inst
make -j2
make install
