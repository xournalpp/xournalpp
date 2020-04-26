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

#include <model/softstorage/LayoutEvent.h>
#include <util/Rectangle.h>

#include "gui/Renderer.h"
#include "model/softstorage/ViewportEvent.h"

#include "gtkdrawingareascrollable.h"

class XournalWidget: public ControllerView<ViewportEvent>, public ControllerView<LayoutEvent> {
public:
    XournalWidget(std::unique_ptr<Renderer> render, std::shared_ptr<Viewport> viewport, std::shared_ptr<Layout> layout);
    ~XournalWidget() override;

    auto eventCallback(const ViewportEvent& event) -> void override;
    auto eventCallback(const LayoutEvent& event) -> void override;

    auto getGtkWidget() -> GtkWidget*;

private:
    auto updateScrollbar(ScrollEvent::ScrollDirection direction, double value, bool infinite) -> void;

    static auto initHScrolling(XournalWidget* self) -> void;
    static auto initVScrolling(XournalWidget* self) -> void;

    static auto sizeAllocateCallback(GtkWidget* widget, GdkRectangle* allocation, XournalWidget* self) -> void;
    static auto realizeCallback(GtkWidget* widget, XournalWidget* self) -> void;
    static auto drawCallback(GtkWidget* widget, cairo_t* cr, XournalWidget* self) -> gboolean;

    static auto horizontalScroll(GtkAdjustment* hadjustment, XournalWidget* self) -> void;
    static auto verticalScroll(GtkAdjustment* vadjustment, XournalWidget* self) -> void;

private:
    GtkWidget* drawingArea;

    /** Renderer */
    std::unique_ptr<Renderer> renderer;

    /** Viewport storage (state) */
    std::shared_ptr<Viewport> viewport;

    /** Layout storage (state) */
    std::shared_ptr<Layout> layout;

    constexpr static double STEP_INCREMENT = 10;
};
