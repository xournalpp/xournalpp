#!/usr/bin/env bash

version=$(cat ../build/VERSION | sed '1!d')
cat << EOF > xournalpp_version.nsh
!define XOURNALPP_VERSION "$version"
EOF