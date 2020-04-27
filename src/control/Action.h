//
// Created by julius on 25.04.20.
//

#pragma once

class Action {
public:
    Action();
    virtual ~Action();
};

class ViewportAction: public Action {};

class Scale: public ViewportAction {
public:
    Scale(double rawScale);
};

class Allocation: public ViewportAction {
public:
    Allocation(int width, int height);
};

class Scroll: public ViewportAction {
public:
    Scroll(double difference);
    virtual ~Scroll();

    const double difference;
};

class HScroll: public Scroll {
public:
    HScroll(double difference);
};
class VScroll: public Scroll {
public:
    VScroll(double difference);
};