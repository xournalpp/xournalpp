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
    [ -d ~/gtk-osx-custom ] && rm -rf ~/gtk-osx-custom
    mkdir ~/gtk-osx-custom

    # Shallow clone into a commit
    (cd ~/gtk-osx-custom && git init)
    (cd ~/gtk-osx-custom && git remote add origin https://gitlab.gnome.org/GNOME/gtk-osx.git)
    (cd ~/gtk-osx-custom && git fetch --depth 1 origin "$gtk_osx_commit")
    (cd ~/gtk-osx-custom && git checkout FETCH_HEAD)

    # Remove existing jhbuild
    rm -rf ~/gtk ~/.new_local ~/.config/jhbuildrc ~/.config/jhbuildrc-custom ~/Source ~/.cache/jhbuild

    # Build jhbuild
    bash ~/gtk-osx-custom/gtk-osx-setup.sh

    # Copy default jhbuild config files from the pinned commit
    install -m644 ~/gtk-osx-custom/jhbuildrc-gtk-osx ~/.config/jhbuildrc
    install -m644 ~/gtk-osx-custom/jhbuildrc-gtk-osx-custom-example ~/.config/jhbuildrc-custom
}

configure_jhbuild_envvars() {
    if ! [ -f ~/.zshrc ] || ! grep '"$HOME"/.new_local/bin' ~/.zshrc; then
        echo "Permanently adding jhbuild to path..."
        echo 'export PATH="$HOME"/.new_local/bin:"$PATH"' >> ~/.zshrc
    fi

    # Add jhbuild to PATH for this shell
    export PATH="$HOME"/.new_local/bin:"$PATH"
}

setup_custom_modulesets() {
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

    echo "interact = False" >> ~/.config/jhbuildrc
}

echo "::group::Setup jhbuild"
install_jhbuild
configure_jhbuild_envvars
setup_custom_modulesets
echo "::endgroup::"


### Step 2: Download all sources
download() {
    jhbuild update meta-gtk-osx-gtk3 gtksourceview3
    jhbuild -m "$MODULEFILE" update meta-xournalpp-deps
    jhbuild -m ~/gtk-osx-custom/modulesets-stable/bootstrap.modules update meta-bootstrap
    echo "Downloaded all jhbuild modules' sources"

    local pdeps_commit=$(get_lockfile_entry xournalpp-pipeline-dependencies)
    [ -d ~/xournalpp-pipeline-dependencies ] && rm -rf ~/xournalpp-pipeline-dependencies
    git clone https://github.com/xournalpp/xournalpp-pipeline-dependencies --depth 1 ~/xournalpp-pipeline-dependencies
    (cd ~/xournalpp-pipeline-dependencies && git checkout "$pdeps_commit")
}
echo "::group::Download sources"
download
echo "::endgroup::"

### Step 3: bootstrap
bootstrap_jhbuild() {
    jhbuild -m ~/gtk-osx-custom/modulesets-stable/bootstrap.modules build --no-network meta-bootstrap
}
echo "::group::Bootstrap jhbuild"
bootstrap_jhbuild
echo "::endgroup::"


### Step 4: build gtk (~15 minutes on a Mac Mini M1 w/ 8 cores)
build_gtk() {
    jhbuild build --no-network meta-gtk-osx-gtk3 gtksourceview3
    echo "Finished building gtk"
}
echo "::group::Build gtk"
build_gtk
echo "::endgroup::"


### Step 5: build xournalpp deps

build_xournalpp_deps() {
    jhbuild -m "$MODULEFILE" build --no-network meta-xournalpp-deps
}
echo "::group::Build deps"
build_xournalpp_deps
echo "::endgroup::"


### Step 6: build binary blob

build_binary_blob() {
    jhbuild run python3 ~/xournalpp-pipeline-dependencies/gtk/package-gtk-bin.py -o xournalpp-binary-blob.tar.gz
}
echo "::group::Build blob"
build_binary_blob
echo "::endgroup::"
