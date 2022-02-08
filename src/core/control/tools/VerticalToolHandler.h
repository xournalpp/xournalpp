/*
 * Xournal++
 *
 * Vertical Space tool
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include <cairo.h>

#include "gui/Redrawable.h"
#include "model/PageRef.h"
#include "undo/MoveUndoAction.h"
#include "view/ElementContainer.h"

#include "SnapToGridInputHandler.h"


class VerticalToolHandler: public ElementContainer {
public:
    /**
     * @param initiallyReverse Set this to true if the user has the reverse mode
     * button (e.g., Ctrl) held down when a vertical selection is started.
     */
    VerticalToolHandler(Redrawable* view, const PageRef& page, Settings* settings, double y, bool initiallyReverse,
                        double zoom, GdkWindow* window);
    ~VerticalToolHandler() override;
    VerticalToolHandler(VerticalToolHandler&) = delete;
    VerticalToolHandler& operator=(VerticalToolHandler&) = delete;
    VerticalToolHandler(VerticalToolHandler&&) = delete;
    VerticalToolHandler&& operator=(VerticalToolHandler&&) = delete;

    void paint(cairo_t* cr, GdkRectangle* rect, double zoom);

    /** Update the tool state with the new spacing position */
    void currentPos(double x, double y);

    bool onKeyPressEvent(GdkEventKey* event);
    bool onKeyReleaseEvent(GdkEventKey* event);

    std::unique_ptr<MoveUndoAction> finalize();

    std::vector<Element*>* getElements() override;

private:
    enum class Side {
        /** elements above the reference line */
        Above = -1,
        /** elements below the reference line */
        Below = 1,
    };

    /**
     * Clear the currently moved elements, and then select all elements
     * above/below startY (depending on the side) to use for the spacing.
     * Lastly, redraw the elements to the buffer.
     */
    void adoptElements(Side side);

    /**
     * Recreate the buffer if the new zoom value is higher.
     */
    void updateZoom(double newZoom);

    /**
     * Clear the buffer and redraw the elements being spaced.
     */
    void redrawBuffer();

    GdkWindow* window;
    Redrawable* view = nullptr;
    PageRef page;
    Layer* layer = nullptr;
    std::vector<Element*> elements;

    /**
     * Image buffer containing a rendering of the elements being spaced. This
     * buffer is rendered below the endY if direction is below, or above the
     * endY if the direction is above.
     */
    cairo_surface_t* crBuffer = nullptr;

    double startY = 0;
    double endY = 0;

    /**
     * Indicates whether to move elements above (negative) or below (positive) the anchor line.
     */
    Side spacingSide;

    /**
     * Current zoom level.
     */
    double zoom;  // TODO: listen to zoom change in case zoom is changed during spacing

    /**
     * The handler for snapping points
     */
    SnapToGridInputHandler snappingHandler;
};
