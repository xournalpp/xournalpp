# Xournal++ Electronics Engineering Suite

An all-in-one productivity, engineering-drawing, and note-taking toolkit for Xournal++ tailored for Practical Electronics Engineering students using Windows 2-in-1 touchscreen devices with a stylus.

## Features

- **Zero Manual Assets**: All SVGs (passives, semiconductors, logic gates, waveforms, graphs) are generated procedurally by `build_assets.py`.
- **Clutter-Free Interface**: The default user interface is stripped of audio, presentation, and heavy rich-text formatting tools to maximize screen space.
- **Engineering Palette**: Replaces the standard multi-color palette with a dedicated 6-color palette (Circuit Black, VCC Red, Ground Blue, Signal Green, Highlight Yellow, Annotation Purple).
- **One-Click Installation**: An automated `install.bat` makes deploying the suite and plugins straightforward on Windows.

## Installation (Windows)

1. Ensure you have installed [Xournal++](https://github.com/xournalpp/xournalpp) and launched it at least once.
2. (Optional but recommended) Install [Python 3](https://www.python.org/) to regenerate SVGs if you plan to modify `build_assets.py`.
3. Double-click `install.bat` to deploy the suite to `%LOCALAPPDATA%\xournalpp`.
4. Open Xournal++ and ensure the plugin is activated under **Plugin Manager**.

## Configuring Touch & Palm Rejection (Crucial)

For the best note-taking experience, you must disable finger drawing so you can rest your palm while drawing with the stylus.

1. Go to **Edit > Preferences > Input System**.
2. Assign your stylus to **Pen**.
3. Assign your touchscreen to **Touch**.
4. Go to the **Touchscreen** section in Preferences.
5. Check the box for: **"Disable drawing with touchscreen (only panning/zooming)"**.
6. Click **OK**.

## Usage

After installation, your toolbar will feature only the essential drawing and selection tools.
You will see a new **Electronics** menu in the top bar. Use it to insert common schematic symbols and graphs directly onto your active page.
