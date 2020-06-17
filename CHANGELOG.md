# Changelog

## 1.0.19

More bugfixes and improvements due to help from the various community
contributors!

* Changed select object algorithm to be more intuitive
* Added ability for taps with Select Rectangle and Select Region to act like
  Select Object (#1980)
* Improved document loading speed (#2002)
* Added a `--version` command to print the Xournal++ version
* Added a `libgtk` version display to the About dialog
* Added a 16kHz sample rate to audio settings and fixed the 91kHz sample rate
  (#2092)
* Added file version check for future compatibility (#1991)
* Changed wording of new page template dialog to be less confusing (#1524)
* Fixed behavior of "Attach file to the journal" option when choosing "Annotate
  PDF" (#1725, #2106). This now allows the background PDF and the annotation files to
  be renamed and moved as long as they 1) share the same file prefix; and 2)
  share the same relative path.
* Fixed an issue where clicking the X on the replace file dialog would overwrite
  the file (#1983)
* (libcairo >= 1.16 only): Fixed PDF export crashing when the table of contents
  is empty (#2236).
* Fixed a bug where the PDF background would not update when loading a new
  document (#1964)
* Fixed plugin window causing a crash on Ubuntu 16.04
* Fixed a bug where the icon would not appear correctly on some desktop
  environments (#1892)
* Fixed inconsistent ordering of button keybindings (#1961)
* Fixed the Enter key not confirming PDF export settings (#1977)
* Fixed exported PDF title (#2039)
* Fixed a bug where different page backgrounds can cause PDFs to be exported
  with the wrong backgrounds (#2119)
* Fixed a bug where the page number count would not be updated after deleting a
  page (#2134)
* Fixed selection object tool not working correctly (#2081) / crashing (#2133)
  when there are multiple layers

## 1.0.18

* Fixed a crash occurring when recent file entries are invalid (#1730, thanks to
  @iczero)
* Fixed translations not being built correctly, causing packaging issues (#1596)
* Fixed background PDF outlines not being saved in exported PDF (only available
  when compiled with Cairo 1.16 or newer)
* Fixed a deadlock occurring when a second PDF with an outline is opened (#1582).
* Fixed the settings file being written to when it is parsed (#1074, thanks to
  @Guldoman)
* Fixed dark mode icons not loading properly (#1767, thanks to @badshah400)
* Added missing dark mode icons (#1765, thanks to @badshah400)
* Fixed crash in `Export As ...` on some page range options (#1790)
* Fixed crash caused by custom colors in toolbar being "too close" (#1659)
* Windows: Fixed the LaTeX tool always failing to find kpsewhich (#1738). Note
  that to make this work properly, a console window will now flash briefly
  before Xournal++ starts.

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
