/*
 * Xournal++
 *
 * Enum for all action groups, e.g. All tools but also for all toggle actions a
 * group is used, and ACTION_NONE for not selected
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>

/******************************************************************************
*******************************************************************************

RUN THE GENERATOR IF YOU CHANGE THIS FILE!

php generateConvert.php

*******************************************************************************
******************************************************************************/

enum ActionGroup {
    GROUP_NOGROUP = 0,
    GROUP_TOOL = 1,
    GROUP_COLOR,
    GROUP_SIZE,
    GROUP_ERASER_MODE,
    GROUP_ERASER_SIZE,
    GROUP_PEN_SIZE,
    GROUP_PEN_FILL,
    GROUP_HIGHLIGHTER_SIZE,
    GROUP_HIGHLIGHTER_FILL,

    // Need group for toggle button, this is the first Toggle Group
    GROUP_TOGGLE_GROUP,

    GROUP_PAIRED_PAGES,
    GROUP_PRESENTATION_MODE,

    GROUP_FULLSCREEN,

    GROUP_RULER,

    GROUP_LINE_STYLE,

    GROUP_AUDIO,

    GROUP_SNAPPING,

    GROUP_GRID_SNAPPING,

    GROUP_HIGHLIGHT_POSITION,

    GROUP_GEOMETRY_TOOL,

    GROUP_FILL,

    GROUP_FIXED_ROW_OR_COLS,

    GROUP_LAYOUT_HORIZONTAL,

    GROUP_LAYOUT_LR,

    GROUP_LAYOUT_TB,

    GROUP_ZOOM_FIT,
};

ActionGroup ActionGroup_fromString(const std::string& value);
std::string ActionGroup_toString(ActionGroup value);
