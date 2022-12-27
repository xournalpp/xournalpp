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

// reserved default view mode ids
constexpr auto VIEW_MODE_DEFAULT = 0;
constexpr auto VIEW_MODE_FULLSCREEN = 1;
constexpr auto VIEW_MODE_PRESENTATION = 2;

// view mode attributes
constexpr auto ATTR_GO_FULLSCREEN = "goFullscren";
constexpr auto ATTR_SHOW_MENUBAR  = "showMenubar";
constexpr auto ATTR_SHOW_TOOLBAR  = "showToolbar";
constexpr auto ATTR_SHOW_SIDEBAR  = "showSidebar";

struct ViewMode {
    std::string name{""};
    // view mode attribues
    bool goFullscreen{false};
    bool showMenubar{false};
    bool showToolbar{false};
    bool showSidebar{false};
};

struct ViewMode settingsStringToViewMode(std::string viewModeString);
const std::string viewModeToSettingsString(struct ViewMode viewMode);
