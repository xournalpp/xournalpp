#!/usr/bin/env bash

set -e
set -o pipefail

### Step 1: install jhbuild

LOCKFILE="$(dirname "$0")"/jhbuild-version.lock
MODULEFILE="$(dirname "$0")"/xournalpp.modules

get_lockfile_entry() {
    local key="$1"
    # Print the value corresponding to the provided lockfile key to stdout
    sed -e "/$key/!d" -e 's/^[^=]*=//' "$LOCKFILE"
}

install_jhbuild() {
    # Fetch the gtk-osx setup script
    local gtk_osx_commit=$(get_lockfile_entry gtk-osx)
    local gtk_osx_url=https://gitlab.gnome.org/GNOME/gtk-osx/raw/"$gtk_osx_commit"
    curl -LR "$gtk_osx_url"/gtk-osx-setup.sh -o ~/gtk-osx-setup.sh

    # Remove existing jhbuild
    rm -rf ~/gtk ~/.new_local ~/.config/jhbuildrc ~/.config/jhbuildrc-custom ~/Source ~/.cache/jhbuild

    # Build jhbuild
    bash ~/gtk-osx-setup.sh

    # Copy default jhbuild config files from the pinned commit
    curl -ks "$gtk_osx_url"/jhbuildrc-gtk-osx -o ~/.config/jhbuildrc
    curl -ks "$gtk_osx_url"/jhbuildrc-gtk-osx-custom-example -o ~/.config/jhbuildrc-custom
}

configure_jhbuild_envvars() {
    if ! [ -f ~/.zshrc ] || ! grep '"$HOME"/.new_local/bin' ~/.zshrc; then
        echo "Permanently adding jhbuild to path..."
        echo 'export PATH="$HOME"/.new_local/bin:"$PATH"' >> ~/.zshrc
    fi

    # Add jhbuild to PATH for this shell
    export PATH="$HOME"/.new_local/bin:"$PATH"
}

install_jhbuild
configure_jhbuild_envvars

### Step 2: set up custom modulesets

setup_custom_modulesets() {
    local gtk_osx_commit=$(get_lockfile_entry gtk-osx)
    [ -d ~/gtk-osx-custom ] && rm -rf ~/gtk-osx-custom
    mkdir ~/gtk-osx-custom

    # Shallow clone into a commit
    (cd ~/gtk-osx-custom && git init)
    (cd ~/gtk-osx-custom && git remote add origin https://gitlab.gnome.org/GNOME/gtk-osx.git)
    (cd ~/gtk-osx-custom && git fetch --depth 1 origin "$gtk_osx_commit")
    (cd ~/gtk-osx-custom && git checkout FETCH_HEAD)
#     git clone https://gitlab.gnome.org/GNOME/gtk-osx.git ~/gtk-osx-custom
#     (cd ~/gtk-osx-custom && git checkout "$gtk_osx_commit")

    # Set osx deployment target
    sed -i '' -e 's/^setup_sdk()/setup_sdk(target="11.7")/' ~/.config/jhbuildrc-custom

    # Enable custom jhbuild configuration
    cat <<EOF >> ~/.config/jhbuildrc-custom

### BEGIN xournalpp macOS CI
use_local_modulesets = True
moduleset = "gtk-osx.modules"
modulesets_dir = os.path.expanduser("~/gtk-osx-custom/modulesets-stable")

# Workaround for https://gitlab.gnome.org/GNOME/glib/-/issues/2759
module_mesonargs['glib'] = mesonargs + ' -Dobjc_args=-Wno-error=declaration-after-statement'

# Fix freetype build finding brotli installed through brew or ports, causing the
# harfbuzz build to fail when jhbuild's pkg-config cannot find brotli.
module_cmakeargs['freetype-no-harfbuzz'] = ' -DFT_DISABLE_BROTLI=TRUE '
module_cmakeargs['freetype'] = ' -DFT_DISABLE_BROTLI=TRUE '

# portaudio may fail with parallel build, so disable parallel building.
module_makeargs['portaudio'] = ' -j1 '

### END
EOF

    # Fix broken arg overrides in jhbuildrc
    sed -i '' -e 's/^\(module_cmakeargs\["freetype"\]\) =/module_cmakeargs.setdefault("freetype", ""); \1 +=/' ~/.config/jhbuildrc
    sed -i '' -e 's/^\(module_cmakeargs\["freetype-no-harfbuzz"\]\) =/module_cmakeargs.setdefault("freetype-no-harfbuzz", ""); \1 +=/' ~/.config/jhbuildrc
}

setup_custom_modulesets

### Step 3: build gtk (~15 minutes on a Mac Mini M1 w/ 8 cores)
build_gtk() {
    jhbuild bootstrap-gtk-osx
    jhbuild build meta-gtk-osx-gtk3 gtksourceview3
    echo "Finished building gtk"
}

build_gtk

### Step 4: build xournalpp deps

build_xournalpp_deps() {
    jhbuild -m "$MODULEFILE" build meta-xournalpp-deps
}

build_xournalpp_deps

### Step 5: build binary blob

build_binary_blob() {
    local pdeps_commit=$(get_lockfile_entry xournalpp-pipeline-dependencies)
    [ -d ~/xournalpp-pipeline-dependencies ] && rm -rf ~/xournalpp-pipeline-dependencies
    git clone https://github.com/xournalpp/xournalpp-pipeline-dependencies --depth 1 ~/xournalpp-pipeline-dependencies
    (cd ~/xournalpp-pipeline-dependencies && git checkout "$pdeps_commit")
    jhbuild run python3 ~/xournalpp-pipeline-dependencies/gtk/package-gtk-bin.py -o xournalpp-binary-blob.tar.gz
}

build_binary_blob
