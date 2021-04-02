# How to build for MacOS

The MacOS build will build most prerequisites via macports. Some updated versions are installed via the manual configure/make/make install. The build environment is located in $HOME/gtk/inst for historical reasons. You need to have Apple XCode installed.

```
cd mac-setup
./installmacports.sh
./cleanmacports.sh
./build-gettext.sh
./build-mac-integration.sh
```

Then your build environment is ready and you can create the app bundle

```
./complete-build.sh $HOME/gtk
```
