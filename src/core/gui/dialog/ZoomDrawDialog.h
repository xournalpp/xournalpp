/*
 * Xournal++
 *
 * Popup dialog for the "Zoom Draw" tool: shows a magnified view of the page
 * around a clicked point and lets the user draw on it. Confirming the dialog
 * transfers the drawn strokes back onto the real page at their true scale.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <vector>

#include <gtk/gtk.h>

#include "model/PageRef.h"       // for PageRef
#include "model/Point.h"         // for Point
#include "util/raii/GtkWindowUPtr.h"
#include "view/DocumentView.h"  // for DocumentView

class Control;
class GladeSearchpath;
class Stroke;

/**
 * @brief Popup window used by the ZoomDrawController / "Zoom Draw" tool.
 *
 * This class is meant to be used through xoj::popup::PopupWindowWrapper<ZoomDrawDialog>,
 * which owns and destroys the instance once the underlying GtkWindow is closed.
 */
class ZoomDrawDialog {
public:
    /**
     * @param gladeSearchPath Used to locate ui/zoomDrawDialog.glade
     * @param control The application's Control instance
     * @param page The page that was clicked on
     * @param centerX X coordinate (page units/pt) of the clicked point P
     * @param centerY Y coordinate (page units/pt) of the clicked point P
     * @param popupSidePx Side length, in screen pixels, of the square magnified canvas
     * @param initialZoomFactor Initial magnification factor (e.g. 8.0 for 8x)
     */
    ZoomDrawDialog(GladeSearchpath* gladeSearchPath, Control* control, PageRef page, double centerX, double centerY,
                   int popupSidePx, double initialZoomFactor);
    ~ZoomDrawDialog();

    ZoomDrawDialog(ZoomDrawDialog const&) = delete;
    ZoomDrawDialog& operator=(ZoomDrawDialog const&) = delete;

    /// Required by xoj::popup::PopupWindowWrapper
    GtkWindow* getWindow() const;

private:
    void redrawCanvas();

    /// Commits the drawn strokes to the document (if any) and closes the popup.
    void applyAndClose();

    /// Discards the drawn strokes and closes the popup, without touching the document.
    void cancelAndClose();

    /// Converts a point in popup-canvas pixel coordinates to page coordinates (pt).
    Point widgetToPage(double wx, double wy) const;

    void beginStroke(double wx, double wy);
    void continueStroke(double wx, double wy);
    void endStroke();

    /// Renders the magnified page + strokes into the canvas's cairo context.
    void drawContents(cairo_t* cr);

    Control* control;
    PageRef page;

    double centerX;
    double centerY;
    int popupSidePx;
    double zoomFactor;

    xoj::util::GtkWindowUPtr window;
    GtkWidget* canvas = nullptr;
    GtkSpinButton* zoomFactorSpin = nullptr;
    GtkButton* btOk = nullptr;
    GtkButton* btCancel = nullptr;

    DocumentView docView;

    /// Strokes that have already been finished (button released) while the popup is open.
    std::vector<std::unique_ptr<Stroke>> strokes;

    /// The stroke currently being drawn (while the mouse button is held down), if any.
    std::unique_ptr<Stroke> currentStroke;

    bool dragging = false;
};
