//
// Created by julius on 26.04.20.
//

#include "Viewport.h"

#include "control/Dispatcher.h"

Viewport::Viewport() {
    Dispatcher::getMainStage().registerListener(*this,
                                                [](auto action) { return typeid(action) == typeid(ViewportAction); });
}