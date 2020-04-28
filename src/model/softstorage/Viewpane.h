//
// Created by julius on 27.04.20.
//

#pragma once


#include <model/hardstorage/DurableData.h>

#include "model/Storage.h"

#include "EphemeralData.h"

class ViewpaneEvent {
public:
    virtual ~ViewpaneEvent() = default;
};

class ViewpaneChangeEvent: public ViewpaneEvent {
public:
    const int width;
    const int height;
    const int x;
    const int y;
};

class Viewpane: public Storage<ViewpaneEvent> {
public:
    Viewpane(std::shared_ptr<Selections> selectionStorage, std::shared_ptr<EphemeralData>, std::shared_ptr<DurableData>,
             std::shared_ptr<Viewport> viewport);

private:
    void onAction(const Action& action) override;
};
