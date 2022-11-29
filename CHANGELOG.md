# Changelog

## 1.1.3+dev (Unreleased)

## 1.1.3

* Fixed a crash that occurs when trying to add/edit/test LaTeX if LaTeX is not found (#4334, #4345)
* Fixed a crash that could occur when quitting when using a touchscreen device (#4343, #4373)
* Fixed a bug that closed text fields immediately after creating them with a secondary mouse or stylus button (#3983, #4309)
* Added a setting in the preferences to disable Gtk inertial scrolling, avoiding weird scrolling on some devices (#4013, #4420)
* Linux: added an Ubuntu 22.04 release build (#4411)

## 1.1.2

Bugfixes from various community contributors.

* Fixed application window being dragged unintentionally when dragging from
  toolbox or menu bars (#3901, #3907).
* Fixed CLI export failures caused by not using the C locale (#3939, #3944).
* Fixed a bug where the Default Tool does not update the tool icon (#3918,
  #3947).
* Fixed a bug where currently editing text boxes would not unfocus when moving
  to a different page (#3765, #4027).
* Fixed stylus/mouse inputs being detected twice for some tools (#4162).
* Fixed a crash that occurs when a plugin calls `app.getDocumentStructure()`
  (#4192, #4196).
* Fixed a memory leak that occurs when using the Stroke Recognizer (#4258).
* Fixed a crash that occurs on some Linux distros when opening a save dialog
  after using the text tool (#4279, #4280).
* Fixed a potential crash in toolbar customization (#4293).
* Fixed a crash that can occur when "Apply Background to All Pages" is undoed
  and then another action is performed (#4139, #4297).
* Fixed PDF Attach Mode settings not loading correctly (#3189, #4228).
* Improved performance of Text Tool rendering by eliminating unnecessary redraws
  (#3810, #4000, #4074, #4135, #4136).
* Improved LaTeX Tool error message when running inside Flatpak by providing
  steps on how to install the TeXLive extension (#3996, #4259).
* Windows: fixed a crash in the installer (#4041, #4062).
* macOS: changed the way the Xournal++ is launched to avoid some potential
  issues (#4087, #4090, #4287).
* Linux: fixed linker errors associated with backtrace library (#3817, #3819).

## 1.1.1

Bugfixes from various community contributors.

* Added `Minimal Top` and `Minimal Side` default toolbar layouts.
* Added code to use GTK dark theme variant on startup if "Use Dark Theme" is
  enabled in Preferences (#2771).
* Improved Ctrl+Scroll zooming (#3358).
* Improved behavior of stroke recognizer when a stroke is recognized as a line
  (#3279, #3285).
* Changed "Content" tab in sidebar to be hidden instead of disabled if no PDF
  outline is available (#3359).
* Changed panning behavior when an object is selected (#2893, #3776)
  * The panning speed scales linearly up to some maximum multiplier,
    configurable in Preferences under `View > Selection Edge Panning`.
  * This fixes the absurdly fast edge pan speed bug (#2889).
* Changed the application `.svg` icon with minified versions (#3345), fixing a
  bug in the icon transparency in KDE Plasma (#3280).
* Fixed pinch-to-zoom calculation error when using `Drawing Area > Scrolling
  outside the page` vertical/horizontal space options (#3298, #3372).
* Fixed a crash that occurs when using the `gcin` IME with the text tool
  (#3315, #3500, #3511).
* Fixed freezing when annotating PDF files (#3585, #3593, #3761).
* Fixed a crash that occurs when "autoload most recent" is enabled but no recent
  files are available (#3734, #3738).
* Fixed inconsistent rendering of filled highligher strokes (#2904, #3355).
* Fixed a bug where single dot strokes would not use pressure sensitivity
  (#1534, #3344).
* Fixed a bug where the first point in a highlight stroke would incorrectly have
  a pressure value when it should not (#3651, #3652).
* Fixed a bug where snapping would prevent text being created close to each
  other (#3352, #3353).
* Fixed a bug where "PDF background missing" would appear behind a transparent
  image background (#3185, #3350).
* Fixed a bug where toolbar button locations would be off-by-one after
  restarting the program (#2970, #2980).
* Fixed undo/redo of layer renaming also affecting the currently selected layer
  (#3257, #3297).
* Fixed a bug where PDF outlines would be exported incorrectly with some locales
  (#3388, #3551).
* Fixed a bug where the PDF background selection dialog that appears when adding
  a new page would not show the last row of PDF page thumbnails (#3744).
* Fixed a bug where a non-lowercase PDF file extension like `.PDF` would cause
  PDF loading to fail (#3548, #3590).
* Fixed a bug where `.xopp` files with dots before the `.xopp` file extension
  would be saved with the wrong file name (#3330, #3333).
* Fixed an issue where backups created during save (e.g., `~*.xopp` files) are
  not deleted (#1498, #3399, #3445).
* Fixed various memory leaks and related bugs (#3392, #3420).
* Fixed the MigrateFontSizes plugin not loading correctly when both GTK 3 and
  GTK 4 are installed (#3428).
* Fixed issues with version number information on some platforms (#2820, #3492).
* Linux: Fixed a bug where tools would be activated on hover when
  TabletPCButtonEnabled is set (#3724, #3658, #3701).
* MacOS: updated the application so that it runs on MacOS Monterey (#3759).
* Windows: Fixed a bug that caused some input methods for non-English text
  (e.g., Chinese, Korean, etc.) to not work (#1997, #3402).
* Windows: Fixed several bugs causing fonts to load incorrectly, including when
  the application is opened outside of the installation `bin` folder or when
  other languages are used (#3207, #3371, #3474, #3534, #3477, #3426).
* Windows: Fixed an issue where a console window would briefly flash when
  starting the application (#2704).
* Updated translations.
* Other non-user-visible internal refactoring, minor bug fixes, and potential
  performance improvements.

## 1.1.0

This is a new major version of Xournal++ with many new features, improvements,
and bug fixes thanks to over one year's worth of contributions from the
community.

* **Breaking changes**:
  * Xournal++ now follows the [XDG Base Directory
      Specification](https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html)
      (#1101, #1384). The configuration files will now be stored in an
      appropriate platform-specific user data folder. Old configuration files
      will be copied automatically if the new user data folder does not exist.
  * For users that have non-default DPI settings, text elements in old
      documents may be displayed with incorrect sizes/positions. A new plugin is
      included to fix the positioning of these text elements. See the relevant
      bug in the "Text tool" section below.
  * The old input system has been removed, which may affect some users.
  * Windows: The installer has been updated, fixing many issues such as:
      incorrectly set up registry keys, missing uninstaller entry, missing
      entries in Open With context menu in Explorer, missing icons, etc. (#2606,
      #2585, #2387, #2141, #2036, #1903, #2666, #3215). When upgrading from
      1.0.18, you **must not run the uninstaller**; instead, run the updated
      installer, which will clean up files from the old install and fix the
      registry key entries. **If you are downgrading from 1.1.0 to 1.0.18 or
      older, you must run the uninstaller first.**
  * Linux: Support for Ubuntu 16.04 (and older distros) has been dropped.
      Please use a distro from 2018 or later, such as Ubuntu 18.04 or Debian
      Buster. The dependency versions have been updated accordingly.
  * Linux: The thumbnailer program has been renamed from `xournal-thumbnailer`
      to `xournalpp-thumbnailer` in order to fix tab completion (#1752).
  * Linux: Xournal++ now has a hard dependency on `librsvg`. This should not affect
      end-users, only maintainers and packagers.
  * The code has been updated to use C++17 (#1485) and must now be compiled
      using a supported compiler version, such as GCC 7 or Clang 5 (or newer).
  * MacOS: Dropped support for macOS High Sierra; minimal version is now
      Catalina (#2989)
* Document viewing
  * Changed page selection system to now select the current page during
      scrolling (#1613, #1184).
  * Improved page load performance and memory consumption with new page
      preload mechanism (#2603).
  * Fixed a bug where scrolling would not work with zoom fit enabled until the
    zoom level changes (#2541).
  * Fixed some bugs that prevented documents from being displayed on ARM and
      32-bit devices (#2576).
  * Fixed vim-style `hjlk` keybindings being inconsistent with the arrow keys
      (#2712).
  * Added a shortcut for the default tool (#2872) and changed shortcuts for
      page deletion and layer navigation (#2766)
  * Fixed various issues related to zooming and scrolling (#2435, #2743,
      #2023, #1830, #2821)
  * Fixed zoom slider tick marks being set to the wrong values when DPI
      calibration setting is different from the default (#2923)
  * Fixed a freeze caused by scrolling between pages of different size (#2770,
      #3099).
* Document export
  * Added a "progressive mode" option to PDF file export dialog. This will
      render layers from bottom to top, exporting a new page every time a layer
      is rendered (#2589, #2609).
  * Simplified background rendering to improve compatibility of exported SVGs
      (#2598).
  * Made line spacing equal in export and on view when pango version >= 1.48.5 is
      available (#2182)
  * Updated the Cairo version on Windows to fix a bug that created corrupt PDF
      files on export (#2871)
  * Fixed a crash that occurs when closing the application before export
      finishes (#3159).
* Sidebar preview panel
  * Added new "Layerstack Preview" tab that shows all layers up to the current
      layer (#2795).
  * Changed sidebar colors to be dark when using a dark theme (#2726).
  * Changed layer previews to only show background in background layer (#2674)
  * Moved close button from the bottom to the top to improve usability
      (#2727).
  * Fixed button tooltips not reflecting the page/layer tabs (#2776).
  * Fixed a bug where the buttons would be enabled/disabled inconsistently
      (#2776).
* Audio playback
  * Added seeking functionality during playback (#1520)
  * Fixed crashes caused by race conditions in the audio system
  * Fixed bug where gaps in the audio stream could appear while recording
  * Added an error message popup which displays when a recording fails to load
      or play (#1573)
* Input System
  * Removed the old input system and touch workaround, both of which have been
      deprecated (#2308).
  * Added a `Mouse+Keyboard` device class for handling e.g. wireless USB
      mouse/keyboard receivers (#1769, #1785).
  * Added Preference settings for minimum pressure level and pressure
      multiplier (#2622).
  * Added an experimental stroke smoothing / input stabilization feature
      (#2512, #2856, #2863).
  * Added a touchpad pinch gesture for zooming (#2651).
  * Added a Preferences setting to ignore the first few pen input events when
      starting a new stroke (#1854).
  * Reimplemented zoom gestures for better compatibility (#1528)
  * Improved tool handling (#2339)
  * Fixed a bug where the touchscreen could not be used to pan and zoom when
      touch drawing is enabled (#2435).
  * Fixed a bug where two-finger zoom would be triggered even when zoom
      gestures are disabled (#2510).
  * Fixed touch drawing not working with the pen tool (#2123).
* LaTeX tool
  * Reworked LaTeX tool implementation (#1952).
  * Added a new tab in the Preferences window for LaTeX configuration.
  * Added a `global template file` setting for custom LaTeX template files
      to be used when rendering LaTeX formulas (#1188).
  * Added a button in the Preferences window for testing LaTeX setup.
  * Fixed a bug where closing the dialog before the initial render would crash
      the application (#2728, #2798).
  * Fixed a bug where line breaks would not be saved correctly (#2849).
  * Windows: Fixed a bug where long user names would break the LaTeX tool
      (#3046).
* Spline tool
  * Added cubic splines as a drawing tool (#1688, #1798, #1861).
  * Click to add anchor points (knots) and drag to create non-trivial
      tangents. Backspace key, arrow keys, s and Shift+s allow to delete/modifiy
      the last set knot/its tangent. Escape key and double click exit the spline
      drawing mode.
* Snapping
  * Added snapping for vertical space (#2011)
  * Added snapping for moving and resizing selections (#1972, #2011)
  * Added snapping for recognized shapes (optional setting; #2011)
  * Added a Preferences settings to preserve line width while resizing a
      selection (#2011)
  * Added a Preferences setting to change the snap grid size (#1920).
  * Fixed a bug in the grid snapping tolerance (#2779).
* Selections
  * Added ability to mirror selected elements when scaling in a negative
      direction (#2723).
  * Added `Edit > Arrange` menu items and the corresponding actions for
      rearranging selected elements (#2794).
  * Changed element selection to not automatically rearrange items (#1680).
      Instead, rearranging must be performed with the newly added menu entries.
  * Fixed some bugs where selections would not be copied correctly (#2277,
      #2090, #2733) and would cause strokes to become invalid/missing when
      saving (#2857, #2464).
  * Fixed a bug in the Select Object algorithm (#2478)
* Pen and eraser tools
  * Added Preferences settings to configure the radius, color, and border of
      the cursor highlight when `Highlight cursor position` is enabled (#1891,
      #1898).
  * Added a new "no cursor" cursor type and changed "Big pen" checkbox in
      Preferences into a combo box (#2111).
  * Renamed "fill transparency" to "fill opacity" to avoid confusion (#2590).
  * Added thick/thin settings to default tool preferences (#2611).
  * Added ability to change line styles of existing strokes (#2641).
  * Changed name of "Draw Circle" to "Draw Ellipse" (#2708).
  * Changed name of "Ruler" to "Draw Line" (#2959).
  * Improved circle drawing controls (#2707).
  * Improved the accuracy of the eraser tool (#1818).
  * Changed pen/highlighter cursor to be in the shape of a circle with the
      approximate stroke size (#1945, #1513).
  * Fixed a cursor update bug (#1954).
  * Fixed strange behavior of color switches when temporarily using the eraser
      (#2004, #1546, #1712).
* Text tool
  * Added support for text edit blinking to be enabled/disabled through
      standard GTK configuration settings (#2170).
  * Fixed several serious bugs and user experience issues with IME pre-edit
      strings (#2789, #2788, #2826, #2851).
  * Fixed a bug where the font button would not be updated when editing a text
      field (#2620).
  * Fixed a bug where text elements would not be displayed at the correct
      positions when an image is used as the page background (#2725).
  * Fixed a bug where text would be displayed with an incorrect size when DPI
      is set to a non-default value. A plugin for migrating documents with wrong
      font sizes has been added (#2724).
  * Fixed a bug where selected text would be highlighted incorrectly (#3131).
* Toolbars
  * Added a print button to the default toolbar (#1921).
  * Added a menu toggle item for showing/hiding the toolbar, bound to F9
      (#2112).
  * Added a vertical mode for the pagespinner tool (#2624).
  * Added color indicators to toolbars when customizing the toolbars (#2726).
  * Improved appearance of the floating toolbar (#2726).
  * Fixed a crash that occurs when the application is closed with the toolbar
      customization dialog open (#1189).
  * Fixed multiple bugs involving the toolbar customizer (#2860).
* Plugins
  * Extended plugin API with many new features and functions, including page
      and layer operations (#2406, #2950).
  * Added a Lua plugin for taking a screenshot and saving it to a file
      (#2086, #2787).
  * Added a Lua plugin for cycling though a color list (#1835, #2251).
  * Added Lua plugin support for MacOS (#2986)
  * Allow using the system Lua package path (#2968)
* Paper backgrounds
  * Added an isometric paper background type (#1994).
  * Changed background types to use lighter line colors when a dark background
      is set. The colors can be set in `pagetemplates.ini` (#2055, #2352).
  * Fixed the confusing behavior of the `Apply to current/all pages` buttons
      used to change the page backgrounds (#2730).
  * Fixed cloned background images not loading correctly (#3170).
* Packaging changes
  * AppImage: Fixed AppImages not running on more recent Linux distros
      (#2600).
  * Linux: Fixed an issue with dock icons not appearing correctly in some
      desktop environments (#2881, #1791).
  * Debian packages: added man pages (#2701)
  * The `lua-lgi` package has been added to the list of `Recommended`
      dependencies. It is useful for creating GUI in Lua plugins.
  * Streamlined and updated package metadata (#3094).
* Misc
  * Updated author information and About dialog appearance (#3209)
  * New action icons (#3154) and new application icon (#2557).
  * Changed the error dialog for missing PDF backgrounds to display the full
      path of the missing PDF (#3140).
  * Changed default key binding of middle mouse button from nothing to hand
      tool (#3121).
  * Changed the `Help > Help` menu item to point to the new website
      [www.xournalpp.github.io](www.xournalpp.github.io), which replaces the
      User Manual wiki.
  * Added a setting in the Preferences window for selecting the language
      (#2188). Simplified language translation file names (#3166, #3201).
  * Added a feature to allow the user to modify the locale directory via the
      TEXTDOMAINDIR environment variable (#2600, #2845).
  * Added support for more export options in command line and GUI (#2449)
  * Added a command line option to create a xopp file (#1919).
  * Added the `Journal > Rename Layer` menu entry to rename layers (#2321).
  * Added the `Journal > Append New PDF Pages` menu entry to append PDF pages
      that are not in the current annotation file (#2146)
  * Improved look of the Preferences window (#2592).
  * Improved Print Dialog verbosity and error handling (#3002)
  * Fixed a bug where the `Autoloading Journals` option would only autoload
      annotation files with `.pdf.xopp` or `.pdf.xoj` extensions. (#2911,
      #3217).
  * Fixed bugs in element cloning, which previously could have caused elements
      to become invalid (#2733, #2720, #2464).
  * Fixed a bug where the thumbnailer would not correctly render previews in
      file managers that sandbox their thumbnailers (#2738).
  * Fixed a bug where some error message dialogs would not display the message
      correctly (#3214).
  * Fixed keyboard shortcuts not working when the menubar is hidden (#2324)
  * Fixed the undo operation for moving objects across page borders (#3068)
  * Updated the translation files.
  * Non-visible refactoring and code cleanup (#1279, #2150, #1944, #2199,
      #2213, #2252, etc.)
  * MacOS: Fixed the startup crash on BigSur and removed the integration
      of Xournal++ into the Mac Menu Bar (#2836, #2976)
  * Windows: Fixed a crash that occurs when closing the application (#2218).

## 1.0.20

More bugfixes.

* Fixed a regression with pdf files that could not be overwritten (#2355)
* Fixed page layout update after inserting or deleting a page, changing the page
  layout or zooming (#1777, #2346, #2411)
* Fixed incorrect rendering of pages after changing the page format (#2457)
* Fixed blocked scrolling after saving a file (#2062)
* Fixed presentation mode after startup

## 1.0.19

More bugfixes and improvements due to help from the various community
contributors!

* Changed select object algorithm to be more intuitive (#1881).
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
