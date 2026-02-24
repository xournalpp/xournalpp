/*
 * Xournal++
 *
 * Handles input to draw exp and ln functions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseShapeHandler.h"

class ExpHandler: public BaseShapeHandler {
public:
    ExpHandler(Control* control, const PageRef& page, bool flipShift = false, bool flipControl = false);
    virtual ~ExpHandler();

private:
    auto createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
            -> std::pair<std::vector<Point>, Range> override;
};
