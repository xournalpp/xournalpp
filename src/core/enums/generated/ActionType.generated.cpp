// ** THIS FILE IS GENERATED **
// ** use generateConvert.php to update this file **


#include <string>

#include "../ActionType.enum.h"

using std::string;
#include <glib.h>


// ** This needs to be copied to the header
ActionType ActionType_fromString(const string& value);
string ActionType_toString(ActionType value);


auto ActionType_fromString(const string& value) -> ActionType {
    if (value == "ACTION_NONE") {
        return ACTION_NONE;
    }

    if (value == "ACTION_NEW") {
        return ACTION_NEW;
    }

    if (value == "ACTION_OPEN") {
        return ACTION_OPEN;
    }

    if (value == "ACTION_ANNOTATE_PDF") {
        return ACTION_ANNOTATE_PDF;
    }

    if (value == "ACTION_SAVE") {
        return ACTION_SAVE;
    }

    if (value == "ACTION_SAVE_AS") {
        return ACTION_SAVE_AS;
    }

    if (value == "ACTION_EXPORT_AS_PDF") {
        return ACTION_EXPORT_AS_PDF;
    }

    if (value == "ACTION_EXPORT_AS") {
        return ACTION_EXPORT_AS;
    }

    if (value == "ACTION_PRINT") {
        return ACTION_PRINT;
    }

    if (value == "ACTION_QUIT") {
        return ACTION_QUIT;
    }

    if (value == "ACTION_UNDO") {
        return ACTION_UNDO;
    }

    if (value == "ACTION_REDO") {
        return ACTION_REDO;
    }

    if (value == "ACTION_CUT") {
        return ACTION_CUT;
    }

    if (value == "ACTION_COPY") {
        return ACTION_COPY;
    }

    if (value == "ACTION_PASTE") {
        return ACTION_PASTE;
    }

    if (value == "ACTION_SEARCH") {
        return ACTION_SEARCH;
    }

    if (value == "ACTION_SELECT_ALL") {
        return ACTION_SELECT_ALL;
    }

    if (value == "ACTION_DELETE") {
        return ACTION_DELETE;
    }

    if (value == "ACTION_SETTINGS") {
        return ACTION_SETTINGS;
    }

    if (value == "ACTION_ARRANGE_BRING_TO_FRONT") {
        return ACTION_ARRANGE_BRING_TO_FRONT;
    }

    if (value == "ACTION_ARRANGE_BRING_FORWARD") {
        return ACTION_ARRANGE_BRING_FORWARD;
    }

    if (value == "ACTION_ARRANGE_SEND_BACKWARD") {
        return ACTION_ARRANGE_SEND_BACKWARD;
    }

    if (value == "ACTION_ARRANGE_SEND_TO_BACK") {
        return ACTION_ARRANGE_SEND_TO_BACK;
    }

    if (value == "ACTION_GOTO_FIRST") {
        return ACTION_GOTO_FIRST;
    }

    if (value == "ACTION_GOTO_BACK") {
        return ACTION_GOTO_BACK;
    }

    if (value == "ACTION_GOTO_PAGE") {
        return ACTION_GOTO_PAGE;
    }

    if (value == "ACTION_GOTO_NEXT") {
        return ACTION_GOTO_NEXT;
    }

    if (value == "ACTION_GOTO_LAST") {
        return ACTION_GOTO_LAST;
    }

    if (value == "ACTION_GOTO_NEXT_LAYER") {
        return ACTION_GOTO_NEXT_LAYER;
    }

    if (value == "ACTION_GOTO_PREVIOUS_LAYER") {
        return ACTION_GOTO_PREVIOUS_LAYER;
    }

    if (value == "ACTION_GOTO_TOP_LAYER") {
        return ACTION_GOTO_TOP_LAYER;
    }

    if (value == "ACTION_GOTO_NEXT_ANNOTATED_PAGE") {
        return ACTION_GOTO_NEXT_ANNOTATED_PAGE;
    }

    if (value == "ACTION_GOTO_PREVIOUS_ANNOTATED_PAGE") {
        return ACTION_GOTO_PREVIOUS_ANNOTATED_PAGE;
    }

    if (value == "ACTION_NEW_PAGE_BEFORE") {
        return ACTION_NEW_PAGE_BEFORE;
    }

    if (value == "ACTION_DUPLICATE_PAGE") {
        return ACTION_DUPLICATE_PAGE;
    }

    if (value == "ACTION_NEW_PAGE_AFTER") {
        return ACTION_NEW_PAGE_AFTER;
    }

    if (value == "ACTION_NEW_PAGE_AT_END") {
        return ACTION_NEW_PAGE_AT_END;
    }

    if (value == "ACTION_APPEND_NEW_PDF_PAGES") {
        return ACTION_APPEND_NEW_PDF_PAGES;
    }

    if (value == "ACTION_CONFIGURE_PAGE_TEMPLATE") {
        return ACTION_CONFIGURE_PAGE_TEMPLATE;
    }

    if (value == "ACTION_DELETE_PAGE") {
        return ACTION_DELETE_PAGE;
    }

    if (value == "ACTION_NEW_LAYER") {
        return ACTION_NEW_LAYER;
    }

    if (value == "ACTION_DELETE_LAYER") {
        return ACTION_DELETE_LAYER;
    }

    if (value == "ACTION_MERGE_LAYER_DOWN") {
        return ACTION_MERGE_LAYER_DOWN;
    }

    if (value == "ACTION_RENAME_LAYER") {
        return ACTION_RENAME_LAYER;
    }

    if (value == "ACTION_MOVE_SELECTION_LAYER_UP") {
        return ACTION_MOVE_SELECTION_LAYER_UP;
    }

    if (value == "ACTION_MOVE_SELECTION_LAYER_DOWN") {
        return ACTION_MOVE_SELECTION_LAYER_DOWN;
    }

    if (value == "ACTION_PAPER_FORMAT") {
        return ACTION_PAPER_FORMAT;
    }

    if (value == "ACTION_PAPER_BACKGROUND_COLOR") {
        return ACTION_PAPER_BACKGROUND_COLOR;
    }

    if (value == "ACTION_TOOL_PEN") {
        return ACTION_TOOL_PEN;
    }

    if (value == "ACTION_TOOL_ERASER") {
        return ACTION_TOOL_ERASER;
    }

    if (value == "ACTION_TOOL_HIGHLIGHTER") {
        return ACTION_TOOL_HIGHLIGHTER;
    }

    if (value == "ACTION_TOOL_TEXT") {
        return ACTION_TOOL_TEXT;
    }

    if (value == "ACTION_TOOL_IMAGE") {
        return ACTION_TOOL_IMAGE;
    }

    if (value == "ACTION_TOOL_SELECT_RECT") {
        return ACTION_TOOL_SELECT_RECT;
    }

    if (value == "ACTION_TOOL_SELECT_REGION") {
        return ACTION_TOOL_SELECT_REGION;
    }

    if (value == "ACTION_TOOL_SELECT_MULTILAYER_RECT") {
        return ACTION_TOOL_SELECT_MULTILAYER_RECT;
    }

    if (value == "ACTION_TOOL_SELECT_MULTILAYER_REGION") {
        return ACTION_TOOL_SELECT_MULTILAYER_REGION;
    }

    if (value == "ACTION_TOOL_SELECT_OBJECT") {
        return ACTION_TOOL_SELECT_OBJECT;
    }

    if (value == "ACTION_TOOL_PLAY_OBJECT") {
        return ACTION_TOOL_PLAY_OBJECT;
    }

    if (value == "ACTION_TOOL_VERTICAL_SPACE") {
        return ACTION_TOOL_VERTICAL_SPACE;
    }

    if (value == "ACTION_TOOL_HAND") {
        return ACTION_TOOL_HAND;
    }

    if (value == "ACTION_TOOL_DEFAULT") {
        return ACTION_TOOL_DEFAULT;
    }

    if (value == "ACTION_SHAPE_RECOGNIZER") {
        return ACTION_SHAPE_RECOGNIZER;
    }

    if (value == "ACTION_TOOL_DRAW_RECT") {
        return ACTION_TOOL_DRAW_RECT;
    }

    if (value == "ACTION_TOOL_DRAW_ELLIPSE") {
        return ACTION_TOOL_DRAW_ELLIPSE;
    }

    if (value == "ACTION_TOOL_DRAW_ARROW") {
        return ACTION_TOOL_DRAW_ARROW;
    }

    if (value == "ACTION_TOOL_DRAW_DOUBLE_ARROW") {
        return ACTION_TOOL_DRAW_DOUBLE_ARROW;
    }

    if (value == "ACTION_TOOL_DRAW_COORDINATE_SYSTEM") {
        return ACTION_TOOL_DRAW_COORDINATE_SYSTEM;
    }

    if (value == "ACTION_TOOL_SELECT_PDF_TEXT_LINEAR") {
        return ACTION_TOOL_SELECT_PDF_TEXT_LINEAR;
    }

    if (value == "ACTION_TOOL_SELECT_PDF_TEXT_RECT") {
        return ACTION_TOOL_SELECT_PDF_TEXT_RECT;
    }

    if (value == "ACTION_RULER") {
        return ACTION_RULER;
    }

    if (value == "ACTION_TOOL_DRAW_SPLINE") {
        return ACTION_TOOL_DRAW_SPLINE;
    }

    if (value == "ACTION_TOOL_FLOATING_TOOLBOX") {
        return ACTION_TOOL_FLOATING_TOOLBOX;
    }

    if (value == "ACTION_TOOL_LINE_STYLE_PLAIN") {
        return ACTION_TOOL_LINE_STYLE_PLAIN;
    }

    if (value == "ACTION_TOOL_LINE_STYLE_DASH") {
        return ACTION_TOOL_LINE_STYLE_DASH;
    }

    if (value == "ACTION_TOOL_LINE_STYLE_DASH_DOT") {
        return ACTION_TOOL_LINE_STYLE_DASH_DOT;
    }

    if (value == "ACTION_TOOL_LINE_STYLE_DOT") {
        return ACTION_TOOL_LINE_STYLE_DOT;
    }

    if (value == "ACTION_SIZE_VERY_FINE") {
        return ACTION_SIZE_VERY_FINE;
    }

    if (value == "ACTION_SIZE_FINE") {
        return ACTION_SIZE_FINE;
    }

    if (value == "ACTION_SIZE_MEDIUM") {
        return ACTION_SIZE_MEDIUM;
    }

    if (value == "ACTION_SIZE_THICK") {
        return ACTION_SIZE_THICK;
    }

    if (value == "ACTION_SIZE_VERY_THICK") {
        return ACTION_SIZE_VERY_THICK;
    }

    if (value == "ACTION_TOOL_ERASER_STANDARD") {
        return ACTION_TOOL_ERASER_STANDARD;
    }

    if (value == "ACTION_TOOL_ERASER_WHITEOUT") {
        return ACTION_TOOL_ERASER_WHITEOUT;
    }

    if (value == "ACTION_TOOL_ERASER_DELETE_STROKE") {
        return ACTION_TOOL_ERASER_DELETE_STROKE;
    }

    if (value == "ACTION_TOOL_ERASER_SIZE_VERY_FINE") {
        return ACTION_TOOL_ERASER_SIZE_VERY_FINE;
    }

    if (value == "ACTION_TOOL_ERASER_SIZE_FINE") {
        return ACTION_TOOL_ERASER_SIZE_FINE;
    }

    if (value == "ACTION_TOOL_ERASER_SIZE_MEDIUM") {
        return ACTION_TOOL_ERASER_SIZE_MEDIUM;
    }

    if (value == "ACTION_TOOL_ERASER_SIZE_THICK") {
        return ACTION_TOOL_ERASER_SIZE_THICK;
    }

    if (value == "ACTION_TOOL_ERASER_SIZE_VERY_THICK") {
        return ACTION_TOOL_ERASER_SIZE_VERY_THICK;
    }

    if (value == "ACTION_TOOL_PEN_SIZE_VERY_FINE") {
        return ACTION_TOOL_PEN_SIZE_VERY_FINE;
    }

    if (value == "ACTION_TOOL_PEN_SIZE_FINE") {
        return ACTION_TOOL_PEN_SIZE_FINE;
    }

    if (value == "ACTION_TOOL_PEN_SIZE_MEDIUM") {
        return ACTION_TOOL_PEN_SIZE_MEDIUM;
    }

    if (value == "ACTION_TOOL_PEN_SIZE_THICK") {
        return ACTION_TOOL_PEN_SIZE_THICK;
    }

    if (value == "ACTION_TOOL_PEN_SIZE_VERY_THICK") {
        return ACTION_TOOL_PEN_SIZE_VERY_THICK;
    }

    if (value == "ACTION_TOOL_PEN_FILL") {
        return ACTION_TOOL_PEN_FILL;
    }

    if (value == "ACTION_TOOL_PEN_FILL_OPACITY") {
        return ACTION_TOOL_PEN_FILL_OPACITY;
    }

    if (value == "ACTION_TOOL_HIGHLIGHTER_SIZE_VERY_FINE") {
        return ACTION_TOOL_HIGHLIGHTER_SIZE_VERY_FINE;
    }

    if (value == "ACTION_TOOL_HIGHLIGHTER_SIZE_FINE") {
        return ACTION_TOOL_HIGHLIGHTER_SIZE_FINE;
    }

    if (value == "ACTION_TOOL_HIGHLIGHTER_SIZE_MEDIUM") {
        return ACTION_TOOL_HIGHLIGHTER_SIZE_MEDIUM;
    }

    if (value == "ACTION_TOOL_HIGHLIGHTER_SIZE_THICK") {
        return ACTION_TOOL_HIGHLIGHTER_SIZE_THICK;
    }

    if (value == "ACTION_TOOL_HIGHLIGHTER_SIZE_VERY_THICK") {
        return ACTION_TOOL_HIGHLIGHTER_SIZE_VERY_THICK;
    }

    if (value == "ACTION_TOOL_HIGHLIGHTER_FILL") {
        return ACTION_TOOL_HIGHLIGHTER_FILL;
    }

    if (value == "ACTION_TOOL_HIGHLIGHTER_FILL_OPACITY") {
        return ACTION_TOOL_HIGHLIGHTER_FILL_OPACITY;
    }

    if (value == "ACTION_TOOL_FILL") {
        return ACTION_TOOL_FILL;
    }

    if (value == "ACTION_ROTATION_SNAPPING") {
        return ACTION_ROTATION_SNAPPING;
    }

    if (value == "ACTION_GRID_SNAPPING") {
        return ACTION_GRID_SNAPPING;
    }

    if (value == "ACTION_HIGHLIGHT_POSITION") {
        return ACTION_HIGHLIGHT_POSITION;
    }

    if (value == "ACTION_SETSQUARE") {
        return ACTION_SETSQUARE;
    }

    if (value == "ACTION_COMPASS") {
        return ACTION_COMPASS;
    }

    if (value == "ACTION_SELECT_COLOR") {
        return ACTION_SELECT_COLOR;
    }

    if (value == "ACTION_SELECT_COLOR_CUSTOM") {
        return ACTION_SELECT_COLOR_CUSTOM;
    }

    if (value == "ACTION_SELECT_FONT") {
        return ACTION_SELECT_FONT;
    }

    if (value == "ACTION_FONT_BUTTON_CHANGED") {
        return ACTION_FONT_BUTTON_CHANGED;
    }

    if (value == "ACTION_TEX") {
        return ACTION_TEX;
    }

    if (value == "ACTION_ZOOM_IN") {
        return ACTION_ZOOM_IN;
    }

    if (value == "ACTION_ZOOM_OUT") {
        return ACTION_ZOOM_OUT;
    }

    if (value == "ACTION_ZOOM_FIT") {
        return ACTION_ZOOM_FIT;
    }

    if (value == "ACTION_ZOOM_100") {
        return ACTION_ZOOM_100;
    }

    if (value == "ACTION_FULLSCREEN") {
        return ACTION_FULLSCREEN;
    }

    if (value == "ACTION_VIEW_PAIRED_PAGES") {
        return ACTION_VIEW_PAIRED_PAGES;
    }

    if (value == "ACTION_VIEW_PRESENTATION_MODE") {
        return ACTION_VIEW_PRESENTATION_MODE;
    }

    if (value == "ACTION_MANAGE_TOOLBAR") {
        return ACTION_MANAGE_TOOLBAR;
    }

    if (value == "ACTION_CUSTOMIZE_TOOLBAR") {
        return ACTION_CUSTOMIZE_TOOLBAR;
    }

    if (value == "ACTION_AUDIO_RECORD") {
        return ACTION_AUDIO_RECORD;
    }

    if (value == "ACTION_AUDIO_PAUSE_PLAYBACK") {
        return ACTION_AUDIO_PAUSE_PLAYBACK;
    }

    if (value == "ACTION_AUDIO_STOP_PLAYBACK") {
        return ACTION_AUDIO_STOP_PLAYBACK;
    }

    if (value == "ACTION_AUDIO_SEEK_FORWARDS") {
        return ACTION_AUDIO_SEEK_FORWARDS;
    }

    if (value == "ACTION_AUDIO_SEEK_BACKWARDS") {
        return ACTION_AUDIO_SEEK_BACKWARDS;
    }

    if (value == "ACTION_SET_PAIRS_OFFSET") {
        return ACTION_SET_PAIRS_OFFSET;
    }

    if (value == "ACTION_TOGGLE_PAIRS_PARITY") {
        return ACTION_TOGGLE_PAIRS_PARITY;
    }

    if (value == "ACTION_SET_COLUMNS") {
        return ACTION_SET_COLUMNS;
    }

    if (value == "ACTION_SET_COLUMNS_1") {
        return ACTION_SET_COLUMNS_1;
    }

    if (value == "ACTION_SET_COLUMNS_2") {
        return ACTION_SET_COLUMNS_2;
    }

    if (value == "ACTION_SET_COLUMNS_3") {
        return ACTION_SET_COLUMNS_3;
    }

    if (value == "ACTION_SET_COLUMNS_4") {
        return ACTION_SET_COLUMNS_4;
    }

    if (value == "ACTION_SET_COLUMNS_5") {
        return ACTION_SET_COLUMNS_5;
    }

    if (value == "ACTION_SET_COLUMNS_6") {
        return ACTION_SET_COLUMNS_6;
    }

    if (value == "ACTION_SET_COLUMNS_7") {
        return ACTION_SET_COLUMNS_7;
    }

    if (value == "ACTION_SET_COLUMNS_8") {
        return ACTION_SET_COLUMNS_8;
    }

    if (value == "ACTION_SET_ROWS") {
        return ACTION_SET_ROWS;
    }

    if (value == "ACTION_SET_ROWS_1") {
        return ACTION_SET_ROWS_1;
    }

    if (value == "ACTION_SET_ROWS_2") {
        return ACTION_SET_ROWS_2;
    }

    if (value == "ACTION_SET_ROWS_3") {
        return ACTION_SET_ROWS_3;
    }

    if (value == "ACTION_SET_ROWS_4") {
        return ACTION_SET_ROWS_4;
    }

    if (value == "ACTION_SET_ROWS_5") {
        return ACTION_SET_ROWS_5;
    }

    if (value == "ACTION_SET_ROWS_6") {
        return ACTION_SET_ROWS_6;
    }

    if (value == "ACTION_SET_ROWS_7") {
        return ACTION_SET_ROWS_7;
    }

    if (value == "ACTION_SET_ROWS_8") {
        return ACTION_SET_ROWS_8;
    }

    if (value == "ACTION_SET_LAYOUT_HORIZONTAL") {
        return ACTION_SET_LAYOUT_HORIZONTAL;
    }

    if (value == "ACTION_SET_LAYOUT_VERTICAL") {
        return ACTION_SET_LAYOUT_VERTICAL;
    }

    if (value == "ACTION_SET_LAYOUT_L2R") {
        return ACTION_SET_LAYOUT_L2R;
    }

    if (value == "ACTION_SET_LAYOUT_R2L") {
        return ACTION_SET_LAYOUT_R2L;
    }

    if (value == "ACTION_SET_LAYOUT_T2B") {
        return ACTION_SET_LAYOUT_T2B;
    }

    if (value == "ACTION_SET_LAYOUT_B2T") {
        return ACTION_SET_LAYOUT_B2T;
    }

    if (value == "ACTION_PLUGIN_MANAGER") {
        return ACTION_PLUGIN_MANAGER;
    }

    if (value == "ACTION_ABOUT") {
        return ACTION_ABOUT;
    }

    if (value == "ACTION_HELP") {
        return ACTION_HELP;
    }

    if (value == "ACTION_FOOTER_PAGESPIN") {
        return ACTION_FOOTER_PAGESPIN;
    }

    if (value == "ACTION_FOOTER_ZOOM_SLIDER") {
        return ACTION_FOOTER_ZOOM_SLIDER;
    }

    if (value == "ACTION_FOOTER_LAYER") {
        return ACTION_FOOTER_LAYER;
    }

    if (value == "ACTION_NOT_SELECTED") {
        return ACTION_NOT_SELECTED;
    }

    g_warning("Invalid enum value for ActionType: \"%s\"", value.c_str());
    return ACTION_NONE;
}


