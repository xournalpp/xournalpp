/*
 * Xournal++
 *
 * Handles input to draw an arrow
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseStrokeHandler.h"

class ArrowHandler: public BaseStrokeHandler {
public:
    ArrowHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page);
    virtual ~ArrowHandler();

private:
    void drawShape(Point& currentPoint, const PositionInputData& pos,
                   const std::lock_guard<std::recursive_mutex>&) override;

private:
};
