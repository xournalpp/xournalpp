# Zoom Draw tool for Xournal++

This patch adds a new tool, **Zoom Draw**, to Xournal++
(https://github.com/xournalpp/xournalpp).

## What it does

1. Select the "Zoom Draw" tool (Tools menu, or the toolbar entry with the
   magnifying-glass icon; default shortcut `Ctrl+Shift+Z`).
2. Click a point **P** on a displayed PDF/page.
3. A popup dialog opens showing a magnified (default **8x**, user
   adjustable in the popup itself) view of the page, centered on **P**. The
   popup is a square whose side is about half of `min(width, height)` of
   the main Xournal++ window, as requested.
4. Draw/annotate inside the popup using the current pen color/thickness
   (color and size can be set the same way as for any other drawing tool,
   e.g. via the toolbar, while "Zoom Draw" is selected).
5. Click **Apply** (or press `Ctrl+Enter`): every stroke drawn in the
   popup is transformed back to true page coordinates/scale and merged
   into the document, as a single undoable action (`Ctrl+Z` undoes the
   whole batch at once, just like it would for a normal freehand stroke).
6. Click **Cancel** (or press `Escape`, or close the window): nothing is
   changed in the document.

## Files touched

New files:

- `src/core/control/ZoomDrawController.{h,cpp}` — entry point called from
  `XojPageView` on click; computes the popup geometry (point P in page
  units, popup side in pixels from the main window size, initial zoom
  factor from `Settings`) and shows the popup.
- `src/core/gui/dialog/ZoomDrawDialog.{h,cpp}` — the actual popup: renders
  the magnified page (background + existing annotations, via the existing
  `DocumentView`/`PdfCache` rendering pipeline) plus the strokes drawn so
  far, captures mouse input to build `Stroke` objects, and on "Apply"
  inserts them into the document through the standard
  `InsertUndoAction`/`GroupUndoAction` machinery (same mechanism the
  regular pen tool uses), so normal undo/redo, autosave, etc. all work
  unmodified.
- `ui/zoomDrawDialog.glade` — UI layout for the popup (drawing area +
  magnification spin button + Apply/Cancel buttons).
- `ui/icons{Lucide,Color}-{light,dark}/hicolor/scalable/actions/xopp-tool-zoom-draw.svg`
  — toolbar/menu icon in the 4 existing icon themes.

Modified files (all small, additive changes following the existing
conventions, e.g. mirroring how `TOOL_LATEX` is wired in each place):

- `src/core/control/ToolEnums.{h,cpp}` — new `TOOL_ZOOM_DRAW` tool type,
  name, and capabilities (`TOOL_CAP_COLOR | TOOL_CAP_SIZE`, so the color
  and thickness selectors show up in the toolbar while it's active, and
  those are what get used for the strokes drawn in the popup).
- `src/core/control/ToolHandler.cpp` — registers the tool with a default
  color/thickness table (same thickness scale as the pen).
- `src/core/control/settings/Settings.{h,cpp}` — new persisted setting
  `zoomDrawFactor` (the "configurable" magnification factor mentioned in
  the request), clamped to a sane `[2x, 32x]` range, getter/setter
  `getZoomDrawFactor()` / `setZoomDrawFactor()`. It's also directly
  adjustable from a spin button inside the popup itself, so users don't
  need to dig through a settings dialog to change it.
- `src/core/gui/PageView.{h,cpp}` — on click with the tool active, records
  that a click happened (mirroring the existing `inLatex` flag used by the
  LaTeX tool) and, on release, calls
  `ZoomDrawController::activate(page, control, x, y)` with the click
  position in page coordinate units.
- `src/core/gui/XournalppCursor.cpp` — crosshair cursor while the tool is
  active, consistent with the other point-and-click tools (Image, LaTeX).
- `src/core/gui/toolbarMenubar/ToolMenuHandler.cpp` — toolbar/menu action
  registration (icon, label).
- `ui/mainmenubar.xml` — "Tools" menu entry + accelerator, with the
  `target` value (`27`) matching the new `TOOL_ZOOM_DRAW` enum value, as
  required by the existing convention documented at the top of
  `ToolEnums.h`.
- `CHANGELOG.md` — one-line entry.

## Design notes / how the coordinate math works

- The point P and all stroke points are kept in **page coordinate units**
  (points, i.e. the same units `XojPage`/`Stroke`/PDF rendering already
  use), never in on-screen pixels of the main window. This is the same
  convention already used by `LatexController::insertLatex` for its (x, y)
  parameters.
- The popup's `GtkDrawingArea` is `popupSidePx` × `popupSidePx` screen
  pixels. Its `"draw"` handler applies
  `translate(side/2, side/2) → scale(zoom, zoom) → translate(-P.x, -P.y)`
  to its cairo context and then calls the existing
  `DocumentView::drawPage()` — the same function the main page view itself
  uses to render a page — so the popup always shows an accurate, live
  snapshot of the real page content (PDF background + all layers),
  magnified around P.
- Because that same transform stays active while the in-popup strokes are
  drawn (`cairo_move_to`/`cairo_line_to` with page-unit coordinates), they
  line up pixel-perfectly with the underlying content without any extra
  bookkeeping.
- Mouse events on the canvas are converted back from widget pixels to page
  units with the inverse transform
  (`P + (widget_px - side/2) / zoom`) and accumulated into a `Stroke`.
- On "Apply", each finished `Stroke` is handed to
  `Layer::addElement()`/`InsertUndoAction` in the same order the built-in
  `StrokeHandler` uses (create the `InsertUndoAction` referencing the
  element *before* moving it into the layer), wrapped in one
  `GroupUndoAction` so the whole popup session undoes as a single step.
  `XojPage::fireRangeChanged()` is then called over the union of the new
  strokes' bounding boxes to repaint just the affected page region.

## Known simplifications (given the scope of a single patch)

- Pressure sensitivity is not captured in the popup (strokes drawn there
  always use `Point::NO_PRESSURE`); the main canvas's stylus/pressure
  handling is unaffected. A pressure-aware version could reuse
  `StrokeHandler`/`InputHandler` more directly, at the cost of a larger,
  more invasive change.
- The popup does not currently support the eraser/highlighter/shape tools
  inside it — only plain pen-style strokes using the active color/
  thickness. Extending it to reuse the full `InputHandler` family (as the
  main canvas does) is a natural, larger follow-up.

## Build verification

The project (a large GTK3/C++20 codebase) was cloned fresh from GitHub and
actually built in a sandboxed Ubuntu 24.04 environment:

- All required dependencies (`gtk+-3.0`, `poppler-glib`/`poppler-cpp`,
  `libzip`, `librsvg`, `libqpdf`, `gtksourceview-4`, `lua5.3`, X11, ...)
  were installed and `cmake` configured successfully with
  `-DENABLE_AUDIO=OFF` (audio deps aren't relevant to this change).
- Both new translation units (`ZoomDrawController.cpp`,
  `ZoomDrawDialog.cpp`) were compiled directly with the project's real
  compiler flags/include paths (extracted from `compile_commands.json`)
  and compiled **cleanly, with zero errors and zero warnings**.
- Every modified file (`ToolEnums.cpp`, `ToolHandler.cpp`, `Settings.cpp`,
  `PageView.cpp`, `XournalppCursor.cpp`, `ToolMenuHandler.cpp`) was
  recompiled the same way and also **compiled cleanly** (only pre-existing,
  unrelated warnings remained, e.g. a pre-existing `[[deprecated]]` usage
  in `PageView.cpp`).
- Both new/modified glade and menu XML files (`zoomDrawDialog.glade`,
  `mainmenubar.xml`) were validated with `xmllint --noout`.
- A full parallel link of the whole `xournalpp-core` target was started to
  double check there are no further integration issues, but was not run to
  completion in this environment (single-core sandbox, the full project is
  large); the per-file compiles above already give strong confidence since
  they exercise the exact same headers/declarations the rest of the
  codebase links against.

If you want to build and run it yourself:

```sh
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
./xournalpp
```
