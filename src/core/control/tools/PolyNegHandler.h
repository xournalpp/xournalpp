/*
 * Xournal++
 *
 * Handles input to draw sqrt(x) x^-1 x^-2 x^-3 functions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseShapeHandler.h"

class PolyNegHandler: public BaseShapeHandler {
public:
    PolyNegHandler(Control* control, const PageRef& page, bool flipShift = false, bool flipControl = false);
    virtual ~PolyNegHandler();

private:
    auto createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
            -> std::pair<std::vector<Point>, Range> override;
};
