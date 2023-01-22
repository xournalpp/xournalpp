/*
 * Xournal++
 *
 * View Mode struct & attribute definitions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>           // for string
#include <vector>           // for vector
#include <unordered_map>    // for unordered_map

using ViewModeId = size_t;

// reserved default view mode ids
enum PresetViewModeIds {
    VIEW_MODE_DEFAULT = 0,
    VIEW_MODE_FULLSCREEN = 1,
    VIEW_MODE_PRESENTATION = 2
};

// view mode attributes
constexpr auto ATTR_GO_FULLSCREEN = "goFullscren";
constexpr auto ATTR_SHOW_MENUBAR  = "showMenubar";
constexpr auto ATTR_SHOW_TOOLBAR  = "showToolbar";
constexpr auto ATTR_SHOW_SIDEBAR  = "showSidebar";

struct ViewMode {
    bool goFullscreen{false};
    bool showMenubar{false};
    bool showToolbar{false};
    bool showSidebar{false};
};

// default ViewModes
constexpr ViewMode VIEW_MODE_STRUCT_DEFAULT{false, true, true, true};
constexpr ViewMode VIEW_MODE_STRUCT_FULLSCREEN{true, false, true, true};
constexpr ViewMode VIEW_MODE_STRUCT_PRESENTATION{true, false, true, false};

struct ViewMode settingsStringToViewMode(std::string viewModeString);
const std::string viewModeToSettingsString(struct ViewMode viewMode);
