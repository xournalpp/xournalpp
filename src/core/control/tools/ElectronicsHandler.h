#pragma once

#include "BaseShapeHandler.h"
#include "model/PageRef.h"
#include "model/Point.h"
#include "util/Range.h"
#include <vector>

class Control;

class ElectronicsHandler : public BaseShapeHandler {
public:
    ElectronicsHandler(Control* control, const PageRef& page);
    ~ElectronicsHandler() override;

    void onButtonReleaseEvent(const PositionInputData& pos, double zoom) override;

protected:
    std::pair<std::vector<Point>, Range> createShape(bool isAltDown, bool isShiftDown, bool isControlDown) override;
};
