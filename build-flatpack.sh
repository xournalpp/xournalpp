#!/bin/bash

flatpak-builder ./flatpak-build org.github.xournalpp.xournalpp.yaml --force-clean --repo=./repo 
flatpak build-bundle ./repo  xournalpp.flatpak com.github.xournalpp.xournalpp
