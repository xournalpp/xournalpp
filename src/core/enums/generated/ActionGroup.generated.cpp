// ** THIS FILE IS GENERATED **
// ** use generateConvert.php to update this file **


#include <string>

#include "../ActionGroup.enum.h"

using std::string;
#include <glib.h>


// ** This needs to be copied to the header
ActionGroup ActionGroup_fromString(const string& value);
string ActionGroup_toString(ActionGroup value);


auto ActionGroup_fromString(const string& value) -> ActionGroup {
    if (value == "GROUP_NOGROUP") {
        return GROUP_NOGROUP;
    }

    if (value == "GROUP_TOOL") {
        return GROUP_TOOL;
    }

    if (value == "GROUP_COLOR") {
        return GROUP_COLOR;
    }

    if (value == "GROUP_SIZE") {
        return GROUP_SIZE;
    }

    if (value == "GROUP_ERASER_MODE") {
        return GROUP_ERASER_MODE;
    }

    if (value == "GROUP_ERASER_SIZE") {
        return GROUP_ERASER_SIZE;
    }

    if (value == "GROUP_PEN_SIZE") {
        return GROUP_PEN_SIZE;
    }

    if (value == "GROUP_PEN_FILL") {
        return GROUP_PEN_FILL;
    }

    if (value == "GROUP_HIGHLIGHTER_SIZE") {
        return GROUP_HIGHLIGHTER_SIZE;
    }

    if (value == "GROUP_HIGHLIGHTER_FILL") {
        return GROUP_HIGHLIGHTER_FILL;
    }

    if (value == "GROUP_TOGGLE_GROUP") {
        return GROUP_TOGGLE_GROUP;
    }

    if (value == "GROUP_PAIRED_PAGES") {
        return GROUP_PAIRED_PAGES;
    }

    if (value == "GROUP_PRESENTATION_MODE") {
        return GROUP_PRESENTATION_MODE;
    }

    if (value == "GROUP_FULLSCREEN") {
        return GROUP_FULLSCREEN;
    }

    if (value == "GROUP_RULER") {
        return GROUP_RULER;
    }

    if (value == "GROUP_LINE_STYLE") {
        return GROUP_LINE_STYLE;
    }

    if (value == "GROUP_AUDIO") {
        return GROUP_AUDIO;
    }

    if (value == "GROUP_SNAPPING") {
        return GROUP_SNAPPING;
    }

    if (value == "GROUP_GRID_SNAPPING") {
        return GROUP_GRID_SNAPPING;
    }

    if (value == "GROUP_HIGHLIGHT_POSITION") {
        return GROUP_HIGHLIGHT_POSITION;
    }

    if (value == "GROUP_SETSQUARE") {
        return GROUP_SETSQUARE;
    }

    if (value == "GROUP_FILL") {
        return GROUP_FILL;
    }

    if (value == "GROUP_FIXED_ROW_OR_COLS") {
        return GROUP_FIXED_ROW_OR_COLS;
    }

    if (value == "GROUP_LAYOUT_HORIZONTAL") {
        return GROUP_LAYOUT_HORIZONTAL;
    }

    if (value == "GROUP_LAYOUT_LR") {
        return GROUP_LAYOUT_LR;
    }

    if (value == "GROUP_LAYOUT_TB") {
        return GROUP_LAYOUT_TB;
    }

    if (value == "GROUP_ZOOM_FIT") {
        return GROUP_ZOOM_FIT;
    }

    g_warning("Invalid enum value for ActionGroup: \"%s\"", value.c_str());
    return GROUP_NOGROUP;
}


auto ActionGroup_toString(ActionGroup value) -> string {
    if (value == GROUP_NOGROUP) {
        return "GROUP_NOGROUP";
    }

    if (value == GROUP_TOOL) {
        return "GROUP_TOOL";
    }

    if (value == GROUP_COLOR) {
        return "GROUP_COLOR";
    }

    if (value == GROUP_SIZE) {
        return "GROUP_SIZE";
    }

    if (value == GROUP_ERASER_MODE) {
        return "GROUP_ERASER_MODE";
    }

    if (value == GROUP_ERASER_SIZE) {
        return "GROUP_ERASER_SIZE";
    }

    if (value == GROUP_PEN_SIZE) {
        return "GROUP_PEN_SIZE";
    }

    if (value == GROUP_PEN_FILL) {
        return "GROUP_PEN_FILL";
    }

    if (value == GROUP_HIGHLIGHTER_SIZE) {
        return "GROUP_HIGHLIGHTER_SIZE";
    }

    if (value == GROUP_HIGHLIGHTER_FILL) {
        return "GROUP_HIGHLIGHTER_FILL";
    }

    if (value == GROUP_TOGGLE_GROUP) {
        return "GROUP_TOGGLE_GROUP";
    }

    if (value == GROUP_PAIRED_PAGES) {
        return "GROUP_PAIRED_PAGES";
    }

    if (value == GROUP_PRESENTATION_MODE) {
        return "GROUP_PRESENTATION_MODE";
    }

    if (value == GROUP_FULLSCREEN) {
        return "GROUP_FULLSCREEN";
    }

    if (value == GROUP_RULER) {
        return "GROUP_RULER";
    }

    if (value == GROUP_LINE_STYLE) {
        return "GROUP_LINE_STYLE";
    }

    if (value == GROUP_AUDIO) {
        return "GROUP_AUDIO";
    }

    if (value == GROUP_SNAPPING) {
        return "GROUP_SNAPPING";
    }

    if (value == GROUP_GRID_SNAPPING) {
        return "GROUP_GRID_SNAPPING";
    }

    if (value == GROUP_HIGHLIGHT_POSITION) {
        return "GROUP_HIGHLIGHT_POSITION";
    }

    if (value == GROUP_SETSQUARE) {
        return "GROUP_SETSQUARE";
    }

    if (value == GROUP_FILL) {
        return "GROUP_FILL";
    }

    if (value == GROUP_FIXED_ROW_OR_COLS) {
        return "GROUP_FIXED_ROW_OR_COLS";
    }

    if (value == GROUP_LAYOUT_HORIZONTAL) {
        return "GROUP_LAYOUT_HORIZONTAL";
    }

    if (value == GROUP_LAYOUT_LR) {
        return "GROUP_LAYOUT_LR";
    }

    if (value == GROUP_LAYOUT_TB) {
        return "GROUP_LAYOUT_TB";
    }

    if (value == GROUP_ZOOM_FIT) {
        return "GROUP_ZOOM_FIT";
    }

    g_error("Invalid enum value for ActionGroup: %i", value);
    return "";
}
