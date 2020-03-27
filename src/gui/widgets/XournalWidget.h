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
class Rectangle;
class XournalView;
class InputContext;

#include <memory>

class XournalWidget {
public:
    XournalWidget(XournalView* view, std::shared_ptr<InputContext> inputContext);
    virtual ~XournalWidget();

    /** Current selection */
    EditSelection* selection;

    int x;
    int y;

    XournalView* const view;

    auto getGtkWidget() -> GtkWidget*;
    auto getLayout() -> Layout*;
    auto repaintArea(int x1, int y1, int x2, int y2) -> void;
    auto getVisibleArea(XojPageView* p) -> Rectangle*;

private:
    auto init() -> void;

    static auto sizeAllocateCallback(GtkWidget* widget, GdkRectangle* allocation, XournalWidget* self) -> void;
    static auto realizeCallback(GtkWidget* widget, XournalWidget* self) -> void;
    static auto drawCallback(GtkWidget* widget, cairo_t* cr, XournalWidget* self) -> gboolean;

    auto drawShadow(cairo_t* cr, int left, int top, int width, int height, bool selected) -> void;

private:
    GtkWidget* drawingArea;

    /** Layout */
    std::unique_ptr<Layout> layout;

    /** New input handling */
    std::shared_ptr<InputContext> input;
};
