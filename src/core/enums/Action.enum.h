/*
 * Xournal++
 *
 * Enum for all actions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>

#include <glib.h>  // for g_warning

#include "util/Assert.h"
#include "util/StringUtils.h"
#include "util/safe_casts.h"  // for to_underlying Todo(cpp20) remove

/******************************************************************************
*******************************************************************************

RUN THE GENERATOR IF YOU CHANGE THIS FILE!

php generateConvertNEW.php

*******************************************************************************
******************************************************************************/

enum class Action : size_t {
    // Menu file
    NEW_FILE,
    OPEN,
    ANNOTATE_PDF,
    SAVE,
    SAVE_AS,
    EXPORT_AS_PDF,
    EXPORT_AS,
    PRINT,
    QUIT,

    // Menu edit
    ARRANGE_SELECTION_ORDER,
    UNDO,
    REDO,
    CUT,
    COPY,
    PASTE,
    SEARCH,
    SELECT_ALL,
    DELETE,
    MOVE_SELECTION_LAYER_UP,
    MOVE_SELECTION_LAYER_DOWN,
    ROTATION_SNAPPING,
    GRID_SNAPPING,
    PREFERENCES,

    // Menu View
    PAIRED_PAGES_MODE,
    PAIRED_PAGES_OFFSET,
    PRESENTATION_MODE,
    FULLSCREEN,
    SHOW_SIDEBAR,
    SHOW_TOOLBAR,
    SET_LAYOUT_VERTICAL,
    SET_LAYOUT_RIGHT_TO_LEFT,
    SET_LAYOUT_BOTTOM_TO_TOP,
    SET_COLUMNS_OR_ROWS,
    MANAGE_TOOLBAR,
    CUSTOMIZE_TOOLBAR,
    SHOW_MENUBAR,
    ZOOM_IN,
    ZOOM_OUT,
    ZOOM_100,
    ZOOM_FIT,
    ZOOM,  ///< Action whose state is the current zoom value

    // Menu navigation
    GOTO_FIRST,
    GOTO_PREVIOUS,
    GOTO_PAGE,
    GOTO_NEXT,
    GOTO_LAST,
    GOTO_NEXT_ANNOTATED_PAGE,
    GOTO_PREVIOUS_ANNOTATED_PAGE,

    // Menu Journal
    NEW_PAGE_BEFORE,
    NEW_PAGE_AFTER,
    NEW_PAGE_AT_END,
    DUPLICATE_PAGE,
    MOVE_PAGE_TOWARDS_BEGINNING,
    MOVE_PAGE_TOWARDS_END,
    APPEND_NEW_PDF_PAGES,
    CONFIGURE_PAGE_TEMPLATE,
    DELETE_PAGE,
    PAPER_FORMAT,
    PAPER_BACKGROUND_COLOR,

    // Menu Tools
    SELECT_TOOL,
    SELECT_DEFAULT_TOOL,
    TOOL_DRAW_SHAPE_RECOGNIZER,
    TOOL_DRAW_RECTANGLE,
    TOOL_DRAW_ELLIPSE,
    TOOL_DRAW_ARROW,
    TOOL_DRAW_DOUBLE_ARROW,
    TOOL_DRAW_COORDINATE_SYSTEM,
    TOOL_DRAW_LINE,
    TOOL_DRAW_SPLINE,
    SETSQUARE,
    COMPASS,

    TOOL_PEN_SIZE,
    TOOL_PEN_LINE_STYLE,
    TOOL_PEN_FILL,
    TOOL_PEN_FILL_OPACITY,

    TOOL_ERASER_SIZE,
    TOOL_ERASER_TYPE,

    TOOL_HIGHLIGHTER_SIZE,
    TOOL_HIGHLIGHTER_FILL,
    TOOL_HIGHLIGHTER_FILL_OPACITY,

    TOOL_SELECT_PDF_TEXT_MARKER_OPACITY,

    AUDIO_RECORD,
    AUDIO_PAUSE_PLAYBACK,
    AUDIO_STOP_PLAYBACK,
    AUDIO_SEEK_FORWARDS,
    AUDIO_SEEK_BACKWARDS,

    SELECT_FONT,
    FONT,  ///< Action whose state is the font's description
    TEX,

    // Plugin Menu
    PLUGIN_MANAGER,

    // Menu Help
    HELP,
    DEMO,
    ABOUT,

    // Generic tool config, for the toolbar
    TOOL_SIZE,
    TOOL_FILL,
    TOOL_FILL_OPACITY,
    TOOL_COLOR,    ///< Action whose state is the current color with alpha set to 0xff (in ARGB as a uint32_t)
    SELECT_COLOR,  ///< Pops up a color chooser dialog

    // Layer handling
    LAYER_SHOW_ALL,
    LAYER_HIDE_ALL,
    LAYER_NEW,
    LAYER_COPY,
    LAYER_MOVE_UP,
    LAYER_MOVE_DOWN,
    LAYER_DELETE,
    LAYER_MERGE_DOWN,
    LAYER_RENAME,
    LAYER_GOTO_NEXT,
    LAYER_GOTO_PREVIOUS,
    LAYER_GOTO_TOP,
    LAYER_ACTIVE,  ///< Action whose state is the current layer index

    // Miscellaneous
    POSITION_HIGHLIGHTING,

    // Keep this last value
    ENUMERATOR_COUNT
};

#include "generated/Action.NameMap.generated.h"

constexpr auto Action_toString(Action value) -> const char* {
    xoj_assert(value < Action::ENUMERATOR_COUNT);
    return ACTION_NAMES[static_cast<size_t>(value)];
}

constexpr auto Action_fromString(const std::string_view value) -> Action {
    for (size_t n = 0; n < xoj::to_underlying(Action::ENUMERATOR_COUNT); n++) {
        if (value == ACTION_NAMES[n]) {
            return static_cast<Action>(n);
        }
    }
    return Action::NEW_FILE;
}
