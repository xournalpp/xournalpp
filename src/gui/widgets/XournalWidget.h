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

#include <gtk/gtk.h>

class EditSelection;
class Layout;
class XojPageView;
class XournalView;
class InputContext;

#include <memory>

#include <util/Rectangle.h>

#include "gui/Renderer.h"

#include "gtkdrawingareascrollable.h"

class XournalWidget {
public:
    XournalWidget(std::shared_ptr<InputContext> inputContext, std::shared_ptr<Renderer> render);
    virtual ~XournalWidget();

    auto getGtkWidget() -> GtkWidget*;  // TODO remove and encapsulate widget completely

    auto getViewport() -> Rectangle<double>;
    auto repaintViewport(const Rectangle<double>& rect) -> void;

    auto repaintVisibleArea(const Rectangle<double>& rect) -> void;
    auto getVisibleArea() -> Rectangle<double>;
    auto setVisibleArea(const Rectangle<double>& rect) -> void;

    auto setScale(double scale) -> void;

    /**
     * Queues a widget redraw.
     *
     * Call this after you updated the model.
     */
    auto queueRedraw() -> void;

    /**
     * Queues a widget allocate.
     *
     * Call this after you changed the layout of the model. (Additional page, different page layout)
     */
    auto queueAllocate() -> void;

private:
    auto init() -> void;
    auto updateScrollbar(GtkAdjustment* adj, double value, bool infinite) -> void;

    static auto initHScrolling(XournalWidget* self) -> void;
    static auto initVScrolling(XournalWidget* self) -> void;

    static auto sizeAllocateCallback(GtkWidget* widget, GdkRectangle* allocation, XournalWidget* self) -> void;
    static auto realizeCallback(GtkWidget* widget, XournalWidget* self) -> void;
    static auto drawCallback(GtkWidget* widget, cairo_t* cr, XournalWidget* self) -> gboolean;

    static auto horizontalScroll(GtkAdjustment* hadjustment, XournalWidget* self) -> void;
    static auto verticalScroll(GtkAdjustment* vadjustment, XournalWidget* self) -> void;

private:
    GtkWidget* drawingArea;

    /** Layout */
    std::shared_ptr<Renderer> renderer;

    /** New input handling */
    std::shared_ptr<InputContext> input;

    /**
     * Top left corner of the current visible rectangle
     */
    double x = 0;
    double y = 0;
    double scale = 0;

    constexpr static double STEP_INCREMENT = 10;
};
