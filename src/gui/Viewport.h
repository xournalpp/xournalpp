//
// Created by julius on 26.04.20.
//

#pragma once

#include <functional>

#include <model/Storage.h>

#include "util/Rectangle.h"

#include "ViewportEvent.h"

class Viewport: public Storage<ViewportEvent> {
public:
    Viewport();

public:
    auto getX() -> double;
    auto getY() -> double;
    auto getWidth() -> int;
    auto getHeight() -> int;
    auto getRawScale() -> double;
    auto getScale() -> double;
    auto getVisibleRect() -> Rectangle<double>;

    auto onAction(const Action& action) -> void override;

private:
    double x = 0;
    double y = 0;
    double rawScale = 1;
};