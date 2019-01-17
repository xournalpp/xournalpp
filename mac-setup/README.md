## Build Xournal++ .app
Do not install macports or homebrew. If you have installed it, you need to
create a new user, and use this for the whole process. jhbuild does not work,
if there is such an environment installed.

### Make sure the Development environment is installed
Open a Terminal, and type in **git**, confirm popup from Appstore with "Install" to install development tools.

### Build Libraries, needs to be once

#### 1. Build GTK
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

Build again. It should now build
````bash
./build-gtk3.sh
````

#### 2. Build Poppler
Execute in this folder.
````bash
./build-poppler.sh
````

#### 3. Build Mac integration
Execute in this folder.
````bash
./build-mac-integration.sh
````

### Build Xournal++
````bash
export PATH="$HOME/.local/bin:$HOME/gtk/inst/bin:$PATH"

cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/gtk/inst ..
make -j 4
make install
````

### Build App
````bash
./build-app.sh
````
