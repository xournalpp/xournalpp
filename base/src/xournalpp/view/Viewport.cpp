//
// Created by julius on 26.04.20.
//

#include "xournalpp/view/Viewport.h"

auto viewportUpdate(Viewport model, ViewportAction action) -> ViewportResult {
    if (auto scroll = std::get_if<Scroll>(&action)) {
        if (scroll->direction == Scroll::HORIZONTAL)
            model.x = scroll->newVal;
        else
            model.y = scroll->newVal;
    }
    return {model, lager::noop};
}
