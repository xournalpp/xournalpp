# Changelog

## 1.2.7+dev (Unreleased)

## 1.2.7

The support for Ubuntu 20 LTS is dropped.
This release also includes bugfixes from various community contributors:

* Improved error logs (#6319)
* Fixed some undefined behaviours that may have caused crashes (#6326)
* Fixed some memory leaks (#6356)
* Ensured timers are cancelled on exit (#6351)
* Fixed handling of linebreaks in latex tool (#6374)
* Linux: forward crash signals to system handler after emergency save (#6392)
* Fixed wrong glyph spacing in texts using small fonts (#6393)
* Windows: Fixed opening of UNC paths (#6409)
* Fixed rasterization of Tex elements on PDF/SVG export or printing (#6395)
* Updated translations

## 1.2.6

Bugfixes from various community contributors.

* Fixed a weird behaviour when drawing the outline of the compass multiple times (#6265, #6256)
* Exposed selected text to the windowing system for accessibility purposes (#6221, #6215)
* MacOS: Fixed loading of jpeg images from the PDF background (#6184, #6167)
* MacOS: Fixed CJK and Cyrillic font rendering (#6285, #3474)

## 1.2.5

Bugfixes from various community contributors.

* Fixed a regression causing "Out of memory" error when exporting some PDF files on Windows (#6124)
* Fixed a regression causing a crash when exporting on Linux (#6084)
* Allow undo/redo for changes of page size (#6096)
* Add Ctrl+= as a shortcut for zomming in (#6076)
* Fixed a crash when importing some SVG images (#6087)
* Make the geometry tools appear in the middle of the view (#6012)
* Fixed labels in plugin manager (#5992)
* Fixed missing translations in Windows releases (#5973)
* Fixed many issues with Windows installer (#5988)
* Updated translations

## 1.2.4

Bugfixes from various community contributors.

* Fixed various dark/light theme inconsistencies (#5583)
* Improved edge panning for large selections (#5578)
* Made the "do you want to replace" dialog to stay on top (#5550, #3253)
* Made the OK button default in the PDF Export settings dialog (#5558, #5557) 
* Allow missing terminating newlines and empty lines in .gpl files (#5728, #5724) 
* Fixed thumbnailer previews not always showing up (#5551, #5534)
* Fixed potential issues with sliders and LaTeX preview (#5864)
* Fixed an issue with the list of recent files (#5894)
* Fixed crashes related to undo/redo with page transitioning elements (#5936, #5931 #5910, #1232)
* Fixed a potential crash when entering the presentation mode (#5922,  #2943)
* Fixed a crash in the crash handler (#5622, #5618)
* Fixed a drawing related crash (#5662, #5659)
* Fixed some memory leaks (#5597)
* Windows: Fixed conversion problems with paths (#5640, #5638, #5606)
* MacOS: Fixed titlebar color (#5651)
* MacOS: Added support for opening xopp-files from Finder (#5950)
* Appimage: Added universal input method data (#5603)
* Added Ubuntu 24.04 and MacOS ARM builds (#5898, #5783)
* Fixed inconsistencies in tool names (#5961)
* Updated translations

## 1.2.3

Bugfixes from various community contributors.

* Fixed spline live segment display (#5240, #5237)
* Allowed pulling spline tangents outside of page (#5298, #5277)
* Fixed redundant negative pressure values (#5241, #5238)
* Fixed asymmetrical fill of arrow shapes (#5258)
* Fixed zoom jumping and zoom steps (#5108, #4678)
* Fixed touchpad+trackpad combo (#5288, #5287)
* Fixed long text being truncated in LaTeX tool (#5264, #3293)
* Fixed background color in the LaTeX editor (#5307)
* Fixed window maximize state saving (#5302, #5245)
* Fixed project homepage url not being written into xopp-file (#5322)
* Improved backtrace that now prints source files and line numbers (#5312)
* Made .deb packages being built using the RelWithDebInfo build type (#5344)
* Fixed a bugs with empty repaint regions during PDF text selection, text search
  and while using the vertical tool (#5348, #5343, #5393)
* Fixed stroke replacement preventing a crash while the app is hanging (#5366, #5362)
* Fixed wrongly sized loading pages (#5399, #5102)
* Improved error checks for saving operations avoiding corrupt autosave files (#5361, #4934)
* Fixed OK button sensitivity in the PDF page selecction dialog (#5441)
* Added undo handler for missing PDF replacement (#5443, #5435)
* Fixed a filesystem error on MacOS when a document is autosaved the first time (#5485)
* Fixed the PDF filter in the Export as PDF dialog (#5490)
* Fixed toolbar management segfault and out of date menu (#5510)
* Fixed a bug with corrupt PDF exports when exporting only part of a document with an 
  outline (#5512)
* Enabled font hint metrics in PDF export to improve text alignment (#5454, #5405)
* Added IME support to AppImage (#5520, #5493)

## 1.2.2

Bugfixes from various community contributors.

* Fix undefined behaviour due to signal callbacks wrong type (#5057)
* Fix reference handling of sidebar widgets, fixing crashes and warning (#5083,
  #5074, #4026)
* Default build type is now "RelWithDebInfo" (#5118)
* Fix error message when trying to open an autosave file (#5140)
* Fix empty toolbars being shown (#5160, #5051)
* Add a device id check for input sequences, so devices don't interfere with one
  another anymore (#5170, #5116, #5087)
* Fix a bunch of memory leaks (#5164, #5216)
* Properly read PDF files as binary, fixing some reading issues (#5174, #4740)
* Remove unsafe symbols from document name generation (#5144, #5123)
* Fix crash when a modification happens outside of the cached pages (#5144, 
  #5178, #5162, #5046, #5126)
* Fix audio output settings being off by 1 (#5205)
* Clarify CrashLog message: deleting the crashlog will stop the message from popping up (#5214)
* Fix a crash when pressing both mouse buttons at once (#5210, #5149)

## 1.2.1

Bugfixes from various community contributors.

* Added a default toolbar, so that the app does not start without toolbar on
  new installations (#4997, #5011, #5015).
* Added exception handling to color palettes loading to prevent a possible
  crash (#4994, #5016).
* Added an age filter on crash logs reported at startup. Now only crash logs
  no older than one week will be considered relevant (#5022).
* Fixed opening crash logs and opening the directory containing the crash logs.
  This affected the Windows build and (only for directories) the flatpak and
  snap packages on Linux (#4976).
* Changed the link address when sending a bugreport about a recent crash log
  such that the appropriate template from the xournalpp/xournalpp repository
  is used (#5032, #4910, #5084).
* Added a missing double arrow preference to the button configuration dialog,
  therewith also fixing selections below "Draw Arrow" (#4989).
* Fixed occurring negative pressure values that previously resulted in a crash
  due to a failing assertion (#5025, #5028).
* Added options to the command line and preferences to disable the audio system
  used for audio recording (#3905, #3531, #5020, #5068).
* The official release builds are now built in "RelWithDeb" mode as intended
  before. In particular they will not crash on failing asserts any more (#5042).
* Linux: The official release builds are now compiled with the gtksourceview
  styling for the LaTeX tool (#4996).
* Updated the package description with the new features from v1.2.0 (#5056).

## 1.2.0

This is a new major version of Xournal++ with many new features, improvements,
and bug fixes thanks to contributions from the community.

* **Breaking Changes**:
  * Linux: support for Ubuntu 18.04 has been dropped. Dependency versions are
    now pinned to those used in Ubuntu 20.04.
  * Linux: added an optional dependency on gtksourceview4
* Added PDF text selection tools (#1745, #3326, #4362)
  * Two new tools have been introduced, "Select Linear PDF Text" and "Select PDF
    Text In Rectangle", used for selecting text on background PDFs. The tools
    support "linear" and "area" selection modes. Double/triple clicking enables
    line and paragraph selection, respectively.
  * The toolbox has buttons that allow a user to: 1) copy the selected text,
    2) generate a highlight stroke over the selected text; 3) generate a stroke
    to strike-through the text; 4) generate a stroke to underline the text;
    and 5) toggle between linear and rectangle selection.
  * Clicking on a link with one of the text selection tools will open a popup
    that can be used to navigate to the corresponding PDF page (if it exists) or
    display a clickable URL (#1027, #4412, #4673).
  * Pressing `Ctrl+C` will copy the selected text to the clipboard (#4436,
    #4784).
* Added Setsquare Tool and Compass Tool
  * The Setsquare Tool toggles a virtual setsquare, which can be used for
    measurements and as a guide for drawing straight lines (#3082, #3882,
    #4211, #4532, #4422).
  * The Compass Tool toggles a virtual compass, which can be used for drawing
    circles, arcs, and radial line segments (#4443, #4422).
  * Mouse, stylus, and touchscreen input can be used to interact with the
    Setsquare and Compass Tools.
  * See the website for more information.
* Colors
  * Added new custom color palette support (#2379). A custom color palette can
    be created in `.gpl` format in the Xournal++ config folder. See the website
    guide for information on how to create a `.gpl` file.
  * Changed the color picker so that it opens with the currently selected color
    by default (#4569, #4575).
  * Fixed the color picker dialog appearing on a different screen in
    multi-screen setups (#4519, #4543).
* Image Tool
  * Added support for loading any image format supported by GDK Pixbuf (#3782).
    For backwards compatibility, images are still saved in PNG format.
  * Fixed some bugs with copying and pasting images (#4466).
  * Fixed image orientation data not being handled properly (#4577, #4583).
* Text Tool
  * Added functionality to automatically select the currently editing text if
    switching to a selection tool (#1169, #4315).
  * Fixed some bugs when handling IM input, improving compatibility for some
    IMEs such as `gcin` (#3841, #4937).
  * Fixed text input remaining active after changing pages (#4027).
  * Fixed bold toggle also removing italics (#4417).
  * Fixed pasted text not using the color of the text tool (#4466).
  * Fixed an issue where `Ctrl+Alt+Left` (or right) would cause the layer to
    change instead of selecting the previous (or next) word (#977, #4797).
* Hand Tool
  * Changed selections to hide the controls when the Hand Tool is selected (
    #4419).
  * Added capability to interact with links on the background PDF (#1027,
    #4412, #4673)
* Stroke and eraser tools
  * Added a system cursor option (#3540).
  * Added stroke cap styles to the file format (#3326).
  * Added a "double ended arrow" stroke style (#2362, #3946).
  * Added a Preferences setting for setting the minimum size of the shape
    recognizer (#4283).
  * Added a toolbar item for changing fill opacity (#4263).
  * Added each line style as a new, individual toolbar item that can be added
    in toolbar customization (#4565).
  * Added a Preferences setting to set the visibility of a stylus-activated
    eraser (#4665).
  * Improved minor graphical aspects of the fill opacity dialog (#3515).
  * Improved performance by rendering strokes with integer coordinates (#4080).
  * Improved the arrow stroke style by scaling the arrow head by line length
    for small arrows (#4055).
  * Rewrote the stroke rendering code to fix bugs and improve performance
    (#3480, #3580, #4304).
  * Rewrote the Eraser tool to be faster and more reliable (#3532, #997, #949,
    #3537).
  * Rewrote the ellipse stroke type to reduce the number of points while
    (roughly) maintaining the same look (#4137, #4320).
  * Fixed a bug where stroke stabilization could produce NaN pressure values,
    causing them to become invisible when exported to PDF (#4381, #4401).
  * Fixed issues involving documents that contain strokes with negative or NaN
    pressure values (#4354, #4401). When such documents are now loaded, points
    with NaN pressure values will automatically be removed.
  * Fixed a bug in the stroke stabilizer where the stroke pressure value may
    sometimes not be set on the last point (#4676).
* Toolbars
  * Added new "Export as PDF" toolbar item (#3508). It and the "Print" toolbar
    item have also been added to the default layouts.
  * Added "Spacer" and "Separator" toolbar items for creating extra space in
    toolbars (#4644, #4674).
  * Fixed a crash that occurs if the application is closed while toolbar
    customization is open (#4559).
  * Updated the default `toolbar.ini` (#4670).
* Layers
  * Added a context menu for the layer sidebar preview (#3204).
  * Added a feature for merging layers down. This can be accessed via the
    `Journal`menu or the new layer preview context menu (#2784, #3204).
  * Added `Edit > Move Layer Up/Down` menu items for moving layers up/down
    (#4352).
  * Added a CLI flag to choose which layer(s) to export (#4143).
  * Added two new tools, "Select Multi-Layer Rectangle" and "Select Multi-Layer
    Region", which are like the existing rectangle/region select tools but can
    be used to select objects on a different layer than the active one (#4413).
* Plugins
  * Added new plugin APIs, including export options, getting page info, adding
    strokes, adding images etc. (#3677, #3566, #3688, #4359, #4397, #4858).
  * Added a new plugin "Export" used to export the current document to PDF, SVG,
    or PNG format with `<Shift><Alt>p/s/n`, respectively (#3566).
  * Added a new plugin "HighlightPosition" used to toggle the cursor highlight
    (#3023).
  * Added support for additional plugin loading paths (#1155, #2422). Notably,
    plugins will also be searched for in the `plugins` folder contained in the
    user's config directory (see the "File Locations" section of the
    website/guide for where to find this folder).
  * Added an `ACTION_TOGGLE_PAIRS_PARITY` used for programmatically changing
    between single and paired page mode. This is currently only accessible
    through the plugin API (#4424).
  * Added a new feature to `app.registerUI` to register custom toolbar items
    (#2431, #2936, #4669, #4671).
  * QuickScreenshot plugin: added `maim` and `xclip` program support (#3718).
  * Improved error handling so that the plugin API returns actual Lua errors
    (#4652).
  * Fixed several issues with plugin-disabled builds (#4726, #4728, #4746).
* Audio recordings
  * All elements without audio now become faded when the Play Object tool is
    used (#2874).
  * Windows: fixed issues with saving/loading audio files with unicode
    characters in their names (#3783).
* Vertical Spacing Tool
  * Added capability to move elements above the line by holding Ctrl (#3334).
  * Improved performance (#3334, #4306).
* TeX Tool
  * Reimplemented the LaTeX editor using the gtksourceview library (#2809),
    enabling multiline input, syntax highlighting, undo/redo, word wrap, and a
    resizable code editor.
  * Added a tab for viewing the output of the TeX command (#2809).
  * Added Preferences settings for changing the syntax highlighting theme, word
    wrap settings, font, default formula, and other settings (#2809, #4264).
  * Changed the TeX background color to become dark when a light font color is
    selected (#2809).
  * Fixed a bug where editing existing TeX will cause its position to change
    (#4415, #4430).
  * Fixed a memory leak (#4088)
* PDF support
  * Added "Xournal++" as the "Creator" in exported PDF metadata (#3523).
  * Added button to propose replacement background PDF if it is missing and a
    file with a similar name is found in the same folder as the `.xopp` file
    (#4165)
  * Added a "default PDF export name" setting that is used when exporting PDFs
    to support wildcards/placeholders for name, date, and time (#2897, #4599,
    #4854).
  * Minor performance improvements (#4004, #4704).
  * Fixed a crash that occurs when exporting to PDF from the command line, but
    the background PDF does not exist (#4584, #4681).
* Floating toolbox
  * Added capability to open the floating toolbox with a secondary button
    (#3270).
* View modes (#4514)
  * The individual default, fullscreen, and presentation modes have been
    reorganized into a "View Modes" setting in Preferences.
  * The "Hide menu bar" and "Hide sidebar" settings now work correctly
    with these view modes (#2773).
  * Changed the `F5` shortcut so that it can also be used to leave presentation
    mode. Removed the `ESC` shortcut to leave presentation mode, which
    conflicts with cancelling a selection (#4480).
* Optimizations
  * Refactored the rendering system, leading to performance improvements, bug
    fixes, and code that is easier to extend with new features (#1795, #4051,
    #3480, #3503, #3969, #3985, #3990, #4304, #4137, #4417, #4158, #4159,
    #4687, #4442, #4271).
  * Improved rendering performance by eliminating unnecessary redrawing
    (#4502, #4493, #4659, #4611, #4637, #4672).
  * macOS: significantly improved performance due to GTK version upgrade to
    3.24.38.
* Packaging
  * Added build flags to force or disable X11 linking (#4146).
  * AppImage: added support for AppImageUpdate (#1915, #4265, #4781, #4793).
  * Windows: the installer will now record the installed version in the
    Windows registry (#4548).
    macOS: added a script to build a .dmg for Xournal++ (#4371).
* Miscellaneous
  * Improved file size of documents with large amounts of stroke data by up to
    15% (#4065).
  * Improved the page range validation UX in the export menu (#3390).
  * Added a `Journal > Duplicate Page` menu item (#3831, #4313).
  * Added code to detect and fix documents with pages that have the corrupt page
    type "Copy" (#4139).
  * Added built-in support for the existing Lucide Light and Lucide Dark icon
    themes (#4039, #4063).
  * Added an action for toggling the cursor highlight (#3023).
  * Added a Preferences setting for choosing whether to show the file path in
    the titlebar (#3921, #3934).
  * Added an `Edit > Select All` menu option for selecting all elements on the
    current layer (#1917, #4024).
  * Added margin width and line spacing parameters to be used for
    `pagetemplates.ini`, allowing the vertical margin lines to be moved and the
    line spacing to change (#1713, #3542, #3969).
  * Added a keyboard shortcut `Ctrl+Shift+Z` for redo (#1323, #4292).
  * Added a menu item to the bottom of the recent files list for clearing the
    recent files list (#4410, #4440).
  * Added keybindings for switching to the first 10 colors in the palette to
    keys 0-9 (#2007, #4416).
  * Added vim keybindings `Shift+j` and `Shift+k` for page scrolling (#4573).
  * Refactored key event handling such that key-press and key-release events
    are first propogated to the focus window and up the focus chain container,
    thus allowing for plugins using keyboard shortcuts that would otherwise
    conflict with the text tool (#4797, #4811).
  * Added a new setting for automatically appending a new page to the document
    when scrolling to the last page (#1343, #4311).
  * Added page numbers to the sidebar page previews, which can be configured
    in the Preferences (#4624, #4693).
  * Changed version numbers in About window to be selectable text (#4152).
  * Changed the About dialog to hide the Git commit info if it is not found
    (#4537).
  * Changed the autosave directory from the user config folder to the user cache
    folder (#3587).
  * Changed the "Apply to current page" button so that it is disabled in the
    page type dropdown menu when "Copy current page" is selected (#4142, #4255).
    This helps to ensure that the corrupt "Copy" page background type cannot
    actually be produced in a document (see #4127).
  * Fixed the cursor highlight clipping when the sum of the radius and border
    width exceed 60 pixels (#3023).
  * Fixed the cursor highlight not showing with "None" cursor type (#3023).
  * Fixed the cursor not immediately changing when activating the vertical space
    tool (#4851, #4864).
  * Fixed a scrolling bug when undoing page insertion (#2757, #4875).
  * Linux: fixed Hand Recognition not working on X11 (#4043).
  * Fixed a bug where floating point values would not be parsed correctly in
    `pagetemplates.ini` (#3655, #3676).
  * Fixed several memory leaks and other issues (#3748, #4554, #4679, #4680).
  * Fixed a bug where grid backgrounds with borders would not be rendered
    correctly (#3969).
  * Fixed the line style toolbar item not being updated correctly when selecting
    existing stroke elements (#3793, #4089).
  * Fixed a crash that occurs when using Annotate PDF to open a non-PDF file
    (#3368, #4526).
  * Fixed potential race conditions (#4533).
  * Fixed a potential crash that can occur when adding a color selection
    toolbar item to a side toolbar (#4505, #4535).
  * Fixed an issue where failing to create a backup during saving would silently
    abort saving instead of informing the user (#4675, #4715).
  * Fixed a bug where undo/redo would not deselect the current selection (#4609,
    #4725).
  * Added undo/redo for selections moved via keyboard (#4835, #4842).
  * Fixed a bug with changing linestyle via toolbar or plugins not working
    properly (#4831, #4827, #4823).
  * Fixed a "File Bug Report" dialog that has never been shown but should now be
    shown when an error log is detected on startup (#4808).
  * Improved the CrashLog by adding the Xournal++ and Gtk versions as well as
    every log message issued via g_message, g_critical, g_debug and so on (#4848).
  * Fixed a mismatch between the app id the window sets and the name of the desktop
    file, leading to the Wayland icon being shown instead of the Xournal++ icon.
    The program name and StarupWMClass were therefore changed from xournalpp to
    com.github.xournalpp.xournalpp (#4870, #4887).
  * Replaced deprecated icons, which are missing on newer versions of GNOME as
    well as Windows and macOS builds (#3945, #4544).
  * macOS: fixed clipboard image paste not working (#3455, #4489).
  * macOS: fixed audio recording by compiling libsndfile with external libraries
    (#4656, #4371).
  * Removed old MIME types from XDG desktop metadata files (#4130).
  * Fixed header includes for compiling with GCC 13 (#4912).
  * Define MAX_PATH if undefined to address Hurd build failure (#4962).
  * Other minor code cleanup and improvements.
  * Initial preparatory work to migrate to GTK4 (#4441, #4448).
  * Updated translations.

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
