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

```sh
mkdir build
cd build

cmake ..

cmake --build . # For a faster build, set the flag -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

- Use `cmake-gui ..` to graphically configure CMake

## Run

```sh
# Before running this command, ensure you're in the './build' directory
./xournalpp
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
