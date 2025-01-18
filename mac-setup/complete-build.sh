#!/usr/bin/env bash

set -e
set -o pipefail

# Build and package the application
if [ $# -eq 0 ]; then
  echo 'Please provide the path of your gtk prefix parent (example: ~/gtk)'
  exit 1
fi

ENSURE_JHBUILD=
if [ -z "$UNDER_JHBUILD" ]; then
    if ! which jhbuild > /dev/null; then
        echo "Not running jhbuild environment..."
        echo "Warning: jhbuild not found on PATH! This script may not work as expected."
    else
        echo "Not running jhbuild environment, prepending jhbuild to every command"
        ENSURE_JHBUILD="jhbuild run"
    fi
fi

root_dir=$(python3 -c 'import pathlib, sys; print(pathlib.Path(sys.argv[1]).resolve().parent.parent)' "$0")
prefix_parent="$1"
[ ! -d "$prefix_parent" ] && echo "$prefix_parent doesn't exist." && exit 1
[ ! -d "$prefix_parent"/inst ] && echo "$prefix_parent/inst doesn't exist." && exit 1

build_dir="$root_dir"/build
mkdir -p "$build_dir"

pushd "$build_dir"
$ENSURE_JHBUILD cmake .. -GNinja
$ENSURE_JHBUILD cmake --build .
$ENSURE_JHBUILD cmake --install . --prefix "$prefix_parent"/inst
popd

$ENSURE_JHBUILD bash ./build-app.sh "$prefix_parent"
