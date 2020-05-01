//
// Created by julius on 26.04.20.
//

#pragma once

struct Viewport {
    int width;
    int height;
    double x;
    double y;
    double rawScale;
};

// Actions
struct Scroll {
    enum Direction { HORIZONTAL, VERTICAL };
    Direction direction;
    double difference;
};

struct Scale {
    double rawScale;
};

struct Resize {
    int width;
    int height;
};