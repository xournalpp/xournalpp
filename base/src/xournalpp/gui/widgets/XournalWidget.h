/*
 * Xournal++
 *
 * Xournal widget which is the "View" widget
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>

#include <gtk/gtk.h>
#include <lager/reader.hpp>
#include <xournalpp/settings/Settings.h>
#include <xournalpp/view/Viewport.h>

#include "gtkdrawingareascrollable.h"

class XournalWidget {
public:
    XournalWidget(const lager::reader<Settings>& settings, lager::reader<Viewport> viewport,
                  lager::context<ViewportAction> context);

    ~XournalWidget();

    auto getGtkWidget() -> GtkWidget*;

private:
    auto updateScrollbar(GtkAdjustment* adj, double value) -> void;

    static auto initHScrolling(XournalWidget* self) -> void;
    static auto initVScrolling(XournalWidget* self) -> void;

    static auto sizeAllocateCallback(GtkWidget* widget, GdkRectangle* allocation, XournalWidget* self) -> void;
    static auto realizeCallback(GtkWidget* widget, XournalWidget* self) -> void;
    static auto drawCallback(GtkWidget* widget, cairo_t* cr, XournalWidget* self) -> gboolean;

    static auto horizontalScroll(GtkAdjustment* hadjustment, XournalWidget* self) -> void;
    static auto verticalScroll(GtkAdjustment* vadjustment, XournalWidget* self) -> void;

private:
    GtkWidget* drawingArea;

    /** State */
    lager::reader<Viewport> viewport;
    lager::reader<double> scrollX;
    lager::reader<double> scrollY;
    lager::reader<double> rawScale;
    lager::reader<Settings::DocumentMode> docMode;
    lager::context<ViewportAction> context;

    /*
     * References to library state, document stuff etc.
     */


    constexpr static double STEP_INCREMENT = 10;
};
