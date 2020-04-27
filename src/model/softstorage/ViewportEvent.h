//
// Created by julius on 26.04.20.
//

#pragma once


#include <model/StorageEvent.h>
class ViewportEvent: public StorageEvent {
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
