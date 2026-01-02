/*
 * Xournal++
 *
 * Handles input to draw polynomial functions (x²,x³,x⁴,x⁵)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseShapeHandler.h"

class PolyHandler: public BaseShapeHandler {
public:
    PolyHandler(Control* control, const PageRef& page, bool flipShift = false, bool flipControl = false);
    virtual ~PolyHandler();

private:
    auto createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
            -> std::pair<std::vector<Point>, Range> override;
};
