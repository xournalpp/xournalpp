/*
 * Xournal++
 *
 * Handles input to draw sinus functions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseShapeHandler.h"

class SinusHandler: public BaseShapeHandler {
public:
    SinusHandler(Control* control, const PageRef& page, bool flipShift = false, bool flipControl = false);
    virtual ~SinusHandler();

private:
    auto createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
            -> std::pair<std::vector<Point>, Range> override;
};
