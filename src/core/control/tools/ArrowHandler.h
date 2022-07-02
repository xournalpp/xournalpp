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

#include "model/PageRef.h"  // for PageRef

#include "BaseShapeHandler.h"  // for BaseShapeHandler

class Point;
class PositionInputData;
class XojPageView;
class XournalView;

class ArrowHandler: public BaseShapeHandler {
public:
    ArrowHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool doubleEnded);
    ~ArrowHandler() override;

private:
    void drawShape(Point& currentPoint, const PositionInputData& pos) override;
    bool doubleEnded = false;

private:
};