auto ActionType_toString(ActionType value) -> string {
    if (value == ACTION_NONE) {
        return "ACTION_NONE";
    }

    if (value == ACTION_NEW) {
        return "ACTION_NEW";
    }

    if (value == ACTION_OPEN) {
        return "ACTION_OPEN";
    }

    if (value == ACTION_ANNOTATE_PDF) {
        return "ACTION_ANNOTATE_PDF";
    }

    if (value == ACTION_SAVE) {
        return "ACTION_SAVE";
    }

    if (value == ACTION_SAVE_AS) {
        return "ACTION_SAVE_AS";
    }

    if (value == ACTION_EXPORT_AS_PDF) {
        return "ACTION_EXPORT_AS_PDF";
    }

    if (value == ACTION_EXPORT_AS) {
        return "ACTION_EXPORT_AS";
    }

    if (value == ACTION_PRINT) {
        return "ACTION_PRINT";
    }

    if (value == ACTION_QUIT) {
        return "ACTION_QUIT";
    }

    if (value == ACTION_UNDO) {
        return "ACTION_UNDO";
    }

    if (value == ACTION_REDO) {
        return "ACTION_REDO";
    }

    if (value == ACTION_CUT) {
        return "ACTION_CUT";
    }

    if (value == ACTION_COPY) {
        return "ACTION_COPY";
    }

    if (value == ACTION_PASTE) {
        return "ACTION_PASTE";
    }

    if (value == ACTION_SEARCH) {
        return "ACTION_SEARCH";
    }

    if (value == ACTION_SELECT_ALL) {
        return "ACTION_SELECT_ALL";
    }

    if (value == ACTION_DELETE) {
        return "ACTION_DELETE";
    }

    if (value == ACTION_SETTINGS) {
        return "ACTION_SETTINGS";
    }

    if (value == ACTION_ARRANGE_BRING_TO_FRONT) {
        return "ACTION_ARRANGE_BRING_TO_FRONT";
    }

    if (value == ACTION_ARRANGE_BRING_FORWARD) {
        return "ACTION_ARRANGE_BRING_FORWARD";
    }

    if (value == ACTION_ARRANGE_SEND_BACKWARD) {
        return "ACTION_ARRANGE_SEND_BACKWARD";
    }

    if (value == ACTION_ARRANGE_SEND_TO_BACK) {
        return "ACTION_ARRANGE_SEND_TO_BACK";
    }

    if (value == ACTION_GOTO_FIRST) {
        return "ACTION_GOTO_FIRST";
    }

    if (value == ACTION_GOTO_BACK) {
        return "ACTION_GOTO_BACK";
    }

    if (value == ACTION_GOTO_PAGE) {
        return "ACTION_GOTO_PAGE";
    }

    if (value == ACTION_GOTO_NEXT) {
        return "ACTION_GOTO_NEXT";
    }

    if (value == ACTION_GOTO_LAST) {
        return "ACTION_GOTO_LAST";
    }

    if (value == ACTION_GOTO_NEXT_LAYER) {
        return "ACTION_GOTO_NEXT_LAYER";
    }

    if (value == ACTION_GOTO_PREVIOUS_LAYER) {
        return "ACTION_GOTO_PREVIOUS_LAYER";
    }

    if (value == ACTION_GOTO_TOP_LAYER) {
        return "ACTION_GOTO_TOP_LAYER";
    }

    if (value == ACTION_GOTO_NEXT_ANNOTATED_PAGE) {
        return "ACTION_GOTO_NEXT_ANNOTATED_PAGE";
    }

    if (value == ACTION_GOTO_PREVIOUS_ANNOTATED_PAGE) {
        return "ACTION_GOTO_PREVIOUS_ANNOTATED_PAGE";
    }

    if (value == ACTION_NEW_PAGE_BEFORE) {
        return "ACTION_NEW_PAGE_BEFORE";
    }

    if (value == ACTION_DUPLICATE_PAGE) {
        return "ACTION_DUPLICATE_PAGE";
    }

    if (value == ACTION_NEW_PAGE_AFTER) {
        return "ACTION_NEW_PAGE_AFTER";
    }

    if (value == ACTION_NEW_PAGE_AT_END) {
        return "ACTION_NEW_PAGE_AT_END";
    }

    if (value == ACTION_APPEND_NEW_PDF_PAGES) {
        return "ACTION_APPEND_NEW_PDF_PAGES";
    }

    if (value == ACTION_CONFIGURE_PAGE_TEMPLATE) {
        return "ACTION_CONFIGURE_PAGE_TEMPLATE";
    }

    if (value == ACTION_DELETE_PAGE) {
        return "ACTION_DELETE_PAGE";
    }

    if (value == ACTION_NEW_LAYER) {
        return "ACTION_NEW_LAYER";
    }

    if (value == ACTION_DELETE_LAYER) {
        return "ACTION_DELETE_LAYER";
    }

    if (value == ACTION_MERGE_LAYER_DOWN) {
        return "ACTION_MERGE_LAYER_DOWN";
    }

    if (value == ACTION_RENAME_LAYER) {
        return "ACTION_RENAME_LAYER";
    }

    if (value == ACTION_MOVE_SELECTION_LAYER_UP) {
        return "ACTION_MOVE_SELECTION_LAYER_UP";
    }

    if (value == ACTION_MOVE_SELECTION_LAYER_DOWN) {
        return "ACTION_MOVE_SELECTION_LAYER_DOWN";
    }

    if (value == ACTION_PAPER_FORMAT) {
        return "ACTION_PAPER_FORMAT";
    }

    if (value == ACTION_PAPER_BACKGROUND_COLOR) {
        return "ACTION_PAPER_BACKGROUND_COLOR";
    }

    if (value == ACTION_TOOL_PEN) {
        return "ACTION_TOOL_PEN";
    }

    if (value == ACTION_TOOL_ERASER) {
        return "ACTION_TOOL_ERASER";
    }

    if (value == ACTION_TOOL_HIGHLIGHTER) {
        return "ACTION_TOOL_HIGHLIGHTER";
    }

    if (value == ACTION_TOOL_TEXT) {
        return "ACTION_TOOL_TEXT";
    }

    if (value == ACTION_TOOL_IMAGE) {
        return "ACTION_TOOL_IMAGE";
    }

    if (value == ACTION_TOOL_SELECT_RECT) {
        return "ACTION_TOOL_SELECT_RECT";
    }

    if (value == ACTION_TOOL_SELECT_REGION) {
        return "ACTION_TOOL_SELECT_REGION";
    }

    if (value == ACTION_TOOL_SELECT_MULTILAYER_RECT) {
        return "ACTION_TOOL_SELECT_MULTILAYER_RECT";
    }

    if (value == ACTION_TOOL_SELECT_MULTILAYER_REGION) {
        return "ACTION_TOOL_SELECT_MULTILAYER_REGION";
    }

    if (value == ACTION_TOOL_SELECT_OBJECT) {
        return "ACTION_TOOL_SELECT_OBJECT";
    }

    if (value == ACTION_TOOL_PLAY_OBJECT) {
        return "ACTION_TOOL_PLAY_OBJECT";
    }

    if (value == ACTION_TOOL_VERTICAL_SPACE) {
        return "ACTION_TOOL_VERTICAL_SPACE";
    }

    if (value == ACTION_TOOL_HAND) {
        return "ACTION_TOOL_HAND";
    }

    if (value == ACTION_TOOL_DEFAULT) {
        return "ACTION_TOOL_DEFAULT";
    }

    if (value == ACTION_SHAPE_RECOGNIZER) {
        return "ACTION_SHAPE_RECOGNIZER";
    }

    if (value == ACTION_TOOL_DRAW_RECT) {
        return "ACTION_TOOL_DRAW_RECT";
    }

    if (value == ACTION_TOOL_DRAW_ELLIPSE) {
        return "ACTION_TOOL_DRAW_ELLIPSE";
    }

    if (value == ACTION_TOOL_DRAW_ARROW) {
        return "ACTION_TOOL_DRAW_ARROW";
    }

    if (value == ACTION_TOOL_DRAW_DOUBLE_ARROW) {
        return "ACTION_TOOL_DRAW_DOUBLE_ARROW";
    }

    if (value == ACTION_TOOL_DRAW_COORDINATE_SYSTEM) {
        return "ACTION_TOOL_DRAW_COORDINATE_SYSTEM";
    }

    if (value == ACTION_TOOL_SELECT_PDF_TEXT_LINEAR) {
        return "ACTION_TOOL_SELECT_PDF_TEXT_LINEAR";
    }

    if (value == ACTION_TOOL_SELECT_PDF_TEXT_RECT) {
        return "ACTION_TOOL_SELECT_PDF_TEXT_RECT";
    }

    if (value == ACTION_RULER) {
        return "ACTION_RULER";
    }

    if (value == ACTION_TOOL_DRAW_SPLINE) {
        return "ACTION_TOOL_DRAW_SPLINE";
    }

    if (value == ACTION_TOOL_FLOATING_TOOLBOX) {
        return "ACTION_TOOL_FLOATING_TOOLBOX";
    }

    if (value == ACTION_TOOL_LINE_STYLE_PLAIN) {
        return "ACTION_TOOL_LINE_STYLE_PLAIN";
    }

    if (value == ACTION_TOOL_LINE_STYLE_DASH) {
        return "ACTION_TOOL_LINE_STYLE_DASH";
    }

    if (value == ACTION_TOOL_LINE_STYLE_DASH_DOT) {
        return "ACTION_TOOL_LINE_STYLE_DASH_DOT";
    }

    if (value == ACTION_TOOL_LINE_STYLE_DOT) {
        return "ACTION_TOOL_LINE_STYLE_DOT";
    }

    if (value == ACTION_SIZE_VERY_FINE) {
        return "ACTION_SIZE_VERY_FINE";
    }

    if (value == ACTION_SIZE_FINE) {
        return "ACTION_SIZE_FINE";
    }

    if (value == ACTION_SIZE_MEDIUM) {
        return "ACTION_SIZE_MEDIUM";
    }

    if (value == ACTION_SIZE_THICK) {
        return "ACTION_SIZE_THICK";
    }

    if (value == ACTION_SIZE_VERY_THICK) {
        return "ACTION_SIZE_VERY_THICK";
    }

    if (value == ACTION_TOOL_ERASER_STANDARD) {
        return "ACTION_TOOL_ERASER_STANDARD";
    }

    if (value == ACTION_TOOL_ERASER_WHITEOUT) {
        return "ACTION_TOOL_ERASER_WHITEOUT";
    }

    if (value == ACTION_TOOL_ERASER_DELETE_STROKE) {
        return "ACTION_TOOL_ERASER_DELETE_STROKE";
    }

    if (value == ACTION_TOOL_ERASER_SIZE_VERY_FINE) {
        return "ACTION_TOOL_ERASER_SIZE_VERY_FINE";
    }

    if (value == ACTION_TOOL_ERASER_SIZE_FINE) {
        return "ACTION_TOOL_ERASER_SIZE_FINE";
    }

    if (value == ACTION_TOOL_ERASER_SIZE_MEDIUM) {
        return "ACTION_TOOL_ERASER_SIZE_MEDIUM";
    }

    if (value == ACTION_TOOL_ERASER_SIZE_THICK) {
        return "ACTION_TOOL_ERASER_SIZE_THICK";
    }

    if (value == ACTION_TOOL_ERASER_SIZE_VERY_THICK) {
        return "ACTION_TOOL_ERASER_SIZE_VERY_THICK";
    }

    if (value == ACTION_TOOL_PEN_SIZE_VERY_FINE) {
        return "ACTION_TOOL_PEN_SIZE_VERY_FINE";
    }

    if (value == ACTION_TOOL_PEN_SIZE_FINE) {
        return "ACTION_TOOL_PEN_SIZE_FINE";
    }

    if (value == ACTION_TOOL_PEN_SIZE_MEDIUM) {
        return "ACTION_TOOL_PEN_SIZE_MEDIUM";
    }

    if (value == ACTION_TOOL_PEN_SIZE_THICK) {
        return "ACTION_TOOL_PEN_SIZE_THICK";
    }

    if (value == ACTION_TOOL_PEN_SIZE_VERY_THICK) {
        return "ACTION_TOOL_PEN_SIZE_VERY_THICK";
    }

    if (value == ACTION_TOOL_PEN_FILL) {
        return "ACTION_TOOL_PEN_FILL";
    }

    if (value == ACTION_TOOL_PEN_FILL_OPACITY) {
        return "ACTION_TOOL_PEN_FILL_OPACITY";
    }

    if (value == ACTION_TOOL_HIGHLIGHTER_SIZE_VERY_FINE) {
        return "ACTION_TOOL_HIGHLIGHTER_SIZE_VERY_FINE";
    }

    if (value == ACTION_TOOL_HIGHLIGHTER_SIZE_FINE) {
        return "ACTION_TOOL_HIGHLIGHTER_SIZE_FINE";
    }

    if (value == ACTION_TOOL_HIGHLIGHTER_SIZE_MEDIUM) {
        return "ACTION_TOOL_HIGHLIGHTER_SIZE_MEDIUM";
    }

    if (value == ACTION_TOOL_HIGHLIGHTER_SIZE_THICK) {
        return "ACTION_TOOL_HIGHLIGHTER_SIZE_THICK";
    }

    if (value == ACTION_TOOL_HIGHLIGHTER_SIZE_VERY_THICK) {
        return "ACTION_TOOL_HIGHLIGHTER_SIZE_VERY_THICK";
    }

    if (value == ACTION_TOOL_HIGHLIGHTER_FILL) {
        return "ACTION_TOOL_HIGHLIGHTER_FILL";
    }

    if (value == ACTION_TOOL_HIGHLIGHTER_FILL_OPACITY) {
        return "ACTION_TOOL_HIGHLIGHTER_FILL_OPACITY";
    }

    if (value == ACTION_TOOL_FILL) {
        return "ACTION_TOOL_FILL";
    }

    if (value == ACTION_ROTATION_SNAPPING) {
        return "ACTION_ROTATION_SNAPPING";
    }

    if (value == ACTION_GRID_SNAPPING) {
        return "ACTION_GRID_SNAPPING";
    }

    if (value == ACTION_HIGHLIGHT_POSITION) {
        return "ACTION_HIGHLIGHT_POSITION";
    }

    if (value == ACTION_SETSQUARE) {
        return "ACTION_SETSQUARE";
    }

    if (value == ACTION_COMPASS) {
        return "ACTION_COMPASS";
    }

    if (value == ACTION_SELECT_COLOR) {
        return "ACTION_SELECT_COLOR";
    }

    if (value == ACTION_SELECT_COLOR_CUSTOM) {
        return "ACTION_SELECT_COLOR_CUSTOM";
    }

    if (value == ACTION_SELECT_FONT) {
        return "ACTION_SELECT_FONT";
    }

    if (value == ACTION_FONT_BUTTON_CHANGED) {
        return "ACTION_FONT_BUTTON_CHANGED";
    }

    if (value == ACTION_TEX) {
        return "ACTION_TEX";
    }

    if (value == ACTION_ZOOM_IN) {
        return "ACTION_ZOOM_IN";
    }

    if (value == ACTION_ZOOM_OUT) {
        return "ACTION_ZOOM_OUT";
    }

    if (value == ACTION_ZOOM_FIT) {
        return "ACTION_ZOOM_FIT";
    }

    if (value == ACTION_ZOOM_100) {
        return "ACTION_ZOOM_100";
    }

    if (value == ACTION_FULLSCREEN) {
        return "ACTION_FULLSCREEN";
    }

    if (value == ACTION_VIEW_PAIRED_PAGES) {
        return "ACTION_VIEW_PAIRED_PAGES";
    }

    if (value == ACTION_VIEW_PRESENTATION_MODE) {
        return "ACTION_VIEW_PRESENTATION_MODE";
    }

    if (value == ACTION_MANAGE_TOOLBAR) {
        return "ACTION_MANAGE_TOOLBAR";
    }

    if (value == ACTION_CUSTOMIZE_TOOLBAR) {
        return "ACTION_CUSTOMIZE_TOOLBAR";
    }

    if (value == ACTION_AUDIO_RECORD) {
        return "ACTION_AUDIO_RECORD";
    }

    if (value == ACTION_AUDIO_PAUSE_PLAYBACK) {
        return "ACTION_AUDIO_PAUSE_PLAYBACK";
    }

    if (value == ACTION_AUDIO_STOP_PLAYBACK) {
        return "ACTION_AUDIO_STOP_PLAYBACK";
    }

    if (value == ACTION_AUDIO_SEEK_FORWARDS) {
        return "ACTION_AUDIO_SEEK_FORWARDS";
    }

    if (value == ACTION_AUDIO_SEEK_BACKWARDS) {
        return "ACTION_AUDIO_SEEK_BACKWARDS";
    }

    if (value == ACTION_SET_PAIRS_OFFSET) {
        return "ACTION_SET_PAIRS_OFFSET";
    }

    if (value == ACTION_TOGGLE_PAIRS_PARITY) {
        return "ACTION_TOGGLE_PAIRS_PARITY";
    }

    if (value == ACTION_SET_COLUMNS) {
        return "ACTION_SET_COLUMNS";
    }

    if (value == ACTION_SET_COLUMNS_1) {
        return "ACTION_SET_COLUMNS_1";
    }

    if (value == ACTION_SET_COLUMNS_2) {
        return "ACTION_SET_COLUMNS_2";
    }

    if (value == ACTION_SET_COLUMNS_3) {
        return "ACTION_SET_COLUMNS_3";
    }

    if (value == ACTION_SET_COLUMNS_4) {
        return "ACTION_SET_COLUMNS_4";
    }

    if (value == ACTION_SET_COLUMNS_5) {
        return "ACTION_SET_COLUMNS_5";
    }

    if (value == ACTION_SET_COLUMNS_6) {
        return "ACTION_SET_COLUMNS_6";
    }

    if (value == ACTION_SET_COLUMNS_7) {
        return "ACTION_SET_COLUMNS_7";
    }

    if (value == ACTION_SET_COLUMNS_8) {
        return "ACTION_SET_COLUMNS_8";
    }

    if (value == ACTION_SET_ROWS) {
        return "ACTION_SET_ROWS";
    }

    if (value == ACTION_SET_ROWS_1) {
        return "ACTION_SET_ROWS_1";
    }

    if (value == ACTION_SET_ROWS_2) {
        return "ACTION_SET_ROWS_2";
    }

    if (value == ACTION_SET_ROWS_3) {
        return "ACTION_SET_ROWS_3";
    }

    if (value == ACTION_SET_ROWS_4) {
        return "ACTION_SET_ROWS_4";
    }

    if (value == ACTION_SET_ROWS_5) {
        return "ACTION_SET_ROWS_5";
    }

    if (value == ACTION_SET_ROWS_6) {
        return "ACTION_SET_ROWS_6";
    }

    if (value == ACTION_SET_ROWS_7) {
        return "ACTION_SET_ROWS_7";
    }

    if (value == ACTION_SET_ROWS_8) {
        return "ACTION_SET_ROWS_8";
    }

    if (value == ACTION_SET_LAYOUT_HORIZONTAL) {
        return "ACTION_SET_LAYOUT_HORIZONTAL";
    }

    if (value == ACTION_SET_LAYOUT_VERTICAL) {
        return "ACTION_SET_LAYOUT_VERTICAL";
    }

    if (value == ACTION_SET_LAYOUT_L2R) {
        return "ACTION_SET_LAYOUT_L2R";
    }

    if (value == ACTION_SET_LAYOUT_R2L) {
        return "ACTION_SET_LAYOUT_R2L";
    }

    if (value == ACTION_SET_LAYOUT_T2B) {
        return "ACTION_SET_LAYOUT_T2B";
    }

    if (value == ACTION_SET_LAYOUT_B2T) {
        return "ACTION_SET_LAYOUT_B2T";
    }

    if (value == ACTION_PLUGIN_MANAGER) {
        return "ACTION_PLUGIN_MANAGER";
    }

    if (value == ACTION_ABOUT) {
        return "ACTION_ABOUT";
    }

    if (value == ACTION_HELP) {
        return "ACTION_HELP";
    }

    if (value == ACTION_FOOTER_PAGESPIN) {
        return "ACTION_FOOTER_PAGESPIN";
    }

    if (value == ACTION_FOOTER_ZOOM_SLIDER) {
        return "ACTION_FOOTER_ZOOM_SLIDER";
    }

    if (value == ACTION_FOOTER_LAYER) {
        return "ACTION_FOOTER_LAYER";
    }

    if (value == ACTION_NOT_SELECTED) {
        return "ACTION_NOT_SELECTED";
    }

    g_error("Invalid enum value for ActionType: %i", value);
    return "";
}
