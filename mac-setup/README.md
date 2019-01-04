## Build Xournal++ .app
Do not install macports or homebrew. If you have installed it, you need to
create a new user, and use this for the whole process. jhbuild does not work,
if there is such an environment installed.

### Make sure the Development environement is installed
Open a Terminal, and type in **git**, confirm popup from Appstore with "Install" to install development tools.

### Build GTK
Execute in this folder.
````bash
./build-gtk3.sh
````

The build will fail. After first failure (missing python module six)
Download from here: https://pypi.org/project/six/
Execute
````bash
$HOME/gtk/inst/bin/python setup.py install
````

### Build Xournal++
````bash
cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/gtk/inst -DENABLE_MATHTEX=OFF ..
make -j 4
make install
````

### Build App
````bash
./build-app.sh
````
