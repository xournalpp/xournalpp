/*
 * Xournal++
 *
 * A flying icon widget on the drawing area
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>

#include <gdk/gdk.h>  // for GdkEvent, GdkRectangle
#include <glib.h>     // for gboolean
#include <gtk/gtk.h>  // for GtkWidget, GtkOverlayicon

#include "util/Point.h"
#include "util/raii/GObjectSPtr.h"

class MainWindow;

class FlyingClickableIcon {
public:
    enum class Anchor { NORTH_EAST, NORTH, NORTH_WEST, EAST, CENTER, WEST, SOUTH_EAST, SOUTH, SOUTH_WEST };

    FlyingClickableIcon(MainWindow* theMainWindow, const char* iconName, Anchor a);
    virtual ~FlyingClickableIcon();

public:
    /// Move the icon at the provided coordinates (in widget-coordinates).
    void setPosition(xoj::util::Point<int> position);

    inline GtkWidget* getWidget() { return widget.get(); }
    /// Signals added this way will be disconnected upon destruction
    void addSignal(GObject* o, gulong sig);

private:
    /**
     * Callback for positioning overlayed floating menu
     */
    static bool getOverlayPosition(GtkOverlay* overlay, GtkWidget* widget, GdkRectangle* alloc,
                                   FlyingClickableIcon* self);

private:
    MainWindow* mainWindow;
    GtkOverlay* overlay;  //< Parent overlay
    xoj::util::raii::WidgetSPtr widget;

    xoj::util::Point<char> anchor = {};  // 0 = anchor WEST/NORTH, 1 = CENTER, 2 = anchor EAST/SOUTH

    /// Communicating with getOverlayPosition callback
    xoj::util::Point<int> position = {};

    std::vector<std::pair<GObject*, gulong>> signals;
};
