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
class Renderer;

#include <memory>

#include <util/Rectangle.h>

class XournalWidget {
public:
    XournalWidget(std::shared_ptr<InputContext> inputContext, std::shared_ptr<Renderer> render);
    virtual ~XournalWidget();

    auto getGtkWidget() -> GtkWidget*; //TODO remove and encapsulate widget completely
    auto repaintArea(const Rectangle<int>& rect) -> void;
    auto getVisibleArea() -> Rectangle<int>;
    auto setVisibleArea(const Rectangle<int>& rect) -> void;
    auto scroll(int xDiff, int yDiff) -> void;
    auto zoom(int originX, int originY, double scale) -> void;
    auto queueRedraw() -> void;

private:
    auto init() -> void;

    static auto sizeAllocateCallback(GtkWidget* widget, GdkRectangle* allocation, XournalWidget* self) -> void;
    static auto realizeCallback(GtkWidget* widget, XournalWidget* self) -> void;
    static auto drawCallback(GtkWidget* widget, cairo_t* cr, XournalWidget* self) -> gboolean;

private:
    GtkWidget* drawingArea;

    /** Layout */
    std::shared_ptr<Renderer> renderer;

    /** New input handling */
    std::shared_ptr<InputContext> input;

    /**
     * Top left corner of the current visible rectangle
     */
    Rectangle<int> viewport;
    double scale = 0;

    static const int SIZE_EXTENSION = 50;
};
