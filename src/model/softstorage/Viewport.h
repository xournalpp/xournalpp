//
// Created by julius on 26.04.20.
//

#pragma once

#include <functional>

#include <model/Storage.h>

#include "util/Rectangle.h"

class ViewportEvent {
public:
    virtual ~ViewportEvent();
};
class ScrollEvent: public ViewportEvent {
public:
    enum ScrollDirection { HORIZONTAL, VERTICAL };

public:
    ScrollEvent(ScrollDirection direction, double difference);

public:
    auto getDirection() -> ScrollDirection;
    auto getDifference() -> double;
};

class ScaleEvent: public ViewportEvent {
public:
    ScaleEvent(double rawScale);

public:
    auto getRawScale() -> double;
};

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