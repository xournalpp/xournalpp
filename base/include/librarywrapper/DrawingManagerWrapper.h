//
// Created by julius on 01.05.20.
//

#pragma once

class DrawingManagerWrapper {
public:
    DrawingManagerWrapper(std::shared_ptr<DrawingManager> drawingManager);
    auto render(cairo_t* cr) -> void;
    auto onRedraw(std::function<void> callback) -> void;

private:
    std::shared_ptr<DrawingManager> drawingManager;
};