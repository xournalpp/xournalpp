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

#include "BaseStrokeHandler.h"

class RulerHandler: public BaseStrokeHandler {
public:
    RulerHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page);
    virtual ~RulerHandler();

private:
    std::vector<Point> createShape(const PositionInputData& pos) override;

private:
};
