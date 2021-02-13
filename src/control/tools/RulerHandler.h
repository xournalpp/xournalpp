/*
 * Xournal++
 *
 * Handles input of the ruler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "PLShapeHandler.h"

class RulerHandler: public PLShapeHandler {
public:
    RulerHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page);
    virtual ~RulerHandler();

private:
    virtual void drawShape(Point& currentPoint, const PositionInputData& pos) override;

private:
};
