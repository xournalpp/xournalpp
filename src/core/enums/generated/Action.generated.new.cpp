// ** THIS FILE IS GENERATED **
// ** use generateConvertNEW.php to update this file **


#include <string_view>  // for string_view

#include <glib.h>  // for g_warning

#include "../Action.enum.h"
#include "util/Assert.h"  // for xoj_assert

// ** This needs to be copied to the header
Action Action_fromString(const std::string_view value);
const char* Action_toString(Action value);


constexpr const char* NAMES[] = {  // Action to string conversion map
        "new-file",
        "open",
        "annotate-pdf",
        "save",
        "save-as",
        "export-as-pdf",
        "export-as",
        "print",
        "quit",
        "arrange-selection-order",
        "undo",
        "redo",
        "cut",
        "copy",
        "paste",
        "search",
        "select-all",
        "delete",
        "move-selection-layer-up",
        "move-selection-layer-down",
        "rotation-snapping",
        "grid-snapping",
        "settings",
        "paired-pages-mode",
        "presentation-mode",
        "fullscreen",
        "show-sidebar",
        "show-toolbar",
        "set-layout-vertical",
        "set-layout-right-to-left",
        "set-layout-bottom-to-top",
        "set-columns-or-rows",
        "manage-toolbar",
        "customize-toolbar",
        "hide-menubar",
        "zoom-in",
        "zoom-out",
        "zoom-100",
        "zoom-fit",
        "goto-first",
        "goto-previous",
        "goto-page",
        "goto-next",
        "goto-last",
        "goto-next-annotated-page",
        "goto-previous-annotated-page",
        "new-page-before",
        "new-page-after",
        "new-page-at-end",
        "duplicate-page",
        "append-new-pdf-pages",
        "configure-page-template",
        "delete-page",
        "paper-format",
        "paper-background-color",
        "select-tool",
        "select-default-tool",
        "tool-draw-shape-recognizer",
        "tool-draw-rectangle",
        "tool-draw-ellipse",
        "tool-draw-arrow",
        "tool-draw-double-arrow",
        "tool-draw-coordinate-system",
        "tool-draw-line",
        "tool-draw-spline",
        "setsquare",
        "compass",
        "tool-pen-size",
        "tool-pen-line-style",
        "tool-pen-fill",
        "tool-pen-fill-opacity",
        "tool-eraser-size",
        "tool-eraser-type",
        "tool-highlighter-size",
        "tool-highlighter-fill",
        "tool-highlighter-fill-opacity",
        "audio-record",
        "audio-pause-playback",
        "audio-stop-playback",
        "audio-seek-forwards",
        "audio-seek-backwards",
        "select-font",
        "tex",
        "plugin-manager",
        "help",
        "about",
        "tool-size",
        "tool-fill",
        "tool-fill-opacity",
        "layer-new",
        "layer-delete",
        "layer-merge-down",
        "layer-rename",
        "layer-goto-next",
        "layer-goto-previous",
        "layer-goto-top"};

auto Action_toString(Action value) -> const char* {
    xoj_assert(value <= Action::_MAX_VALUE);
    return NAMES[static_cast<size_t>(value)];
}

auto Action_fromString(const std::string_view value) -> Action {
    for (size_t n = 0; n <= static_cast<size_t>(Action::_MAX_VALUE); n++) {
        if (value == NAMES[n]) {
            return static_cast<Action>(n);
        }
    }
    g_warning("Invalid enum value for Action: \"%s\"", value.data());
    return Action::NEW_FILE;
}
