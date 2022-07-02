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

#include "model/PageRef.h"  // for PageRef

#include "BaseShapeHandler.h"  // for BaseShapeHandler

class Point;
class PositionInputData;
class XojPageView;
class XournalView;

class RulerHandler: public BaseShapeHandler {
public:
    RulerHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page);
    ~RulerHandler() override;

private:
    void drawShape(Point& currentPoint, const PositionInputData& pos) override;

private:
};
