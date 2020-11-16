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
#include "XournalType.h"

class VerticalToolHandler: public ElementContainer {
public:
    VerticalToolHandler(Redrawable* view, const PageRef& page, Settings* settings, double y, double zoom);
    ~VerticalToolHandler() override;

    void paint(cairo_t* cr, GdkRectangle* rect, double zoom);
    void currentPos(double x, double y);

    std::unique_ptr<MoveUndoAction> finalize();

    vector<Element*>* getElements() override;

private:
    Redrawable* view = nullptr;
    PageRef page;
    Layer* layer = nullptr;
    vector<Element*> elements;

    cairo_surface_t* crBuffer = nullptr;

    double startY = 0;
    double endY = 0;

    /**
     * When we create a new page
     */
    double jumpY = 0;

    /**
     * The handler for snapping points
     */
    SnapToGridInputHandler snappingHandler;
};
