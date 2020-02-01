# Changelog

## 1.0.17

* Fixed arrow tip scaling: now scales with thickness instead of length (#967,
  thanks to @redweasel)
* Changed coordinate draw direction (thanks to @redweasel)
* Fixed audio playback failures not showing error messages to the user (#1573)
* Fixed text tool bold shortcut not working when capslock is enabled (#1583,
  thanks to @matepak)
* Fixed sidebar preview context menu "Move Page Up" and "Move Page Down" buttons
  not being disabled on the first and last page, respectively (#1637)
* Fixed Enter keypress on the "Goto Page" (Ctrl-G) dialog not changing the page
  (#975, thanks to @MrMallIronmaker)
* Fixed missing Xournal++ icon errors on most of the dialog windows (#1667)
* (Windows) Fixed missing libssl/libcrypto errors in the official installation
  (#1660).

## 1.0.16

* Fixed currently editing textboxes not exporting to PDF.
* Fixed line tool breaking when snap-to-grid is disabled.

## 1.0.15

**Attention**: Please see the 1.0.14 patch notes before installing this version.

* Fixed an issue where copying and pasting strokes would crash the program.

## 1.0.14

**Attention:** users who installed with `make install` will need to follow
special instructions to update. See the "Breaking change" below.

We now officially support packaging Xournal++ using CMake. This allows users to
generate DEB and tar packages. See `readme/linux.md` for more details.

* **Breaking change**: the desktop and icon files were renamed from `xournalpp`
  to `com.github.xournalpp.xournalpp` to be more in line with the AppStream
  specification. This change should only affect users that install with `make
  install`; these users must run `make uninstall` with a cloned version of
  _Xournal++ 1.0.12_ first. We recommend migrating to another installation
  format, such as Flatpak, AppImage, or tarball. Refer to [this
  issue](https://github.com/xournalpp/xournalpp/issues/1442#issuecomment-524566511)
  for more details on how to uninstall if `make uninstall` is unavailable. Refer
  to `readme/LinuxBuild.md` for build instructions.
* Text field
    * (New input system) double/triple text selection in text fields
    * Fixed issue with cursor not showing on mouse movement after typing into
      text field
    * Fixed text not rendering correctly when used with highlighter and
      non-white backgrounds
* Latex tool
    * Now warns users if latex dependencies are missing
    * Newly created objects are now placed in the center of the screen or page
* Image tool
    * Images are now selected by default on insertion
    * Fixed memory leak
    * Fixed an issue where pasted images were zoomed in and ignoring DPI settings
* Quality-of-life changes
    * Improved copy-paste behavior
    * Improved audio recording quality and stability
    * Enable Enter/Shift+Enter to advance search bar
    * Enabled left and right arrow keys to change pages in presentation mode
    * Xournal++ icon is now rendered in thumbnails of Xournal++ files
    * Renamed "Thin" thickness to "Fine"; Added "Very Fine" and "Very Thick"
      thicknesses
    * Added a right-click context menu to the page preview sidebar
* (Experimental) Floating toolbox
    * Can be enabled in preferences by mapping mouse/stylus buttons to Floating Toolbox
    * To use it, create a new custom toolbar in `View > Toolbars > Manage`.
      Switch to the new toolbar, then choose `View > Toolbars > Customize`. Drag
      the desired tools into the floating toolbox (currently, tools may only
      be placed on exactly one of toolbar or the floating toolbox, but not both)
    * This feature is still a work-in-progress and may contain bugs
* Input
    * Improved input detection
    * Added option to (forcefully) assign input classes to devices
    * Fixed Select Object sometimes failing to select overlapping objects
    * (New input system) Fixed pressure sensitivity only changing after program restart
* Preferences window
    * Redesigned layout of the preferences windows
    * Renamed and enhanced tap select/quick select options
* Major bug fixes
    * Fixed some performance issues causing pages to flash
    * Fixed an issue where discarding changes to current file and then selecting
      "Cancel" in the file open dialog caused subsequent saves to fail
    * Fixed an issue where pressing "Cancel" in the export dialog could cause
      the application to crash
    * Fixed a bug introduced in 1.0.12 where "Open File" could crash the
      application on some systems
* Refactoring and other non-user visible changes
* Other misc features and bugfixes
    * Added appdata file
    * Added ability to customize UI styles using CSS
    * Added option to disable scrollbar fade
    * Allow `*.pdf.xopp` filenames. Any file whose name follows the format
      `$filename.pdf.xopp` will be exported as `$filename.pdf`.
    * Added ability for installation to be relocatable
    * Updated translations
    * Updated dependency information for DEB packages
    * Improved file size by reducing stroke coordinate precision
    * Fixed an issue where last eraser thickness settings were not being remembered
    * Fixed pen strokes not appearing after searching
    * Fixed user-defined toolbar shrinking in size after customizing
    * Fixed scrollbar issues caused by window resizes
    * (Windows) Fixed the User Guide webpage not opening (the `Help > Help` menu
      option)
