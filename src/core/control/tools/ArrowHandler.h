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

#include <vector>  // for vector

#include "model/PageRef.h"  // for PageRef

#include "BaseShapeHandler.h"  // for BaseShapeHandler

class Point;
class Control;

class ArrowHandler: public BaseShapeHandler {
public:
    ArrowHandler(Control* control, const PageRef& page, bool doubleEnded);
    ~ArrowHandler() override;

private:
    auto createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
            -> std::pair<std::shared_ptr<Path>, Range> override;
    bool doubleEnded = false;
};
