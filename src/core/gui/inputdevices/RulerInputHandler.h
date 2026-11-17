#pragma once

#include "GeometryToolInputHandler.h"

class RulerInputHandler: public GeometryToolInputHandler {
public:
    RulerInputHandler(XournalView* xournalView, GeometryToolController* controller);
    ~RulerInputHandler() noexcept override;

private:
    bool handlePointer(InputEvent const& event) override;
    double getMinHeight() const override;
    double getMaxHeight() const override;
};
