//
// Created by julius on 26.04.20.
//

#pragma once

#include <functional>

#include <model/Storage.h>

#include "ViewportEvent.h"

class Viewport: public Storage<ViewportEvent> {
public:
    Viewport();

public:
    auto getX() -> double;
    auto getY() -> double;
    auto getWidth() -> unsigned int;
    auto getHeight() -> unsigned int;
    auto getRawScale() -> double;
    auto getScale() -> double;

    auto onAction(const Action& action) -> void override;

private:
    double x = 0;
    double y = 0;
    double rawScale = 1;
};