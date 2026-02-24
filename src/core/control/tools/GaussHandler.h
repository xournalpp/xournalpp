/*
 * Xournal++
 *
 * Handles input to draw gaussian curve
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseShapeHandler.h"

class GaussHandler: public BaseShapeHandler {
public:
    GaussHandler(Control* control, const PageRef& page, bool flipShift = false, bool flipControl = false);
    virtual ~GaussHandler();

private:
    auto createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
            -> std::pair<std::vector<Point>, Range> override;
};
