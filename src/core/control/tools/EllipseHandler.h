/*
 * Xournal++
 *
 * Handles input to draw ellipses
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>  // for vector

#include "model/PageRef.h"  // for PageRef

#include "BaseShapeHandler.h"  // for BaseShapeHandler

class Control;

class EllipseHandler: public BaseShapeHandler {
public:
    EllipseHandler(Control* control, const PageRef& page, bool flipShift = false, bool flipControl = false);
    ~EllipseHandler() override;

private:
    auto createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
            -> std::pair<std::shared_ptr<Path>, Range> override;
};
