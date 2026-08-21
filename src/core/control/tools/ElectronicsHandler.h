#pragma once

#include "BaseShapeHandler.h"

class ElectronicsHandler : public BaseShapeHandler {
public:
    ElectronicsHandler(Control* control, const PageRef& page);
    ~ElectronicsHandler() override;

private:
    std::pair<std::vector<Point>, Range> createShape(bool isAltDown, bool isShiftDown, bool isControlDown) override;
};
