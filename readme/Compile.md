# Compilation and Testing

This file contains platform-independent instructions for compiling and developing locally. For instructions on how to create platform-specific packages, or instructions on preliminary steps for building Xournal++, please see these platform-specific instructions:

- [LinuxBuild.md](./LinuxBuild.md)
- [MacBuild.md](./MacBuild.md)
- [WindowsBuild.md](./WindowsBuild.md)

## Get sources

```sh
git clone http://github.com/xournalpp/xournalpp
cd xournalpp
```

## Compile
For testing purposes, install in a subdirectory:

```sh
mkdir build
cd build

# For a faster build, remove this '#' ~~⇓
cmake .. -DCMAKE_INSTALL_PREFIX=install #-DCMAKE_BUILD_TYPE=RelWithDebInfo 

cmake --build . 
cmake --build . --target install
```

- Use `cmake-gui ..` to graphically configure CMake
- Running without building the `install` target will most likely not work, as
some resources need to be generated and located in the right directories.

## Run

```sh
# Before running this command, ensure you're in the './build' directory
./install/bin/xournalpp
```

## Test

The unit tests can be enabled by setting `-DENABLE_GTEST=on` when running the
CMake command. This requires having `googletest` available, either through your
system's package manager or by setting `-DDOWNLOAD_GTEST=on` to automatically
download and build `googletest`.

```sh
mkdir build
cd build

cmake .. -DENABLE_GTEST=on

# Build unit test executables
cmake --build . --target test-units

# Run unit tests
cmake --build . --target test
```
