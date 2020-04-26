//
// Created by julius on 26.04.20.
//

#pragma once


class Viewport: Storage {
public:
    Viewport();

public:
    auto onScaleUpdate(std::function<void(double)> callback) -> void;

private:
    double x = 0;
    double y = 0;
    double rawScale = 1;
};
