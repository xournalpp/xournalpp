#include "ZoomDrawController.h"

#include <algorithm>  // for min, max

#include <gtk/gtk.h>

#include "control/Control.h"
#include "control/settings/Settings.h"
#include "gui/GladeSearchpath.h"
#include "gui/dialog/ZoomDrawDialog.h"
#include "model/XojPage.h"
#include "util/PopupWindowWrapper.h"

void ZoomDrawController::activate(const PageRef& page, Control* control, double x, double y) {
    if (!page || !control) {
        return;
    }

    GtkWindow* mainWindow = control->getGtkWindow();

    // "about half the size of the min(width, height) of original window W"
    int winWidth = gtk_widget_get_allocated_width(GTK_WIDGET(mainWindow));
    int winHeight = gtk_widget_get_allocated_height(GTK_WIDGET(mainWindow));
    int minSide = std::min(winWidth, winHeight);
    if (minSide <= 0) {
        // The main window might not be realized/mapped yet (e.g. during automated testing);
        // fall back to a sensible default so the tool still works.
        minSide = 800;
    }
    // Keep the popup within a usable range regardless of how the main window is sized.
    int popupSidePx = std::clamp(minSide / 2, 250, 1200);

    double zoomFactor = control->getSettings()->getZoomDrawFactor();

    xoj::popup::PopupWindowWrapper<ZoomDrawDialog> popup(control->getGladeSearchPath(), control, page, x, y,
                                                          popupSidePx, zoomFactor);
    popup.show(mainWindow);
}
