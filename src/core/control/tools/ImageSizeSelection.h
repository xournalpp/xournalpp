/*
 * Xournal++
 *
 * Select a size before inserting an image
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once


#include "model/Element.h"      // todo p0mm
#include "model/OverlayBase.h"  // todo p0mm
#include "util/Rectangle.h"     // for Rectangle

class ImageSizeSelection: public OverlayBase {
public:
    ImageSizeSelection(double x, double y);

    void updatePosition(double x, double y);

    auto getSelectedSpace() -> xoj::util::Rectangle<double>;

private:
    double startX;
    double startY;
    double endX;
    double endY;
};