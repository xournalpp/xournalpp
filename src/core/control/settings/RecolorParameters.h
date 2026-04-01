#pragma once

#include "util/Recolor.h"

struct RecolorParameters {
    bool recolorizeMainView = false;
    bool recolorizeSidebarMiniatures = false;
    Recolor recolor;

    bool operator==(const RecolorParameters& other) const {
        return recolorizeMainView == other.recolorizeMainView &&
               recolorizeSidebarMiniatures == other.recolorizeSidebarMiniatures && recolor == other.recolor;
    }

    bool operator!=(const RecolorParameters& other) const { return !(*this == other); }
};
