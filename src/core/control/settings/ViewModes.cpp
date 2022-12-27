#include "ViewModes.h"
#include "util/StringUtils.h"

#include <iostream>

struct ViewMode settingsStringToViewMode(std::string viewModeString) {
    struct ViewMode viewMode;
    for (const std::string& attr : StringUtils::split(viewModeString, ',')) {
        if (attr.compare(ATTR_GO_FULLSCREEN) == 0) {
            viewMode.goFullscreen = true;
        } else if (attr.compare(ATTR_SHOW_MENUBAR) == 0) {
            viewMode.showMenubar = true;
        } else if (attr.compare(ATTR_SHOW_TOOLBAR) == 0) {
            viewMode.showToolbar = true;
        } else if (attr.compare(ATTR_SHOW_SIDEBAR) == 0) {
            viewMode.showSidebar = true;
        }
    }
    return viewMode;
}

const std::string viewModeToSettingsString(struct ViewMode viewMode) {
    if (!(viewMode.goFullscreen || viewMode.showMenubar || viewMode.showToolbar || viewMode.showSidebar)) {
        return "";
    }
    return ((viewMode.goFullscreen ? "," + static_cast<std::string>(ATTR_GO_FULLSCREEN) : "")
        + (viewMode.showMenubar ? "," + static_cast<std::string>(ATTR_SHOW_MENUBAR) : "")
        + (viewMode.showToolbar ? "," + static_cast<std::string>(ATTR_SHOW_TOOLBAR) : "")
        + (viewMode.showSidebar ? "," + static_cast<std::string>(ATTR_SHOW_SIDEBAR) : "")).erase(0,1);
}
